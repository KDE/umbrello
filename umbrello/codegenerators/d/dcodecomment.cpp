/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   copyright (C) 2008-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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

void DCodeComment::saveToXMI1 (QDomDocument & doc, QDomElement & root)
{
    QDomElement blockElement = doc.createElement(QLatin1String("dcodecomment"));

    // as we added no additional fields to this class we may
    // just use parent TextBlock method
    setAttributesOnNode(doc, blockElement);
    root.appendChild(blockElement);
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
