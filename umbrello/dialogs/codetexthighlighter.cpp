/*
    CodeTextHighlighter: Syntax highlighter for the CodeTextEdit widget.
    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>
    Code based on examples of the Qt Toolkit under BSD license,
    <http://doc.qt.nokia.com/4.6/richtext-syntaxhighlighter.html>.
    SPDX-FileCopyrightText: 2010-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "codetexthighlighter.h"

#include "codegenerator.h"
#include "codegenfactory.h"
#include "uml.h"

/**
 * Constructor.
 * Creates the highlighting rule by calling the function keywords().
 * @param parent   the parent QTextDocument
 */
CodeTextHighlighter::CodeTextHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    m_keywordFormat.setForeground(Qt::darkBlue);
    m_keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywordPatterns = keywords();
    Q_FOREACH (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = m_keywordFormat;
        m_highlightingRules.append(rule);
    }

    m_classFormat.setFontWeight(QFont::Bold);
    m_classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression(QString::fromLatin1("\\bQ[A-Za-z]+\\b"));
    rule.format = m_classFormat;
    m_highlightingRules.append(rule);

    m_singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression(QString::fromLatin1("//[^\n]*"));
    rule.format = m_singleLineCommentFormat;
    m_highlightingRules.append(rule);

    m_multiLineCommentFormat.setForeground(Qt::red);

    m_quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QString::fromLatin1("\".*\""));
    rule.format = m_quotationFormat;
    m_highlightingRules.append(rule);

    m_functionFormat.setFontItalic(true);
    m_functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression(QString::fromLatin1("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = m_functionFormat;
    m_highlightingRules.append(rule);

    m_commentStartExpression = QRegularExpression(QString::fromLatin1("/\\*"));
    m_commentEndExpression = QRegularExpression(QString::fromLatin1("\\*/"));
}

/**
 * Does highlighting the code block.
 * @param text   the code block to highlight
 */
void CodeTextHighlighter::highlightBlock(const QString &text)
{
    for(const HighlightingRule &rule : m_highlightingRules) {
        QRegularExpression expression(rule.pattern);
        const QRegularExpressionMatch match = expression.match(text);
        if (!match.hasMatch()) {
            continue;
        }

        for (int i = 0; i <= match.lastCapturedIndex(); i++) {
            setFormat(match.capturedStart(i), match.capturedLength(i), rule.format);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = m_commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = m_commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + m_commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, m_multiLineCommentFormat);
        startIndex = m_commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}

/**
 * Create a list of keywords for the selected programming language.
 * @return list of keywords
 */
QStringList CodeTextHighlighter::keywords()
{
    Uml::ProgrammingLanguage::Enum pl = UMLApp::app()->activeLanguage();
    QStringList keywordList;
    if (pl == Uml::ProgrammingLanguage::Reserved)
        return keywordList;  // empty
    CodeGenerator* generator = CodeGenFactory::createObject(pl);
    keywordList = generator->reservedKeywords();
    delete generator;

//     if (keywordList.size() <= 0) {
//         keywordList << "\\bchar\\b" << "\\bclass\\b" << "\\bconst\\b"
//                     << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
//                     << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
//                     << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
//                     << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
//                     << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
//                     << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
//                     << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
//                     << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
//                     << "\\bvoid\\b" << "\\bvolatile\\b";
//     }

    return keywordList;
}
