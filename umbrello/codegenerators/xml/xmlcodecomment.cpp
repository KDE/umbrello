/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "xmlcodecomment.h"

XMLCodeComment::XMLCodeComment (CodeDocument * doc, const QString & text)
  : CodeComment (doc, text)
{
}

XMLCodeComment::~XMLCodeComment ()
{
}

QString XMLCodeComment::toString () const
{
    QString output;

    // simple output method
    if (getWriteOutText())
    {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        QString body = getText();
        output.append(indent + QLatin1String("<!-- "));
        if (!body.isEmpty()) {
            output.append(formatMultiLineText (body, indent, endLine));
        }
        output.append(indent + QLatin1String("-->") + endLine);
    }

    return output;
}
