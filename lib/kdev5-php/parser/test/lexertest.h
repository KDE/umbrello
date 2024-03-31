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

private:
    Q_SLOT void testOpenTagWithNewline();
    Q_SLOT void testOpenTagWithSpace();
    Q_SLOT void testCommentOneLine();
    Q_SLOT void testCommentOneLine2();
    Q_SLOT void testCommentMultiLine();
    Q_SLOT void testCommentMultiLine2();
    Q_SLOT void testEndTag();
    Q_SLOT void testNewlineInString();
    Q_SLOT void testNewlineInString2();
    Q_SLOT void testNewlineInStringWithVar();
    Q_SLOT void testNewlineInStringWithVar2();
    Q_SLOT void testNewlineInStringWithVar3();
    Q_SLOT void testMultiplePhpSections();
    Q_SLOT void testHereDoc();
    Q_SLOT void testHereDocQuoted();
    Q_SLOT void testNowdoc();
    Q_SLOT void testCommonStringTokens();
    Q_SLOT void testNonTerminatedStringWithVar();
    Q_SLOT void testPhpBlockWithComment();
    Q_SLOT void testNamespaces();
    Q_SLOT void testCloseTagInComment();
    Q_SLOT void testBinaryNumber();
    Q_SLOT void testHexadecimalNumber();
    Q_SLOT void testTypeHintsOnFunction();
    Q_SLOT void testExponentiation();
    Q_SLOT void testExceptionFinally();

protected:
    TokenStream* tokenize(const QString& unit, bool debug = false, int initialState = Lexer::HtmlState);
};

}

#endif // LEXERTEST_H
