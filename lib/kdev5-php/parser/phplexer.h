/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHPLEXER_H
#define PHPLEXER_H

#include <QStack>
#include <QString>

#include "parserexport.h"

class QString;

namespace KDevPG
{
class TokenStream;
}

namespace Php
{
class TokenStream;

/**
 * Hand-written Lexer that generates the same tokens as php uses.
 * This includes also a whitespace and comment token.
 *
 * For debugging output can be compared to php-tokens using the
 * test/test-tokenize.php script
 **/
class KDEVPHPPARSER_EXPORT Lexer
{
public:
    Lexer(TokenStream *tokenStream, const QString& contents, int initialState = HtmlState);

    int nextTokenKind();
    qint64 tokenBegin() const;
    qint64 tokenEnd() const;

private:
    QString m_content;
    TokenStream* m_tokenStream;
    int m_curpos;
    int m_contentSize;
    qint64 m_tokenBegin;
    qint64 m_tokenEnd;

    int state(int deepness = 0) const;
    void pushState(int state);
    void popState();
    void printState();

    bool processVariable(const QChar* it);
    bool isValidVariableIdentifier(const QChar* it);
    void createNewline(int pos);
    bool isEscapedWithBackslash(const QChar* it, int curPos, int startPos);
    bool isHereNowDocEnd(const QChar* it);

    QStack<int> m_state;

    QString m_hereNowDocIdentifier;
    int m_haltCompiler;

public:
    enum State {
        ErrorState = -1,
        HtmlState = 0,
        DefaultState = 1,
        String = 2,
        StringVariable = 3,
        StringVariableBracket = 4,
        StringVariableObjectOperator = 5,
        StringVariableCurly = 6,
        StringVarname = 7,
        StringHeredoc = 8,
        StringBacktick = 9,
        StringNowdoc = 10
    };
};

}

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
