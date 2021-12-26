/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

// qt/kde includes
#include <QXmlStreamWriter>

/**
 * Constructor
 * @param parent   parent code document
 */
CodeGenObjectWithTextBlocks::CodeGenObjectWithTextBlocks (CodeDocument *parent)
  : m_pCodeDoc(parent)
{
}

/**
 * Destructor
 */
CodeGenObjectWithTextBlocks::~CodeGenObjectWithTextBlocks ()
{
    resetTextBlocks();
}

/**
 * Get the list of TextBlock objects held by m_textblockVector
 * @return list of TextBlock objects held by m_textblockVector
 */
TextBlockList * CodeGenObjectWithTextBlocks::getTextBlockList () const
{
    return const_cast<TextBlockList*>(&m_textblockVector);
}

/**
 * Add a TextBlock object to the m_textblockVector List.
 * @param  add_object   text block to add
 * @return boolean value where false means not added because an TextBlock
 *                 object with that tag already exists in this document.
 */
bool CodeGenObjectWithTextBlocks::addTextBlock(TextBlock* add_object)
{
    QString tag = add_object->getTag();

    // assign a tag if one doesn't already exist
    if (tag.isEmpty()) {
        tag = getUniqueTag();
        add_object->setTag(tag);
    } else {
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
bool CodeGenObjectWithTextBlocks::removeTextBlock (TextBlock * remove_object)
{
    // check if we can remove it from our local list
    int indx = m_textblockVector.indexOf(remove_object);
    if (indx > -1) {
        m_textblockVector.removeAt(indx);
    }
    else {
        // may be hiding in child hierarchical codeblock
        foreach (TextBlock* tb, m_textblockVector) {
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
TextBlock * CodeGenObjectWithTextBlocks::findTextBlockByTag(const QString &tag)
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
        return (CodeGenObjectWithTextBlocks*) 0;

    if (!findTextBlockByTag(tag)) {
        // may be hiding in child hierarchical codeblock
        foreach (TextBlock* tb, m_textblockVector) {
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
    return (CodeGenObjectWithTextBlocks*) 0;
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
HierarchicalCodeBlock * CodeGenObjectWithTextBlocks::getHierarchicalCodeBlock (const QString &tag, const QString &comment, int indentLevel)
{
    // now actually declare the fields
    HierarchicalCodeBlock * codeBlock = dynamic_cast<HierarchicalCodeBlock*>(findTextBlockByTag(tag));
    if (!codeBlock) {
        codeBlock = newHierarchicalCodeBlock();
        codeBlock->setTag(tag);
        codeBlock->setComment(CodeGenFactory::newCodeComment(m_pCodeDoc));
        // don't write empty comments out
        if (comment.isEmpty())
            codeBlock->getComment()->setWriteOutText(false);
        if (!addTextBlock(codeBlock)) {
            delete codeBlock;
            return (HierarchicalCodeBlock*) 0;
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
CodeBlockWithComments * CodeGenObjectWithTextBlocks::getCodeBlockWithComments (const QString &tag, const QString &comment, int indentLevel)
{
    // now actually declare the fields
    CodeBlockWithComments * codeBlock = dynamic_cast<CodeBlockWithComments*>(findTextBlockByTag(tag));
    if (!codeBlock) {
        codeBlock = newCodeBlockWithComments();
        codeBlock->setTag(tag);
        codeBlock->setComment(CodeGenFactory::newCodeComment(m_pCodeDoc));
        // don't write empty comments out
        if (comment.isEmpty())
            codeBlock->getComment()->setWriteOutText(false);
        if (!addTextBlock(codeBlock)) {
            delete codeBlock;
            return (CodeBlockWithComments*) 0;
        }
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
CodeComment * CodeGenObjectWithTextBlocks::addOrUpdateTaggedCodeComment (const QString &tag, const QString &text, int indentationLevel)
{
    TextBlock * tBlock = findTextBlockByTag(tag);
    CodeComment * codeComment = dynamic_cast<CodeComment*>(tBlock);
    bool createdCodeComment = false;

    if (!codeComment) {
        createdCodeComment = true;
        codeComment = CodeGenFactory::newCodeComment(m_pCodeDoc);
        codeComment->setTag(tag);
        if (!addTextBlock(codeComment)) {
            delete codeComment;
            return 0; // hmm. total failure.., was there a preexisting comment with this tag?? lets return null
        }
    }

    codeComment->setText(text);
    if (createdCodeComment) {
        if (!text.isEmpty())
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
CodeBlockWithComments * CodeGenObjectWithTextBlocks::addOrUpdateTaggedCodeBlockWithComments (const QString &tag, const QString &text, const QString &ctext, int indentLevel, bool forceUserBlockUpdate)
{
    TextBlock * tBlock = findTextBlockByTag(tag);
    CodeBlockWithComments * codeBlock = dynamic_cast<CodeBlockWithComments*>(tBlock);
    bool createdCodeBlock = false;

    if (!codeBlock) {
        createdCodeBlock = true;
        codeBlock = newCodeBlockWithComments();
        codeBlock->setTag(tag);
        if (!addTextBlock(codeBlock)) {
            delete codeBlock;
            return 0; // hmm. total failure.., was there a preexisting codeblock with this tag?? lets return null
        }
    }

    // ONLY update IF we are forcing the update of user blocks OR it is an "AutoGenerated" Block
    if (forceUserBlockUpdate || codeBlock->contentType() == CodeBlock::AutoGenerated) {
        codeBlock->setText(text);
        codeBlock->getComment()->setText(ctext);

        // if we created this from scratch, make it write out only when the block isnt empty
        if (createdCodeBlock) {
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
    for (TextBlockListIt it(m_textblockVector); (tb = it.current()) != 0; ++it)
        delete tb;
     **************  else crash happens on loading an XMI file
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
void CodeGenObjectWithTextBlocks::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // set a section to hold document content
    writer.writeStartElement(QLatin1String("textblocks"));

    // only concrete calls to textblocks are saved
    TextBlockList * tbList = getTextBlockList();
    foreach (TextBlock* block, *tbList) {
        block->saveToXMI(writer);
    }
    writer.writeEndElement();
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 * @param root   node from which to load the child text blocks
 */
void CodeGenObjectWithTextBlocks::setAttributesFromNode (QDomElement & root)
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
void CodeGenObjectWithTextBlocks::loadChildTextBlocksFromNode (QDomElement & root)
{
    QDomNode tnode = root.firstChild();
    QDomElement telement = tnode.toElement();
    bool loadCheckForChildrenOK = false;
    while (!telement.isNull()) {
        QString nodeName = telement.tagName();

        if (nodeName != QLatin1String("textblocks")) {
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

            if (name == QLatin1String("codecomment")) {
                CodeComment * block = CodeGenFactory::newCodeComment(m_pCodeDoc);
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add codeComment to :" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == QLatin1String("codeaccessormethod") ||
                       name == QLatin1String("ccfdeclarationcodeblock")) {
                QString acctag = element.attribute(QLatin1String("tag"));
                // search for our method in the
                TextBlock * tb = findCodeClassFieldTextBlockByTag(acctag);
                if (!tb || !addTextBlock(tb)) {
                    uError() << "unable to add code accessor/decl method block (tag:"
                        << acctag << ") to:" << this;
                    // DON'T delete
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == QLatin1String("codeblock")) {
                CodeBlock * block = newCodeBlock();
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add codeBlock to :" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == QLatin1String("codeblockwithcomments")) {
                CodeBlockWithComments * block = newCodeBlockWithComments();
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add codeBlockwithcomments to:" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == QLatin1String("header")) {
                // do nothing.. this is treated elsewhere

            } else if (name == QLatin1String("hierarchicalcodeblock")) {
                HierarchicalCodeBlock * block = new HierarchicalCodeBlock(m_pCodeDoc);
                block->loadFromXMI(element);
                if (!addTextBlock(block)) {
                    uError() << "unable to add hierarchicalcodeBlock to:" << this;
                    delete block;
                } else
                    loadCheckForChildrenOK = true;

            } else if (name == QLatin1String("codeoperation")) {
                // find the code operation by id
                QString id = element.attribute(QLatin1String("parent_id"), QLatin1String("-1"));
                UMLObject * obj = UMLApp::app()->document()->findObjectById(Uml::ID::fromString(id));
                UMLOperation * op = obj->asUMLOperation();
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
                uWarning() << "Got strange tag in text block stack: name=" << name << ", ignoring";

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

