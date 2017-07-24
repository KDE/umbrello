/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2008 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

protected:
    TokenStream* tokenize(const QString& unit, bool debug = false, int initialState = Lexer::HtmlState);
};

}

#endif // LEXERTEST_H
