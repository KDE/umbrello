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
#include "hierarchicalcodeblock.h"

// local includes
#include "codedocument.h"
#include "classifiercodedocument.h"
#include "codeclassfield.h"
#include "codegenerationpolicy.h"
#include "codegenerators/codegenfactory.h"
#include "debug_utils.h"
#include "uml.h"

/**
 * Constructor
 */
HierarchicalCodeBlock::HierarchicalCodeBlock(CodeDocument * doc , const QString &start, const QString &endString, const QString &comment)
  : CodeBlockWithComments (doc, start, comment),
    CodeGenObjectWithTextBlocks(doc)
{
    setEndText(endString);
    initAttributes();
}

HierarchicalCodeBlock::~HierarchicalCodeBlock()
{
}

/**
 * Set the value of m_endText
 * @param new_var the new value of m_endText
 */
void HierarchicalCodeBlock::setEndText ( const QString &new_var )
{
    m_endText = new_var;
}

/**
 * Get the value of m_endText
 * @return the value of m_endText
 */
QString HierarchicalCodeBlock::getEndText ( ) const
{
    return m_endText;
}

/**
 * return a unique, and currently unallocated, text block tag for this hblock
 */
QString HierarchicalCodeBlock::getUniqueTag()
{
    return getUniqueTag("hblock_tag");
}

/**
 * return a unique, and currently unallocated, text block tag for this hblock
 */
QString HierarchicalCodeBlock::getUniqueTag( const QString& prefix )
{
    return getParentDocument()->getUniqueTag(prefix);
}

CodeBlock * HierarchicalCodeBlock::newCodeBlock()
{
    return getParentDocument()->newCodeBlock();
}

CodeBlockWithComments * HierarchicalCodeBlock::newCodeBlockWithComments()
{
    return getParentDocument()->newCodeBlockWithComments();
}

HierarchicalCodeBlock * HierarchicalCodeBlock::newHierarchicalCodeBlock()
{
    HierarchicalCodeBlock *hb = new HierarchicalCodeBlock(getParentDocument());
    //hb->update();
    return hb;
}

/**
 * Add a TextBlock object to the m_textblockVector List
 */
bool HierarchicalCodeBlock::addTextBlock(TextBlock* add_object )
{
    if (CodeGenObjectWithTextBlocks::addTextBlock(add_object))
    {
        getParentDocument()->addChildTagToMap(add_object->getTag(), add_object);
        return true;
    }
    return false;
}

/**
 * Insert a new text block before/after the existing text block. Returns
 * false if it cannot insert the textblock.
 */
bool HierarchicalCodeBlock::insertTextBlock(TextBlock * newBlock, TextBlock * existingBlock, bool after)
{
    if (!newBlock || !existingBlock)
        return false;

    QString tag = existingBlock->getTag();
    // FIX: just do a quick check if the parent DOCUMENT has this.
    // IF it does, then the lack of an index will force us into
    // a search of any child hierarchical codeblocks we may have
    // Its not efficient, but works. I don't think speed is a problem
    // right now for the current implementation, but in the future
    // when code import/roundtripping is done, it *may* be. -b.t.
    if (!getParentDocument()->findTextBlockByTag(tag, true))
        return false;

    int index = m_textblockVector.indexOf(existingBlock);
    if (index < 0)
    {
        // may be hiding in child hierarchical codeblock
        foreach (TextBlock* tb, m_textblockVector)
        {
            HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock*>(tb);
            if (hb && hb->insertTextBlock(newBlock, existingBlock, after))
                return true; // found, and inserted, otherwise keep going
        }
        uWarning() << "Warning: couldnt insert text block (tag:" << newBlock->getTag() <<
                      "). Reference text block (tag:" << existingBlock->getTag() << ") not found.";
        return false;
    }

    // if we get here.. it was in this object so insert

    // check for tag FIRST
    QString new_tag = newBlock->getTag();

    // assign a tag if one doesn't already exist
    if (new_tag.isEmpty())
    {
        new_tag = getUniqueTag();
        newBlock->setTag(new_tag);
    }

    if (m_textBlockTagMap.contains(new_tag)) {
        return false; // return false, we already have some object with this tag in the list
    }
    else {
        m_textBlockTagMap.insert(new_tag, newBlock);
        getParentDocument()->addChildTagToMap(new_tag, newBlock);
    }

    if(after)
        index++;

    m_textblockVector.insert(index,newBlock);

    return true;
}

/**
 * Remove a TextBlock object from m_textblockVector List
 * returns boolean - true if successful
 */
bool HierarchicalCodeBlock::removeTextBlock ( TextBlock * remove_object )
{
    // try to remove from the list in this object
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
                return true; // because we got in child hb;
        }
        return false;
    }

    // IF we get here, the text block was in THIS object (and not a child)..
    QString tag = remove_object->getTag();
    if (!(tag.isEmpty())) {
        m_textBlockTagMap.remove(tag);
        getParentDocument()->removeChildTagFromMap(tag);
    }
    return true;
}

/**
 * @param   text
 */
void HierarchicalCodeBlock::setStartText ( const QString &text )
{
    m_startText = text;
}

/**
 * @return  QString
 */
QString HierarchicalCodeBlock::getStartText ( ) const
{
    return m_startText;
}

/**
 * Utility method to add accessormethods in this object
 */
void HierarchicalCodeBlock::addCodeClassFieldMethods(CodeClassFieldList &list )
{
    CodeClassFieldList::Iterator it = list.begin();
    CodeClassFieldList::Iterator end = list.end();
    for ( ; it != end; ++it )
    {
        CodeClassField * field = *it;
        CodeAccessorMethodList list = field->getMethodList();
        Q_FOREACH( CodeAccessorMethod *method, list )
        {
            QString tag = method->getTag();
            if (tag.isEmpty())
            {
                tag = getUniqueTag();
                method->setTag(tag);
            }

            addTextBlock(method); // wont add if already exists in object;
        }
    }
}

/**
 * Save the XMI representation of this object
 */
void HierarchicalCodeBlock::saveToXMI ( QDomDocument & doc, QDomElement & root )
{
    QDomElement blockElement = doc.createElement( "hierarchicalcodeblock" );
    setAttributesOnNode(doc, blockElement);
    root.appendChild( blockElement );
}

/**
 * set attributes of the node that represents this class
 * in the XMI document.
 */
void HierarchicalCodeBlock::setAttributesOnNode (QDomDocument & doc, QDomElement & elem )
{
    // set super-class attributes
    CodeBlockWithComments::setAttributesOnNode(doc, elem);
    CodeGenObjectWithTextBlocks::setAttributesOnNode(doc, elem);

    // set local class attributes
    if (contentType() != CodeBlock::AutoGenerated)
    {
        QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
        elem.setAttribute("startText",encodeText(getStartText(),endLine));
        elem.setAttribute("endText",encodeText(getEndText(),endLine));
    }
}

/**
 * load params from the appropriate XMI element node.
 */
void HierarchicalCodeBlock::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

/**
 * set the class attributes of this object from
 * the passed element node.
 */
void HierarchicalCodeBlock::setAttributesFromNode ( QDomElement & root)
{
    // set attributes from the XMI
    CodeBlockWithComments::setAttributesFromNode(root); // superclass load

    if (contentType() != CodeBlock::AutoGenerated)
    {
        QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
        setStartText(decodeText(root.attribute("startText",""),endLine));
        setEndText(decodeText(root.attribute("endText",""),endLine));
    }

    // do this *after* all other attributes saved
    CodeGenObjectWithTextBlocks::setAttributesFromNode(root);
}

/**
 * set the class attributes from a passed object
 */
void HierarchicalCodeBlock::setAttributesFromObject (TextBlock * obj)
{
    CodeBlockWithComments::setAttributesFromObject(obj);

    HierarchicalCodeBlock * hb = dynamic_cast<HierarchicalCodeBlock*>(obj);
    if (hb) {
        setStartText(hb->getStartText());
        setEndText(hb->getEndText());
        CodeGenObjectWithTextBlocks *cgowtb = dynamic_cast<CodeGenObjectWithTextBlocks*>(obj);
        CodeGenObjectWithTextBlocks::setAttributesFromObject(cgowtb);
    }
}

/**
 * @return  QString
 */
QString HierarchicalCodeBlock::toString() const
{
    QString string;

    if (getWriteOutText()) {
        QString indent = getIndentationString();
        QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
        QString startText;
        QString endText;
        if (!getStartText().isEmpty())
            startText = formatMultiLineText (getStartText(), indent, endLine);
        if (!getEndText().isEmpty())
            endText = formatMultiLineText (getEndText(), indent, endLine);

        QString body = childTextBlocksToString();
        QString comment = getComment()->toString();

        // tack in text, if there is something there..
        if (!comment.isEmpty() && getComment()->getWriteOutText())
            string.append(comment);

        if (!startText.isEmpty())
            string.append(startText);

        if (!body.isEmpty())
            string.append(body);

        if (!endText.isEmpty())
            string.append(endText);
    }
    return string;
}

QString  HierarchicalCodeBlock::childTextBlocksToString() const
{
    TextBlockList* list = getTextBlockList();
    QString retString;
    foreach (TextBlock* block, *list)
    {
        QString blockValue = block->toString();
        if (!blockValue.isEmpty())
            retString.append(blockValue);
    }
    return retString;
}

/**
 * look for specific text blocks which belong to code classfields
 */
TextBlock * HierarchicalCodeBlock::findCodeClassFieldTextBlockByTag ( const QString &tag )
{
    ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(getParentDocument());
    if(cdoc)
        return cdoc->findCodeClassFieldTextBlockByTag(tag);
    else
        uError() << "HierarchicalCodeBlock: findCodeClassFieldTextBlockByTag() finds NO parent document! Badly constructed textblock?!?";

    // if we get here, we failed.
    return (TextBlock*) NULL;
}

void HierarchicalCodeBlock::initAttributes()
{
    setCanDelete(false);
    m_startText.clear();
    m_endText.clear();
}

/**
 * causes the text block to release all of its connections
 * and any other text blocks that it 'owns'.
 * needed to be called prior to deletion of the textblock.
 */
void HierarchicalCodeBlock::release()
{
    resetTextBlocks();
    TextBlock::release();
}
