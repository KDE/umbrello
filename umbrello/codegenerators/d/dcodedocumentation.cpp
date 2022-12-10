/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dcodedocumentation.h"

// app includes
#include "codedocument.h"
#include "codegenerator.h"
#include "codegenerationpolicy.h"
#include "uml.h"

// qt/kde includes
#include <QRegExp>

DCodeDocumentation::DCodeDocumentation(CodeDocument * doc, const QString & text)
  : CodeComment(doc, text)
{
}

DCodeDocumentation::~DCodeDocumentation()
{
}

void DCodeDocumentation::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("dcodedocumentation"));
    setAttributesOnNode(writer); // as we added no additional fields to this class we may
    // just use parent TextBlock method
    writer.writeEndElement();
}

QString DCodeDocumentation::toString() const
{
    QString output;

    // simple output method
    if(getWriteOutText())
    {
        bool useDoubleDashOutput = true;

        // need to figure out output type from d policy
        CodeGenerationPolicy * p = UMLApp::app()->commonPolicy();
        if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
            useDoubleDashOutput = false;

        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        QString body = getText();

        if(useDoubleDashOutput)
        {
            if(!body.isEmpty()) {
                output += (formatMultiLineText (body, indent + QStringLiteral("// "), endLine));
            }
        } else {
            output += indent + QStringLiteral("/**") + endLine;
            output += formatMultiLineText (body, indent + QStringLiteral(" * "), endLine);
            output += indent + QStringLiteral(" */") + endLine;
        }
    }

    return output;
}

QString DCodeDocumentation::getNewEditorLine(int amount)
{
    CodeGenerationPolicy * p = UMLApp::app()->commonPolicy();
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
        return getIndentationString(amount) + QStringLiteral(" * ");
    else
        return getIndentationString(amount) + QStringLiteral("// ");
}

int DCodeDocumentation::firstEditableLine()
{
    CodeGenerationPolicy * p = UMLApp::app()->commonPolicy();
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
        return 1;
    return 0;
}

int DCodeDocumentation::lastEditableLine()
{
    CodeGenerationPolicy * p = UMLApp::app()->commonPolicy();
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
    {
        return -1; // very last line is NOT editable
    }
    return 0;
}

/**
 * UnFormat a long text string. Typically, this means removing
 * the indentation (linePrefix) and/or newline chars from each line.
 */
QString DCodeDocumentation::unformatText(const QString & text, const QString & indent)
{
    QString mytext = TextBlock::unformatText(text, indent);
    CodeGenerationPolicy * p = UMLApp::app()->commonPolicy();
    // remove leading or trailing comment stuff
    mytext.remove(QRegExp(QLatin1Char('^') + indent));
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
    {
        mytext.remove(QRegExp(QStringLiteral("^\\/\\*\\*\\s*\n?")));
        mytext.remove(QRegExp(QStringLiteral("\\s*\\*\\/\\s*\n?$")));
        mytext.remove(QRegExp(QStringLiteral("^\\s*\\*\\s*")));
    } else
        mytext.remove(QRegExp(QStringLiteral("^\\/\\/\\s*")));

    return mytext;
}
