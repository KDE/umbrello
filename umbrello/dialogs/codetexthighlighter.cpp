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
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = m_keywordFormat;
        m_highlightingRules.append(rule);
    }

    m_classFormat.setFontWeight(QFont::Bold);
    m_classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = m_classFormat;
    m_highlightingRules.append(rule);

    m_singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = m_singleLineCommentFormat;
    m_highlightingRules.append(rule);

    m_multiLineCommentFormat.setForeground(Qt::red);

    m_quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = m_quotationFormat;
    m_highlightingRules.append(rule);

    m_functionFormat.setFontItalic(true);
    m_functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = m_functionFormat;
    m_highlightingRules.append(rule);

    m_commentStartExpression = QRegExp("/\\*");
    m_commentEndExpression = QRegExp("\\*/");
}

/**
 * Does highlighting the code block.
 * @param text   the code block to highlight
 */
void CodeTextHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, m_highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
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
    Uml::ProgrammingLanguage pl = UMLApp::app()->activeLanguage();
    CodeGenerator* generator = CodeGenFactory::createObject(pl);
    QStringList keywordList = generator->reservedKeywords();

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
