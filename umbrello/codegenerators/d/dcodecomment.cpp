/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dcodecomment.h"

// qt/kde includes
#include <QRegularExpression>

DCodeComment::DCodeComment (CodeDocument * doc, const QString & text)
        : CodeComment (doc, text)
{
}

DCodeComment::~DCodeComment ()
{
}

void DCodeComment::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("dcodecomment"));

    // as we added no additional fields to this class we may
    // just use parent TextBlock method
    setAttributesOnNode(writer);
    writer.writeEndElement();
}

QString DCodeComment::toString () const
{
    QString output;

    // simple output method
    if(getWriteOutText())
    {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        QString body = getText();

        // check the need for multiline comments
        if (body.indexOf(QRegularExpression(endLine)) >= 0) {
            output += indent + QStringLiteral("/**") + endLine;
            output += formatMultiLineText (body, indent + QStringLiteral(" * "), endLine);
            output += indent + QStringLiteral(" */") + endLine;
        } else {
            output += formatMultiLineText (body, indent + QStringLiteral("// "), endLine);
        }
    }

    return output;
}

// TODO: where is this used?
QString DCodeComment::getNewEditorLine (int amount)
{
    QString line = getIndentationString(amount) + QStringLiteral("// ");
    return line;
}

// TODO: where is this used?
QString DCodeComment::unformatText (const QString & text, const QString & indent)
{
    // remove leading or trailing comment stuff
    QString mytext = TextBlock::unformatText(text, indent);

    // now leading slashes
    mytext.remove(QRegularExpression(QStringLiteral("^\\/\\/\\s*")));
    return mytext;
}
