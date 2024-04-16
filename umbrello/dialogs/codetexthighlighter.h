/*
    CodeTextHighlighter: Syntax highlighter for the CodeTextEdit widget.
    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>
    Code based on examples of the Qt Toolkit under BSD license,
    <http://doc.qt.nokia.com/4.6/richtext-syntaxhighlighter.html>.
    SPDX-FileCopyrightText: 2010 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CODETEXTHIGHLIGHTER_H
#define CODETEXTHIGHLIGHTER_H

#include <QHash>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

class CodeTextHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit CodeTextHighlighter(QTextDocument  *parent = nullptr);

protected:
    void highlightBlock(const QString &text);

private:
    QStringList keywords();

    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> m_highlightingRules;

    QRegExp m_commentStartExpression;
    QRegExp m_commentEndExpression;

    QTextCharFormat m_keywordFormat;
    QTextCharFormat m_classFormat;
    QTextCharFormat m_singleLineCommentFormat;
    QTextCharFormat m_multiLineCommentFormat;
    QTextCharFormat m_quotationFormat;
    QTextCharFormat m_functionFormat;
};

#endif // CODETEXTHIGHLIGHTER_H
