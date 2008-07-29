/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "textblock.h"

// local includes
#include "codedocument.h"
#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "uml.h"

// qt includes
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

TextBlock::TextBlock ( CodeDocument * parent, const QString & text )
{
    m_canDelete = true;
    m_writeOutText = true;
    m_parentDocument = parent;
    m_text = "";
    m_tag = "";
    m_indentationLevel = 0;
    setText(text);
}

TextBlock::~TextBlock ()
{
}

void TextBlock::setCanDelete(bool canDelete)
{
    m_canDelete = canDelete;
}

bool TextBlock::canDelete() const
{
    return m_canDelete;
}

CodeDocument * TextBlock::getParentDocument() const
{
    return m_parentDocument;
}

void TextBlock::setText ( const QString & text )
{
    m_text = text;
}

void TextBlock::appendText ( const QString & text )
{
    m_text = m_text + text;
}

QString TextBlock::getText() const
{
    return m_text;
}

QString TextBlock::getTag() const
{
    return m_tag;
}

void TextBlock::setTag ( const QString & value )
{
    m_tag = value;
}

void TextBlock::setWriteOutText ( bool write )
{
    m_writeOutText = write;
}

bool TextBlock::getWriteOutText() const
{
    return m_writeOutText;
}

void TextBlock::setIndentationLevel ( int level )
{
    m_indentationLevel = level;
}

int TextBlock::getIndentationLevel() const
{
    return m_indentationLevel;
}

QString TextBlock::getNewLineEndingChars()
{
    CodeGenerationPolicy* policy = UMLApp::app()->getCommonPolicy();
    return policy->getNewLineEndingChars();
}

QString TextBlock::getIndentation()
{
    CodeGenerationPolicy* policy = UMLApp::app()->getCommonPolicy();
    return policy->getIndentation();
}

QString TextBlock::getIndentationString ( int level ) const
{
    if (!level)
        level = m_indentationLevel;
    QString indentAmount = getIndentation();
    QString indentation;
    for(int i=0; i<level; i++)
        indentation.append(indentAmount);
    return indentation;
}

int TextBlock::firstEditableLine()
{
    return 0;
}

int TextBlock::lastEditableLine()
{
    return 0;
}

QString TextBlock::getNewEditorLine ( int amount )
{
    return getIndentationString(amount);
}

QString TextBlock::unformatText ( const QString & text, const QString & indent )
{
    QString output = text;
    QString myIndent = indent;
    if (myIndent.isEmpty())
        myIndent = getIndentationString();

    if (!output.isEmpty())
        // remove indenation from this text block.
        output.remove(QRegExp('^' + myIndent));

    return output;
}

void TextBlock::release()
{
}

QString TextBlock::formatMultiLineText ( const QString & work, const QString & linePrefix,
        const QString & breakStr, bool addBreak, bool lastLineHasBreak )
{
    QString output;
    QString text = work;
    QString endLine = getNewLineEndingChars();
    int matches = text.indexOf(QRegExp(breakStr));
    if (matches >= 0)
    {
        // check that last part of string matches, if not, then
        // we have to tack on extra match
        if (!text.contains(QRegExp(breakStr + "\\$")))
            matches++;

        for (int i=0; i < matches; i++)
        {
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

void TextBlock::setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement)
{
    QString endLine = UMLApp::app()->getCommonPolicy()->getNewLineEndingChars();

    if (&doc != 0 ) {

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
}

void TextBlock::setAttributesFromObject(TextBlock * obj)
{
    // DON'T set tag here.
    setIndentationLevel(obj->getIndentationLevel());
    setText(obj->getText());
    setWriteOutText(obj->getWriteOutText());
    m_canDelete = obj->canDelete();
}

void TextBlock::setAttributesFromNode(QDomElement & root)
{
    QString endLine = UMLApp::app()->getCommonPolicy()->getNewLineEndingChars();

    setIndentationLevel(root.attribute("indentLevel", "0").toInt());
    setTag(root.attribute("tag", ""));
    setText(decodeText(root.attribute("text", ""), endLine));
    setWriteOutText(root.attribute("writeOutText", "true") == "true" ? true : false);
    m_canDelete = root.attribute("canDelete", "true") == "true" ? true : false;
}

QString TextBlock::encodeText(const QString & text, const QString & endLine)
{
    QString encoded = text;
    encoded.replace(QRegExp(endLine), "&#010;");
    return encoded;
}

QString TextBlock::decodeText(const QString & text, const QString & endLine)
{
    QString decoded = text;
    decoded.replace(QRegExp("&#010;"), endLine);
    return decoded;
}

QString TextBlock::toString() const
{
    // simple output method
    if (m_writeOutText && !m_text.isEmpty())
    {
        QString endLine = UMLApp::app()->getCommonPolicy()->getNewLineEndingChars();
        return formatMultiLineText(m_text, getIndentationString(), endLine);
    } else
        return QString();
}

QTextStream& operator<<(QTextStream& os, const TextBlock& obj)
{
    os << "TextBlock: tag=" << obj.getTag()
       << ", writeOutText=" << (obj.getWriteOutText() ? "true" : "false")
       << ", canDelete=" << (obj.canDelete() ? "true" : "false")
       << ", indentationLevel=" << obj.getIndentationLevel()
       << ", parentDocument id=" << (obj.getParentDocument() ? obj.getParentDocument()->getID() : "null")
       << ", text=" << obj.getText();
    return os;
}
