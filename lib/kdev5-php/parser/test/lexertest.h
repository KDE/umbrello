/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef LEXERTEST_H
#define LEXERTEST_H

#include <QObject>
#include <QByteArray>
#include <QTest>

#include "phplexer.h"

namespace KDevelop
{
class TopDUContext;
}

namespace Php
{
class TokenStream;
class LexerTest : public QObject
{
    Q_OBJECT

public:
    LexerTest();

private slots:
    void testOpenTagWithNewline();
    void testOpenTagWithSpace();
    void testCommentOneLine();
    void testCommentOneLine2();
    void testCommentMultiLine();
    void testCommentMultiLine2();
    void testEndTag();
    void testNewlineInString();
    void testNewlineInString2();
    void testNewlineInStringWithVar();
    void testNewlineInStringWithVar2();
    void testNewlineInStringWithVar3();
    void testMultiplePhpSections();
    void testHereDoc();
    void testHereDocQuoted();
    void testNowdoc();
    void testCommonStringTokens();
    void testNonTerminatedStringWithVar();
    void testPhpBlockWithComment();
    void testNamespaces();
    void testCloseTagInComment();
    void testBinaryNumber();
    void testHexadecimalNumber();
    void testTypeHintsOnFunction();
    void testExponentiation();
    void testExceptionFinally();

protected:
    TokenStream* tokenize(const QString& unit, bool debug = false, int initialState = Lexer::HtmlState);
};

}

#endif // LEXERTEST_H
