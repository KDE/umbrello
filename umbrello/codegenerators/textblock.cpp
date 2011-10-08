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
#include "textblock.h"

// local includes
#include "codedocument.h"
#include "codegenerationpolicy.h"
#include "debug_utils.h"
#include "uml.h"

// qt includes
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

/**
 * Constructor.
 */
TextBlock::TextBlock(CodeDocument * parent, const QString & text)
  : m_text(QString()),
    m_tag(QString()),
    m_canDelete(true),
    m_writeOutText(true),
    m_indentationLevel(0),
    m_parentDocument(parent)
{
    setText(text);
}

/**
 * Destructor.
 */
TextBlock::~TextBlock()
{
}

/**
 * Set the attribute m_canDelete.
 * @param canDelete   the new value to set
 */
void TextBlock::setCanDelete(bool canDelete)
{
    m_canDelete = canDelete;
}

/**
 * Determine if its OK to delete this textblock from the document.
 * Used by the text editor to know if deletion could cause a crash of
 * the program.
 * @return   the value of m_canDelete
 */
bool TextBlock::canDelete() const
{
    return m_canDelete;
}

/**
 * Get the value of m_parentDoc
 * @return the value of m_parentDoc
 */
CodeDocument * TextBlock::getParentDocument() const
{
    return m_parentDocument;
}

/**
 * Set the value of m_text
 * The actual text of this code block.
 * @param text   the new value of m_text
 */
void TextBlock::setText(const QString & text)
{
    m_text = text;
}

/**
 * Add text to this object.
 * @param text   the text to add
 */
void TextBlock::appendText(const QString & text)
{
    m_text = m_text + text;
}

/**
 * Get the value of m_text
 * The actual text of this code block.
 * @return   the value of m_text
 */
QString TextBlock::getText() const
{
    return m_text;
}

/**
 * Get the tag of this text block. This tag
 * may be used to find this text block in the code document
 * to which it belongs.
 * @return   the tag
 */
QString TextBlock::getTag() const
{
    return m_tag;
}

/**
 * Set the tag of this text block. This tag
 * may be used to find this text block in the code document
 * to which it belongs.
 * @param value   the new value for the tag
 */
void TextBlock::setTag(const QString & value)
{
    m_tag = value;
}

/**
 * Set the value of m_writeOutText
 * Whether or not to include the text of this TextBlock into a file.
 * @param write   the new value of m_writeOutText
 */
void TextBlock::setWriteOutText(bool write)
{
    m_writeOutText = write;
}

/**
 * Get the value of m_writeOutText
 * Whether or not to include the text of this TextBlock into a file.
 * @return the value of m_writeOutText
 */
bool TextBlock::getWriteOutText() const
{
    return m_writeOutText;
}

/**
 * Set how many times to indent this text block.
 * The amount of each indenatation is determined from the parent
 * codedocument codegeneration policy.
 * @param level   the new value for the indentation level
 */
void TextBlock::setIndentationLevel(int level)
{
    m_indentationLevel = level;
}

/**
 * Get how many times to indent this text block.
 * The amount of each indenatation is determined from the parent
 * codedocument codegeneration policy.
 * @return   the indentation level
 */
int TextBlock::getIndentationLevel() const
{
    return m_indentationLevel;
}

/**
 * Get the new line chars which ends the line.
 * @return   the ending chars for new line
 */
QString TextBlock::getNewLineEndingChars()
{
    CodeGenerationPolicy* policy = UMLApp::app()->commonPolicy();
    return policy->getNewLineEndingChars();
}

/**
 * Get how much a single "level" of indentation will actually indent.
 * @return   the unit of indentation (for one level)
 */
QString TextBlock::getIndentation()
{
    CodeGenerationPolicy* policy = UMLApp::app()->commonPolicy();
    return policy->getIndentation();
}

/**
 * Get the actual amount of indentation for a given level of indentation.
 * @param level   the level of interest
 * @return        the indentation string
 */
QString TextBlock::getIndentationString(int level) const
{
    if (!level) {
        level = m_indentationLevel;
    }
    QString indentAmount = getIndentation();
    QString indentation;
    for (int i=0; i<level; ++i) {
        indentation.append(indentAmount);
    }
    return indentation;
}

/**
 * TODO: Ush. These are terrifically bad and must one day go away.
 * Both methods indicate the range of lines in this textblock
 * which may be edited by the codeeditor (assuming that any are
 * actually editable). The default case is no lines are editable.
 * The line numbering starts with '0' and a '-1' means no line
 * qualifies.
 * @return   line number
 */
int TextBlock::firstEditableLine()
{
    return 0;
}

/**
 * @see firstEditableLine
 */
int TextBlock::lastEditableLine()
{
    return 0;
}

/**
 * Used by the CodeEditor. It provides it with an appropriate
 * starting string for a new line of text within the given textblock
 * (for example a string with the proper indentation).
 * If the indentation amount is '0' the current indentation string will
 * be used.
 * <p>
 * TODO: Can be refactored away and replaced with
 * <a href="#getIndentationString">getIndentationString</a>.
 * @param amount   the number of indent steps to use
 * @return         the new line
 */
QString TextBlock::getNewEditorLine(int amount)
{
    return getIndentationString(amount);
}

/**
 * UnFormat a long text string. Typically, this means removing
 * the indentaion (linePrefix) and/or newline chars from each line.
 * If an indentation is not specified, then the current indentation is used.
 * @param text     the original text for unformatting
 * @param indent   the indentation
 * @return         the unformatted text
 */
QString TextBlock::unformatText(const QString & text, const QString & indent)
{
    QString output = text;
    QString myIndent = indent;
    if (myIndent.isEmpty()) {
        myIndent = getIndentationString();
    }

    if (!output.isEmpty()) {
        // remove indenation from this text block.
        output.remove(QRegExp('^' + myIndent));
    }

    return output;
}

/**
 * Causes the text block to release all of its connections
 * and any other text blocks that it 'owns'.
 * Needed to be called prior to deletion of the textblock.
 * TODO: Does nothing.
 */
void TextBlock::release()
{
}

/**
 * Format a long text string to be more readable.
 * @param work               the original text for formatting
 * @param linePrefix         a line prefix
 * @param breakStr           a break string
 * @param addBreak           control to add always a break string
 * @param lastLineHasBreak   control to add a break string to the last line
 * @return                   the new formatted text
 */
QString TextBlock::formatMultiLineText(const QString & work, const QString & linePrefix,
        const QString & breakStr, bool addBreak, bool lastLineHasBreak)
{
    QString output;
    QString text = work;
    QString endLine = getNewLineEndingChars();
    int matches = text.indexOf(QRegExp(breakStr));
    if (matches >= 0) {
        // check that last part of string matches, if not, then
        // we have to tack on extra match
        if (!text.contains(QRegExp(breakStr + "\\$")))
            matches++;

        for (int i=0; i < matches; ++i) {
            QString line = text.section(QRegExp(breakStr),i,i);
            output += linePrefix + line;
            if ((i != matches-1) || lastLineHasBreak)
                output += endLine; // add break to line
        }
    } else {
        output = linePrefix + text;
        if (addBreak)
            output += breakStr;
    }

    return output;
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 * @param doc            the xmi document
 * @param blockElement   the xmi element holding the attributes
 */
void TextBlock::setAttributesOnNode(QDomDocument & doc, QDomElement & blockElement)
{
    Q_UNUSED(doc);

    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    blockElement.setAttribute("tag",getTag());

    // only write these if different from defaults
    if (getIndentationLevel())
        blockElement.setAttribute("indentLevel", QString::number(getIndentationLevel()));
    if (!m_text.isEmpty())
        blockElement.setAttribute("text", encodeText(m_text, endLine));
    if (!getWriteOutText())
        blockElement.setAttribute("writeOutText", getWriteOutText()?"true":"false");
    if (!canDelete())
        blockElement.setAttribute("canDelete", canDelete()?"true":"false");
}

/**
 * Set the class attributes from a passed object.
 * @param obj   text block from which the attributes are taken
 */
void TextBlock::setAttributesFromObject(TextBlock * obj)
{
    // DON'T set tag here.
    setIndentationLevel(obj->getIndentationLevel());
    setText(obj->getText());
    setWriteOutText(obj->getWriteOutText());
    m_canDelete = obj->canDelete();
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 * @param root   the xmi element from which to load
 */
void TextBlock::setAttributesFromNode(QDomElement & root)
{
    QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();

    setIndentationLevel(root.attribute("indentLevel", "0").toInt());
    setTag(root.attribute("tag", ""));
    setText(decodeText(root.attribute("text", ""), endLine));
    setWriteOutText(root.attribute("writeOutText", "true") == "true" ? true : false);
    m_canDelete = root.attribute("canDelete", "true") == "true" ? true : false;
}

/**
 * Encode text for XML storage.
 * We simply convert all types of newLines to the "\n" or &#010;
 * entity.
 * @param text      the not yet encoded text
 * @param endLine   the chars at the end of each line
 * @return          the encoded text
 */
QString TextBlock::encodeText(const QString & text, const QString & endLine)
{
    QString encoded = text;
    encoded.replace(QRegExp(endLine), "&#010;");
    return encoded;
}

/**
 * Decode text from XML storage.
 * We simply convert all newLine entity &#010; to chosen line ending.
 * @param text      the not yet decoded text
 * @param endLine   the chars at the end of each line
 * @return          the decoded text
 */
QString TextBlock::decodeText(const QString & text, const QString & endLine)
{
    QString decoded = text;
    decoded.replace(QRegExp("&#010;"), endLine);
    return decoded;
}

/**
 * Return the text in the right format. Returned string is empty
 * if m_writeOutText is false.
 * @return  QString
 */
QString TextBlock::toString() const
{
    // simple output method
    if (m_writeOutText && !m_text.isEmpty()) {
        QString endLine = UMLApp::app()->commonPolicy()->getNewLineEndingChars();
        return formatMultiLineText(m_text, getIndentationString(), endLine);
    }
    else {
        return QString();
    }
}

/**
 * Operator '<<' for TextBlock.
 */
QDebug operator<<(QDebug os, const TextBlock& obj)
{
    os.nospace() << "TextBlock: tag=" << obj.getTag()
       << ", writeOutText=" << (obj.getWriteOutText() ? "true" : "false")
       << ", canDelete=" << (obj.canDelete() ? "true" : "false")
       << ", indentationLevel=" << obj.getIndentationLevel()
       << ", parentDocument id=" << (obj.getParentDocument() ? obj.getParentDocument()->getID() : "null")
       << ", text=" << obj.getText();
    return os.space();
}
