/*
    CodeTextHighlighter: Syntax highlighter for the CodeTextEdit widget.
    Copyright 2010  Nokia Corporation and/or its subsidiary(-ies) <qt-info@nokia.com>
                    Code based on examples of the Qt Toolkit under BSD license,
                    <http://doc.qt.nokia.com/4.6/richtext-syntaxhighlighter.html>.
    Copyright 2010  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    CodeTextHighlighter(QTextDocument *parent = 0);

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
