/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "javacodedocumentation.h"

// local includes
#include "codegenerationpolicy.h"
#include "javaclassifiercodedocument.h"
#include "uml.h"

// qt includes
#include <QRegExp>

JavaCodeDocumentation::JavaCodeDocumentation(JavaClassifierCodeDocument * doc, const QString & text)
  : CodeComment(doc, text)
{
}

JavaCodeDocumentation::~JavaCodeDocumentation()
{
}

/**
 * Save the XMI representation of this object
 */
void JavaCodeDocumentation::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("javacodedocumentation"));
    setAttributesOnNode(writer); // as we added no additional fields to this class we may
    // just use parent TextBlock method
    writer.writeEndElement();
}

/**
 * @return      QString
 */
QString JavaCodeDocumentation::toString() const
{
    QString output;

    // simple output method
    if(getWriteOutText())
    {
        bool useDoubleDashOutput = true;

        // need to figure out output type from java policy
        CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
        if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
            useDoubleDashOutput = false;

        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        QString body = getText();
        if(useDoubleDashOutput)
        {
            if(!body.isEmpty())
                output.append(formatMultiLineText (body, indent + QLatin1String("// "), endLine));
        } else {
            output.append(indent + QLatin1String("/**") + endLine);
            output.append(formatMultiLineText (body, indent + QLatin1String(" * "), endLine));
            output.append(indent + QLatin1String(" */") + endLine);
        }
    }

    return output;
}

QString JavaCodeDocumentation::getNewEditorLine(int amount)
{
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
        return getIndentationString(amount) + QLatin1String(" * ");
    else
        return getIndentationString(amount) + QLatin1String("// ");
}

int JavaCodeDocumentation::firstEditableLine()
{
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
        return 1;
    return 0;
}

int JavaCodeDocumentation::lastEditableLine()
{
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
    {
        return -1; // very last line is NOT editable
    }
    return 0;
}

/** UnFormat a long text string. Typically, this means removing
 *  the indentation (linePrefix) and/or newline chars from each line.
 */
QString JavaCodeDocumentation::unformatText(const QString & text, const QString & indent)
{
    QString mytext = TextBlock::unformatText(text, indent);
    CodeGenerationPolicy *p = UMLApp::app()->commonPolicy();
    // remove leading or trailing comment stuff
    mytext.remove(QRegExp(QLatin1Char('^') + indent));
    if(p->getCommentStyle() == CodeGenerationPolicy::MultiLine)
    {
        mytext.remove(QRegExp(QLatin1String("^\\/\\*\\*\\s*\n?")));
        mytext.remove(QRegExp(QLatin1String("\\s*\\*\\/\\s*\n?$")));
        mytext.remove(QRegExp(QLatin1String("^\\s*\\*\\s*")));
    } else
        mytext.remove(QRegExp(QLatin1String("^\\/\\/\\s*")));

    return mytext;
}
