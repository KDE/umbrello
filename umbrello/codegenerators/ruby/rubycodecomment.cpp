/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "rubycodecomment.h"

#include <QRegularExpression>

RubyCodeComment::RubyCodeComment(CodeDocument * doc, const QString & text)
        : CodeComment(doc, text)
{
}

RubyCodeComment::~RubyCodeComment()
{
}

QString RubyCodeComment::getNewEditorLine(int amount)
{
    QString line = getIndentationString(amount) + QStringLiteral("# ");
    return line;
}

QString RubyCodeComment::unformatText(const QString & text, const QString & indent)
{
    // remove leading or trailing comment stuff
    QString mytext = TextBlock::unformatText(text, indent);

    // now leading hash
    mytext.remove(QRegularExpression(QStringLiteral("^#\\s*")));
    return mytext;
}

QString RubyCodeComment::toString() const
{
    QString output;

    // simple output method
    if (getWriteOutText()) {
        QString indent = getIndentationString();
        QString endLine = getNewLineEndingChars();
        output.append(formatMultiLineText(getText(), indent + QStringLiteral("# "), endLine + endLine));
    }

    return output;
}
