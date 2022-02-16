/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "rubycodedocumentation.h"

// local includes
#include "rubyclassifiercodedocument.h"
#include "rubycodegenerationpolicy.h"
#include "uml.h"

// qt includes
#include <QRegExp>

RubyCodeDocumentation::RubyCodeDocumentation(RubyClassifierCodeDocument * doc, const QString & text)
  : CodeComment((CodeDocument*) doc, text)
{
}

RubyCodeDocumentation::~RubyCodeDocumentation()
{
}

void RubyCodeDocumentation::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("rubycodedocumentation"));
    setAttributesOnNode(writer); // as we added no additional fields to this class we may
    // just use parent TextBlock method
    writer.writeEndElement();
}

QString RubyCodeDocumentation::toString() const
{
    QString output;

    // simple output method
    if (getWriteOutText())
    {
        bool  useHashOutput = true;

        // need to figure out output type from ruby policy
        CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
        if (p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
             useHashOutput = false;

        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        QString body = getText();
        if (useHashOutput)
        {
            if (!body.isEmpty())
                output.append(formatMultiLineText (body, indent + QLatin1String("# "), endLine));
        } else {
            output.append(QLatin1String("=begin rdoc") + endLine);
            output.append(formatMultiLineText (body, indent + QLatin1Char(' '), endLine));
            output.append(QLatin1String("=end") + endLine);
        }
    }

    return output;
}

QString RubyCodeDocumentation::getNewEditorLine(int amount)
{
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    if (p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
        return getIndentationString(amount) + QLatin1Char(' ');
    else
        return getIndentationString(amount) + QLatin1String("# ");
}

int RubyCodeDocumentation::firstEditableLine()
{
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    if (p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
        return 1;
    return 0;
}

int RubyCodeDocumentation::lastEditableLine()
{
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    if (p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
    {
        return -1; // very last line is NOT editable
    }
    return 0;
}

QString RubyCodeDocumentation::unformatText(const QString & text, const QString & indent)
{
    QString mytext = TextBlock::unformatText(text, indent);
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    // remove leading or trailing comment stuff
    mytext.remove(QRegExp(QLatin1Char('^') + indent));
    if (p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
    {
        mytext.remove(QRegExp(QLatin1String("^=begin\\s*(rdoc)?\\s*\n?")));
        mytext.remove(QRegExp(QLatin1String("^=end\\s*\n?$")));
    } else
        mytext.remove(QRegExp(QLatin1String("^#\\s*")));

    return mytext;
}
