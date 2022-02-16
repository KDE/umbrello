/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "javacodecomment.h"

#include <QRegExp>

JavaCodeComment::JavaCodeComment (CodeDocument * doc, const QString & text)
        : CodeComment (doc, text)
{
}

JavaCodeComment::~JavaCodeComment ()
{
}

QString JavaCodeComment::getNewEditorLine (int amount)
{
    QString line = getIndentationString(amount) + QLatin1String("// ");
    return line;
}

/** UnFormat a long text string. Typically, this means removing
 *  the indentation (linePrefix) and/or newline chars from each line.
 */
QString JavaCodeComment::unformatText (const QString & text, const QString & indent)
{
    // remove leading or trailing comment stuff
    QString mytext = TextBlock::unformatText(text, indent);

    // now leading slashes
    mytext.remove(QRegExp(QLatin1String("^\\/\\/\\s*")));
    return mytext;
}

/**
 * @return      QString
 */
QString JavaCodeComment::toString () const
{
    QString output;

    // simple output method
    if (getWriteOutText()) {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        output.append(formatMultiLineText (getText(), indent + QLatin1String("// "), endLine));
    }

    return output;
}
