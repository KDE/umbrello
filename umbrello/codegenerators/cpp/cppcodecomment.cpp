/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppcodecomment.h"

// qt includes
#include <QRegularExpression>


CPPCodeComment::CPPCodeComment (CodeDocument * doc, const QString & text)
        : CodeComment (doc, text)
{
}

CPPCodeComment::~CPPCodeComment ()
{
}

void CPPCodeComment::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("cppcodecomment"));
    setAttributesOnNode(writer); // as we added no additional fields to this class we may
    // just use parent TextBlock method
    writer.writeEndElement();
}

QString CPPCodeComment::toString () const
{
    QString output;

    // simple output method
    if(getWriteOutText())
    {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        output.append(formatMultiLineText (getText() + endLine, indent + QStringLiteral("// "), endLine));
    }

    return output;
}

QString CPPCodeComment::getNewEditorLine (int amount)
{
    QString line = getIndentationString(amount) + QStringLiteral("// ");
    return line;
}

QString CPPCodeComment::unformatText (const QString & text, const QString & indent)
{
    // remove leading or trailing comment stuff
    QString mytext = TextBlock::unformatText(text, indent);

    // now leading slashes
    mytext.remove(QRegularExpression(QStringLiteral("^\\/\\/\\s*")));
    return mytext;
}
