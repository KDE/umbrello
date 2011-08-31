/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codegenobjectwithtextblocks.h"

// local includes
#include "codedocument.h"
#include "codeoperation.h"
#include "codegenerators/codegenfactory.h"
#include "classifiercodedocument.h"
#include "debug_utils.h"
#include "hierarchicalcodeblock.h"
#include "uml.h"
#include "umldoc.h"

/**
 * Constructor
 * @param parent   parent code document
 */
CodeGenObjectWithTextBlocks::CodeGenObjectWithTextBlocks ( CodeDocument *parent )
  : m_pCodeDoc(parent)
{
}

/**
 * Destructor
 */
CodeGenObjectWithTextBlocks::~CodeGenObjectWithTextBlocks ( )
{
    resetTextBlocks();
}

/**
 * Get the list of TextBlock objects held by m_textblockVector
 * @return list of TextBlock objects held by m_textblockVector
 */
TextBlockList * CodeGenObjectWithTextBlocks::getTextBlockList ( ) const
{
    return const_cast<TextBlockList*>(&m_textblockVector);
}

/**
 * Add a TextBlock object to the m_textblockVector List.
 * @param  add_object   text block to add
 * @return boolean value where false means not added because an TextBlock
 *                 object with that tag already exists in this document.
 */
bool CodeGenObjectWithTextBlocks::addTextBlock(TextBlock* add_object )
{
    QString tag = add_object->getTag();

    // assign a tag if one doesn't already exist
    if (tag.isEmpty())
    {
        tag = getUniqueTag();
        add_object->setTag(tag);
    }
    else
    {
        // if it has a tag, check to see that it is not in some other parent object
        // IF it is then we will need to remove it FIRST before adding to new parent
        CodeDocument * parentDoc = add_object->getParentDocument();
        if (parentDoc) {
            CodeGenObjectWithTextBlocks * oldParent = parentDoc->findParentObjectForTaggedTextBlock (tag);
            if (oldParent && oldParent != this)
                oldParent->removeTextBlock(add_object);
        }
    }

    if (m_textBlockTagMap.contains(tag))
        return false; // return false, we already have some object with this tag in the list

    // if we get here, then the object is a "fresh" one, we havent
    // added before. Add it now and return true.
    m_textBlockTagMap.insert(tag, add_object);
    m_textblockVector.append(add_object);

    return true;
}

/**
 * Remove a TextBlock object from m_textblockVector list.
 * @param remove_object   the text block to be removed
 * @return success status
 */
bool CodeGenObjectWithTextBlocks::removeTextBlock ( TextBlock * remove_object )
{
    // check if we can remove it from our local list
    int indx = m_textblockVector.indexOf(remove_object);
    if (indx > -1) {
        m_textblockVector.removeAt(indx);
    }
    else {
        // may be hiding in child hierarchical codeblock
        foreach (TextBlock* tb, m_textblockVector)
        {
            HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock*>(tb);
            if (hb && hb->removeTextBlock(remove_object))
                return true;
        }
        return false;
    }

    // if we get here.. it was in this object so remove from our map
    QString tag = remove_object->getTag();
    if (!tag.isEmpty()) {
        m_textBlockTagMap.remove(tag);
    }
    return true;
}

/**
 * Find the text block with a given tag.
 * @param tag   the tag to search with
 * @return      the found TextBlock object
 */
TextBlock * CodeGenObjectWithTextBlocks::findTextBlockByTag( const QString &tag )
{
    //if we already know to which file this class was written/should be written, just return it.
    if (m_textBlockTagMap.contains(tag)) {
        return m_textBlockTagMap[tag];
    }
    return 0;
}

/**
 * Find the direct parent for a given textblock. This
 * may be any object which holds text blocks, e.g. a CodeGenObjectWithTextBlocks.
 * IMPORTANT: this will only search for a parent from the viewpoint of this object
 * and down into its Hierarchical codeblocks. This means you should start any
 * search from the parent document of the text block. This method NOT meant for
 * casual usage.
 * @param  tag   tag to find the text block
 * @return parent object. Could return null if the textblock is missing from the
 *         branch of the document tree being examined.
 */
CodeGenObjectWithTextBlocks * CodeGenObjectWithTextBlocks::findParentObjectForTaggedTextBlock (const QString & tag)
{
    // what??!? no tag, then CANT be here
    if (tag.isEmpty())
        return (CodeGenObjectWithTextBlocks*) NULL;

    if (!findTextBlockByTag(tag))
    {
        // may be hiding in child hierarchical codeblock
        foreach (TextBlock* tb, m_textblockVector)
        {
            HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock*>(tb);
            if (hb) {
                CodeGenObjectWithTextBlocks* cgowtb = dynamic_cast<CodeGenObjectWithTextBlocks*>(hb);
                CodeGenObjectWithTextBlocks * obj = cgowtb->findParentObjectForTaggedTextBlock(tag);
                if (obj)
                    return obj;
            }
        }

    } else
        return this;

    // shouldn't happen unless the textblock doesn't exist in this object
    // or its children at all
    return (CodeGenObjectWithTextBlocks*) NULL;
}

/**
 * Will get a hierarchicalcodeblock from the document with given tag. IF the codeblock
 * doesn't exist, then it will create it at the end of the document textBlock
 * list and pass back a reference.
 * @param   tag           tag to find the text block
 * @param   comment       comment
 * @param   indentLevel   indentation level
 * @return  HierarchicalCodeBlock object
 */
HierarchicalCodeBlock * CodeGenObjectWithTextBlocks::getHierarchicalCodeBlock ( const QString &tag, const QString &comment, int indentLevel )
{
    // now actually declare the fields
    HierarchicalCodeBlock * codeBlock = dynamic_cast<HierarchicalCodeBlock*>(findTextBlockByTag(tag));
    if (!codeBlock) {
        codeBlock = newHierarchicalCodeBlock();
        codeBlock->setTag(tag);
        codeBlock->setComment(CodeGenFactory::newCodeComment(m_pCodeDoc));
        // don't write empty comments out
        if(comment.isEmpty())
            codeBlock->getComment()->setWriteOutText(false);

        if(!addTextBlock(codeBlock))
        {
            delete codeBlock;
            return (HierarchicalCodeBlock*) NULL;
        }
    }

    codeBlock->setOverallIndentationLevel (indentLevel);
    codeBlock->getComment()->setText(comment);

    return codeBlock;
}

/**
 * Will get a codeblockwithcomments from the document with given tag. If the codeblock
 * doesn't exist, then it will create it at the end of the document textBlock
 * list and pass back a reference.
 * @param   tag           tag to find the text block
 * @param   comment       comment
 * @param   indentLevel   indentation level
 * @return  CodeBlockWithComments object
 */
CodeBlockWithComments * CodeGenObjectWithTextBlocks::getCodeBlockWithComments ( const QString &tag, const QString &comment, int indentLevel )
{
    // now actually declare the fields
    CodeBlockWithComments * codeBlock = dynamic_cast<CodeBlockWithComments*>(findTextBlockByTag(tag));
    if (!codeBlock) {
        codeBlock = newCodeBlockWithComments();
        codeBlock->setTag(tag);
        codeBlock->setComment(CodeGenFactory::newCodeComment(m_pCodeDoc));
        // don't write empty comments out
        if(comment.isEmpty())
            codeBlock->getComment()->setWriteOutText(false);
        if(!addTextBlock(codeBlock))
            return (CodeBlockWithComments*) NULL;
    }
    codeBlock->setOverallIndentationLevel (indentLevel);
    codeBlock->getComment()->setText(comment);

    return codeBlock;
}

/**
 * Allows the user to add a code comment to the end of the list
 * of text blocks in this document OR, if a text block already exists
 * with that tag, it will update it with the passed text as appropriate.
 * @param tag                tag to find the text block
 * @param text               code comment to set
 * @param indentationLevel   indentation level
 * @return codeblock/comment pointer to the object which was created/updated.
 */
CodeComment * CodeGenObjectWithTextBlocks::addOrUpdateTaggedCodeComment ( const QString &tag, const QString &text, int indentationLevel)
{
    TextBlock * tBlock = findTextBlockByTag(tag);
    CodeComment * codeComment = dynamic_cast<CodeComment*>(tBlock);
    bool createdCodeComment = false;

    if(!codeComment) {
        createdCodeComment = true;
        codeComment = CodeGenFactory::newCodeComment(m_pCodeDoc);
        codeComment->setTag(tag);
        if(!addTextBlock(codeComment))
        {
            delete codeComment;
            return 0; // hmm. total failure..,was there a preexisting comment with this tag?? lets return null
        }
    }

    codeComment->setText(text);
    if (createdCodeComment) {
        if(!text.isEmpty())
            codeComment->setWriteOutText(true); // set to visible, if we created
        else
            codeComment->setWriteOutText(false); // set to not visible, if we created
    }

    codeComment->setIndentationLevel(indentationLevel);

    return codeComment;
}

/**
 * Allows the user to either add a code block with comments to the end of the list
 * of text blocks in this document OR, if a text block already exists
 * with that tag, it will update it with the passed text as appropriate.
 * @param tag                    tag to find the text block
 * @param text                   text to set
 * @param ctext                  comment to set
 * @param indentLevel            indentation level
 * @param forceUserBlockUpdate   ...
 * @return  codeblock/comment pointer to the object which was created/updated
 */
CodeBlockWithComments * CodeGenObjectWithTextBlocks::addOrUpdateTaggedCodeBlockWithComments (const QString &tag, const QString &text, const QString &ctext, int indentLevel, bool forceUserBlockUpdate )
{
    TextBlock * tBlock = findTextBlockByTag(tag);
    CodeBlockWithComments * codeBlock = dynamic_cast<CodeBlockWithComments*>(tBlock);
    bool createdCodeBlock = false;

    if (!codeBlock) {
        createdCodeBlock = true;
        codeBlock = newCodeBlockWithComments();
        codeBlock->setTag(tag);
        if(!addTextBlock(codeBlock))
        {
            delete codeBlock;
            return 0; // hmm. total failure..,was there a preexisting codeblock with this tag?? lets return null
        }
    }

    // ONLY update IF we are forcing the update of user blocks OR it is an "AutoGenerated" Block
    if (forceUserBlockUpdate || codeBlock->contentType() == CodeBlock::AutoGenerated)
    {
        codeBlock->setText(text);
        codeBlock->getComment()->setText(ctext);

        // if we created this from scratch, make it write out only when the block isnt empty
        if (createdCodeBlock)
        {
            if (!ctext.isEmpty())
                codeBlock->getComment()->setWriteOutText(true);
            else
                codeBlock->getComment()->setWriteOutText(false);

            if (!text.isEmpty())
                codeBlock->setWriteOutText(true);
            else
                codeBlock->setWriteOutText(false);
        }

        codeBlock->setOverallIndentationLevel(indentLevel);
    }

    return codeBlock;
}

/**
 * Reset/clear the inventory text blocks held by this object.
 */
void CodeGenObjectWithTextBlocks::resetTextBlocks()
{
    /**************  @todo I had to deactivate this code:
    TextBlock *tb;
    for (TextBlockListIt it(m_textblockVector); (tb = it.current()) != NULL; ++it)
        delete tb;
     **************  else the following crash happens on loading an XMI file:
     Invalid read of size 4
        at 0x8134BAA: QString::QString(QString const&) (qstring.h:650)
        by 0x8331EE5: TextBlock::getTag() const (textblock.cpp:103)
        by 0x82C4634: ClassifierCodeDocument::findCodeClassFieldTextBlockByTag(QString const&) (classifiercodedocument.cpp:730)
        by 0x82F3940: HierarchicalCodeBlock::findCodeClassFieldTextBlockByTag(QString const&) (hierarchicalcodeblock.cpp:366)
        by 0x82D71DC: CodeGenObjectWithTextBlocks::loadChildTextBlocksFromNode(QDomElement&) (codegenobjectwithtextblocks.cpp:422)
        by 0x82D6DFF: CodeGenObjectWithTextBlocks::setAttributesFromNode(QDomElement&) (codegenobjectwithtextblocks.cpp:373)
        by 0x82F3412: HierarchicalCodeBlock::setAttributesFromNode(QDomElement&) (hierarchicalcodeblock.cpp:290)
        by 0x82F3248: HierarchicalCodeBlock::loadFromXMI(QDomElement&) (hierarchicalcodeblock.cpp:269)
        by 0x82D760F: CodeGenObjectWithTextBlocks::loadChildTextBlocksFromNode(QDomElement&) (codegenobjectwithtextblocks.cpp:453)
        by 0x82D6DFF: CodeGenObjectWithTextBlocks::setAttributesFromNode(QDomElement&) (codegenobjectwithtextblocks.cpp:373)
        by 0x82CFB9C: CodeDocument::setAttributesFromNode(QDomElement&) (codedocument.cpp:414)
        by 0x82C3CAE: ClassifierCodeDocument::setAttributesFromNode(QDomElement&) (classifiercodedocument.cpp:595)
        by 0x82C4388: ClassifierCodeDocument::loadFromXMI(QDomElement&) (classifiercodedocument.cpp:687)
        by 0x82D372E: CodeGenerator::loadFromXMI(QDomElement&) (codegenerator.cpp:205)
        by 0x8356381: UMLDoc::loadExtensionsFromXMI(QDomNode&) (umldoc.cpp:1774)
        by 0x8353F16: UMLDoc::loadFromXMI(QIODevice&, short) (umldoc.cpp:1480)
        by 0x834DBB6: UMLDoc::openDocument(KUrl const&, char const*) (umldoc.cpp:475)
        by 0x833F0DE: UMLApp::openDocumentFile(KUrl const&) (uml.cpp:642)
        by 0x83098D9: initDocument(KCmdLineArgs*) (main.cpp:125)
        by 0x830974D: main (main.cpp:101)
      Address 0x66539DC is 12 bytes inside a block of size 68 free'd
        at 0x401D567: operator delete(void*) (vg_replace_malloc.c:244)
        by 0x8204CA2: CPPSourceCodeAccessorMethod::~CPPSourceCodeAccessorMethod() (cppsourcecodeaccessormethod.cpp:51)
        by 0x82D6C6D: CodeGenObjectWithTextBlocks::resetTextBlocks() (codegenobjectwithtextblocks.cpp:333)
        by 0x82D6001: CodeGenObjectWithTextBlocks::~CodeGenObjectWithTextBlocks() (codegenobjectwithtextblocks.cpp:42)
        by 0x82F272A: HierarchicalCodeBlock::~HierarchicalCodeBlock() (hierarchicalcodeblock.cpp:41)
        by 0x82D6C6D: CodeGenObjectWithTextBlocks::resetTextBlocks() (codegenobjectwithtextblocks.cpp:333)
        by 0x82CF294: CodeDocument::resetTextBlocks() (codedocument.cpp:330)
        by 0x82026EC: CPPSourceCodeDocument::resetTextBlocks() (cppsourcecodedocument.cpp:111)
        by 0x82D6DE7: CodeGenObjectWithTextBlocks::setAttributesFromNode(QDomElement&) (codegenobjectwithtextblocks.cpp:370)
        by 0x82CFB9C: CodeDocument::setAttributesFromNode(QDomElement&) (codedocument.cpp:414)
        by 0x82C3CAE: ClassifierCodeDocument::setAttributesFromNode(QDomElement&) (classifiercodedocument.cpp:595)
        by 0x82C4388: ClassifierCodeDocument::loadFromXMI(QDomElement&) (classifiercodedocument.cpp:687)
        by 0x82D372E: CodeGenerator::loadFromXMI(QDomElement&) (codegenerator.cpp:205)
        by 0x8356381: UMLDoc::loadExtensionsFromXMI(QDomNode&) (umldoc.cpp:1774)
        by 0x8353F16: UMLDoc::loadFromXMI(QIODevice&, short) (umldoc.cpp:1480)
        by 0x834DBB6: UMLDoc::openDocument(KUrl const&, char const*) (umldoc.cpp:475)
        by 0x833F0DE: UMLApp::openDocumentFile(KUrl const&) (uml.cpp:642)
        by 0x83098D9: initDocument(KCmdLineArgs*) (main.cpp:125)
        by 0x830974D: main (main.cpp:101)
     *************/

    m_textBlockTagMap.clear();
    m_textblockVector.clear();
}

/**
 * Empty method.
 */
void CodeGenObjectWithTextBlocks::setAttributesFromObject (CodeGenObjectWithTextBlocks * obj)
{
    Q_UNUSED(obj);
/*
    TextBlockList * list = obj->getTextBlockList();
    foreach (TextBlock* tb, *list) {
        // FIX : we need some functionality like
        // loadChildTextBlocksFromObject(obj) here
    }
*/
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeGenObjectWithTextBlocks::setAttributesOnNode (QDomDocument & doc, QDomElement & root)
{
    // set a section to hold document content
    QDomElement tblockElement = doc.createElement( "textblocks" );

    // only concrete calls to textblocks are saved
    TextBlockList * tbList = getTextBlockList();
    foreach (TextBlock* block, *tbList) {
        block->saveToXMI(doc, tblockElement);
    }
    root.appendChild( tblockElement);
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 * @param root   node from which to load the child text blocks
 */
void CodeGenObjectWithTextBlocks::setAttributesFromNode ( QDomElement & root)
{
    // clear existing codeblocks
    resetTextBlocks();
    // now load em back in
    loadChildTextBlocksFromNode(root);
}

/**
 * Load text blocks.
 * In this vanilla version, we only load comments and codeblocks
 * as they are the only instanciatable (vanilla) things
 * this method should be overridden if this class is inherited
 * by some other class that is concrete and takes children
 * derived from codeblock/codecomment/hierarchicalcb/ownedhiercodeblock.
 * @param root   node from which to load the child text blocks
 */
void CodeGenObjectWithTextBlocks::loadChildTextBlocksFromNode ( QDomElement & root)
{
    QDomNode tnode = root.firstChild();
    QDomElement telement = tnode.toElement();
    bool loadCheckForChildrenOK = false;
    while (!telement.isNull()) {
        QString nodeName = telement.tagName();

        if (nodeName != "textblocks") {
            tnode = telement.nextSibling();
            telement = tnode.toElement();
            continue;
        }

        QDomNode node = telement.firstChild();
        QDomElement element = node.toElement();

        // if there is nothing to begin with, then we don't worry about it
        loadCheckForChildrenOK = element.isNull() ? true : false;

        while (!element.isNull()) {
            QString name = element.tagName();

            if (name == "codecomment") {
                CodeComment * block = CodeGenFactory::newCodeComment(m_pCodeDoc);
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add codeComment to :" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == "codeaccessormethod" ||
                       name == "ccfdeclarationcodeblock") {
                QString acctag = element.attribute("tag","");
                // search for our method in the
                TextBlock * tb = findCodeClassFieldTextBlockByTag(acctag);
                if (!tb || !addTextBlock(tb)) {
                    uError() << "unable to add code accessor/decl method block (tag:"
                        << acctag << ") to:" << this;
                    // DON'T delete
                } else
                    loadCheckForChildrenOK = true;

            } else if( name == "codeblock") {
                CodeBlock * block = newCodeBlock();
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add codeBlock to :" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == "codeblockwithcomments") {
                CodeBlockWithComments * block = newCodeBlockWithComments();
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add codeBlockwithcomments to:" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == "header") {
                // do nothing.. this is treated elsewhere

            } else if (name == "hierarchicalcodeblock") {
                HierarchicalCodeBlock * block = new HierarchicalCodeBlock(m_pCodeDoc);
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add hierarchicalcodeBlock to:" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == "codeoperation") {
                // find the code operation by id
                QString id = element.attribute("parent_id","-1");
                UMLObject * obj = UMLApp::app()->document()->findObjectById(STR2ID(id));
                UMLOperation * op = dynamic_cast<UMLOperation*>(obj);
                if (op) {
                    CodeOperation * block = CodeGenFactory::newCodeOperation(dynamic_cast<ClassifierCodeDocument*>(m_pCodeDoc), op);
                    block->loadFromXMI(element);
                    if (addTextBlock(block))
                        loadCheckForChildrenOK = true;
                    else {
                        uError() << "unable to add codeoperation to:" << this;
                        delete block;
                    }
                } else
                    uError() << "unable to create codeoperation for obj id:" << id;
            } else
                uWarning() << "Got strange tag in text block stack: name=" << name << ", ignorning";

            node = element.nextSibling();
            element = node.toElement();
        }

        break;
    }

    if (!loadCheckForChildrenOK) {
        CodeDocument * test = dynamic_cast<CodeDocument*>(this);
        if (test) {
            uWarning() << "unable to initialize any child blocks in doc: " << test->getFileName() << " " << this;
        } else {
            HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock*>(this);
            if (hb)
                uWarning() << "unable to initialize any child blocks in Hblock: "<< hb->getTag() << " " << this;
            else
                uDebug() << "unable to initialize any child blocks in UNKNOWN OBJ:" << this;
        }
    }

}

