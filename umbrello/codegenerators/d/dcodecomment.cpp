/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "dcodecomment.h"

// qt/kde includes
#include <QRegExp>

DCodeComment::DCodeComment (CodeDocument * doc, const QString & text)
        : CodeComment (doc, text)
{
}

DCodeComment::~DCodeComment ()
{
}

void DCodeComment::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QLatin1String("dcodecomment"));

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
        if (body.indexOf(QRegExp(endLine)) >= 0) {
            output += indent + QLatin1String("/**") + endLine;
            output += formatMultiLineText (body, indent + QLatin1String(" * "), endLine);
            output += indent + QLatin1String(" */") + endLine;
        } else {
            output += formatMultiLineText (body, indent + QLatin1String("// "), endLine);
        }
    }

    return output;
}

// TODO: where is this used?
QString DCodeComment::getNewEditorLine (int amount)
{
    QString line = getIndentationString(amount) + QLatin1String("// ");
    return line;
}

// TODO: where is this used?
QString DCodeComment::unformatText (const QString & text, const QString & indent)
{
    // remove leading or trailing comment stuff
    QString mytext = TextBlock::unformatText(text, indent);

    // now leading slashes
    mytext.remove(QRegExp(QLatin1String("^\\/\\/\\s*")));
    return mytext;
}
