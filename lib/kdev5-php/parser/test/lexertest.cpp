/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "lexertest.h"

#include <QtTest/QtTest>

#include "parsesession.h"
#include "phplexer.h"
#include "phptokentext.h"

QTEST_MAIN(Php::LexerTest)
namespace Php
{

#define COMPARE_TOKEN(tokenStream, index, tokenKind, startLine, startColumn, endLine, endColumn) \
    { \
        QVERIFY(tokenStream->at(index).kind == tokenKind); \
        qint64 line; qint64 column; \
        tokenStream->startPosition(index, &line, &column); \
        QCOMPARE(line, (qint64) startLine); \
        QCOMPARE(column, (qint64) startColumn); \
        tokenStream->endPosition(index, &line, &column); \
        QCOMPARE(line, (qint64) endLine); \
        QCOMPARE(column, (qint64) endColumn); \
    }

LexerTest::LexerTest()
{
}

void LexerTest::testOpenTagWithNewline()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\nfoo;"));
    QVERIFY(ts->size() == 3);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_STRING, 1, 0, 1, 2);
    COMPARE_TOKEN(ts, 2, Parser::Token_SEMICOLON, 1, 3, 1, 3);

    delete ts;
}

void LexerTest::testOpenTagWithSpace()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php foo;"));
    QVERIFY(ts->size() == 3);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_STRING, 0, 6, 0, 8);
    COMPARE_TOKEN(ts, 2, Parser::Token_SEMICOLON, 0, 9, 0, 9);
    delete ts;
}

void LexerTest::testCommentOneLine()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\n//comment\nfoo;"));
    QVERIFY(ts->size() == 4);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_COMMENT, 1, 0, 1, 9);
    COMPARE_TOKEN(ts, 2, Parser::Token_STRING, 2, 0, 2, 2);
    COMPARE_TOKEN(ts, 3, Parser::Token_SEMICOLON, 2, 3, 2, 3);
    delete ts;
}

void LexerTest::testCommentOneLine2()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\n#comment\nfoo;"));
    QVERIFY(ts->size() == 4);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_COMMENT, 1, 0, 1, 8);
    COMPARE_TOKEN(ts, 2, Parser::Token_STRING, 2, 0, 2, 2);
    COMPARE_TOKEN(ts, 3, Parser::Token_SEMICOLON, 2, 3, 2, 3);
    delete ts;
}

void LexerTest::testCommentMultiLine()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\n/*com\nment*/\nfoo;"), true);
    QVERIFY(ts->size() == 5);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_COMMENT, 1, 0, 2, 5);
    COMPARE_TOKEN(ts, 2, Parser::Token_WHITESPACE, 2, 6, 2, 6);
    COMPARE_TOKEN(ts, 3, Parser::Token_STRING, 3, 0, 3, 2);
    COMPARE_TOKEN(ts, 4, Parser::Token_SEMICOLON, 3, 3, 3, 3);
    delete ts;
}

void LexerTest::testCommentMultiLine2()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\n/*\nment*/\nfoo;"), true);
    QVERIFY(ts->size() == 5);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_COMMENT, 1, 0, 2, 5);
    COMPARE_TOKEN(ts, 2, Parser::Token_WHITESPACE, 2, 6, 2, 6);
    COMPARE_TOKEN(ts, 3, Parser::Token_STRING, 3, 0, 3, 2);
    COMPARE_TOKEN(ts, 4, Parser::Token_SEMICOLON, 3, 3, 3, 3);
    delete ts;
}

void LexerTest::testEndTag()
{
    TokenStream* ts = tokenize(QStringLiteral("<?\n':\n'?>\n>"), true, Lexer::DefaultState);
    //don't crash and we are fine
    delete ts;
}

void LexerTest::testNewlineInString()
{
    //0            1
    //012345 6 7 890123456789
    TokenStream* ts = tokenize(QStringLiteral("<?php \"\n\";"), true);
    QVERIFY(ts->size() == 3);

    COMPARE_TOKEN(ts, 1, Parser::Token_CONSTANT_ENCAPSED_STRING, 0, 6, 1, 0);
    COMPARE_TOKEN(ts, 2, Parser::Token_SEMICOLON, 1, 1, 1, 1);
    delete ts;
}

void LexerTest::testNewlineInString2()
{
    //0
    //0123 4567
    TokenStream* ts = tokenize(QStringLiteral("<?php '\n';"), true);
    QCOMPARE((int)ts->size(), 3);

    COMPARE_TOKEN(ts, 1, Parser::Token_CONSTANT_ENCAPSED_STRING, 0, 6, 1, 0);
    COMPARE_TOKEN(ts, 2, Parser::Token_SEMICOLON, 1, 1, 1, 1);
    delete ts;
}

void LexerTest::testNewlineInStringWithVar()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php \"$a\n\";"), true);
    QCOMPARE((int)ts->size(), 6);

    COMPARE_TOKEN(ts, 1, Parser::Token_DOUBLE_QUOTE, 0, 6, 0, 6);
    COMPARE_TOKEN(ts, 2, Parser::Token_VARIABLE, 0, 7, 0, 8);
    COMPARE_TOKEN(ts, 3, Parser::Token_ENCAPSED_AND_WHITESPACE, 0, 9, 0, 9);
    COMPARE_TOKEN(ts, 4, Parser::Token_DOUBLE_QUOTE, 1, 0, 1, 0);
    COMPARE_TOKEN(ts, 5, Parser::Token_SEMICOLON, 1, 1, 1, 1);
    delete ts;
}

void LexerTest::testNewlineInStringWithVar2()
{
    //0            1
    //012345 6 789 0123456789
    TokenStream* ts = tokenize(QStringLiteral("<?php \"\n$a\n\";"), true);
    QCOMPARE((int)ts->size(), 7);

    COMPARE_TOKEN(ts, 1, Parser::Token_DOUBLE_QUOTE, 0, 6, 0, 6);
    COMPARE_TOKEN(ts, 2, Parser::Token_ENCAPSED_AND_WHITESPACE, 0, 7, 0, 7);
    COMPARE_TOKEN(ts, 3, Parser::Token_VARIABLE, 1, 0, 1, 1);
    COMPARE_TOKEN(ts, 4, Parser::Token_ENCAPSED_AND_WHITESPACE, 1, 2, 1, 2);
    COMPARE_TOKEN(ts, 5, Parser::Token_DOUBLE_QUOTE, 2, 0, 2, 0);
    COMPARE_TOKEN(ts, 6, Parser::Token_SEMICOLON, 2, 1, 2, 1);
    delete ts;
}

void LexerTest::testNewlineInStringWithVar3()
{
    //0            1
    //012345 6 789 0123456789
    TokenStream* ts = tokenize(QStringLiteral("<?php \"{$$a}\";"), true);
    QCOMPARE((int)ts->size(), 7);

    COMPARE_TOKEN(ts, 1, Parser::Token_DOUBLE_QUOTE, 0, 6, 0, 6);
    COMPARE_TOKEN(ts, 2, Parser::Token_ENCAPSED_AND_WHITESPACE, 0, 7, 0, 8);
    COMPARE_TOKEN(ts, 3, Parser::Token_VARIABLE, 0, 9, 0, 10);
    COMPARE_TOKEN(ts, 4, Parser::Token_ENCAPSED_AND_WHITESPACE, 0, 11, 0, 11);
    COMPARE_TOKEN(ts, 5, Parser::Token_DOUBLE_QUOTE, 0, 12, 0, 12);
    COMPARE_TOKEN(ts, 6, Parser::Token_SEMICOLON, 0, 13, 0, 13);
    delete ts;
}

void LexerTest::testMultiplePhpSections()
{

    //0            1
    //012345 6 789 0123456789
    TokenStream* ts = tokenize(QStringLiteral("<?php $a;?>\n<html>\n<?php $a;?>"), true);
    QCOMPARE((int)ts->size(), 9);

    qint64 index = 0;
    for (qint64 line = 0; line <= 2; ++line) {
        if (line == 1) {
            // the html stuff in the middle
            COMPARE_TOKEN(ts, index, Parser::Token_INLINE_HTML, 0, 11, 1, 6);
            ++index;
        } else {
            // the php stuff (symmetric) at the start and end
            COMPARE_TOKEN(ts, index, Parser::Token_OPEN_TAG, line, 0, line, 5);
            ++index;

            COMPARE_TOKEN(ts, index, Parser::Token_VARIABLE, line, 6, line, 7);
            ++index;

            COMPARE_TOKEN(ts, index, Parser::Token_SEMICOLON, line, 8, line, 8);
            ++index;

            COMPARE_TOKEN(ts, index, Parser::Token_CLOSE_TAG, line, 9, line, 10);
            ++index;
        }
    }
    delete ts;
}

void LexerTest::testHereDoc()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\necho <<<EOD1\nstart $text\nend\nEOD1;\n$extern;"), true);
    QCOMPARE((int)ts->size(), 12);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_ECHO, 1, 0, 1, 3);
    COMPARE_TOKEN(ts, 3, Parser::Token_START_HEREDOC, 1, 5, 1, 12);
    COMPARE_TOKEN(ts, 4, Parser::Token_ENCAPSED_AND_WHITESPACE, 2, 0, 2, 5);
    COMPARE_TOKEN(ts, 5, Parser::Token_VARIABLE, 2, 6, 2, 10);
    COMPARE_TOKEN(ts, 6, Parser::Token_ENCAPSED_AND_WHITESPACE, 2, 11, 3, 3);
    COMPARE_TOKEN(ts, 7, Parser::Token_END_HEREDOC, 4, 0, 4, 3);
    COMPARE_TOKEN(ts, 8, Parser::Token_SEMICOLON, 4, 4, 4, 4);
    COMPARE_TOKEN(ts, 10, Parser::Token_VARIABLE, 5, 0, 5, 6);
    COMPARE_TOKEN(ts, 11, Parser::Token_SEMICOLON, 5, 7, 5, 7);
    delete ts;
}

void LexerTest::testHereDocQuoted()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\necho <<<\"EOD1\"\nstart $text\nend\nEOD1;\n$extern;"), true);
    QCOMPARE((int)ts->size(), 12);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_ECHO, 1, 0, 1, 3);
    COMPARE_TOKEN(ts, 3, Parser::Token_START_HEREDOC, 1, 5, 1, 14);
    COMPARE_TOKEN(ts, 4, Parser::Token_ENCAPSED_AND_WHITESPACE, 2, 0, 2, 5);
    COMPARE_TOKEN(ts, 5, Parser::Token_VARIABLE, 2, 6, 2, 10);
    COMPARE_TOKEN(ts, 6, Parser::Token_ENCAPSED_AND_WHITESPACE, 2, 11, 3, 3);
    COMPARE_TOKEN(ts, 7, Parser::Token_END_HEREDOC, 4, 0, 4, 3);
    COMPARE_TOKEN(ts, 8, Parser::Token_SEMICOLON, 4, 4, 4, 4);
    COMPARE_TOKEN(ts, 10, Parser::Token_VARIABLE, 5, 0, 5, 6);
    COMPARE_TOKEN(ts, 11, Parser::Token_SEMICOLON, 5, 7, 5, 7);
    delete ts;
}

void LexerTest::testNowdoc()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\necho <<<'EOD1'\nstart $text\nend\nEOD1;\n$extern;"), true);
    QCOMPARE((int)ts->size(), 10);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_ECHO, 1, 0, 1, 3);
    COMPARE_TOKEN(ts, 3, Parser::Token_START_NOWDOC, 1, 5, 1, 14);
    COMPARE_TOKEN(ts, 4, Parser::Token_STRING, 2, 0, 3, 3);
    COMPARE_TOKEN(ts, 5, Parser::Token_END_NOWDOC, 4, 0, 4, 3);
    COMPARE_TOKEN(ts, 6, Parser::Token_SEMICOLON, 4, 4, 4, 4);
    COMPARE_TOKEN(ts, 8, Parser::Token_VARIABLE, 5, 0, 5, 6);
    COMPARE_TOKEN(ts, 9, Parser::Token_SEMICOLON, 5, 7, 5, 7);
    delete ts;
}

void LexerTest::testCommonStringTokens()
{
    // all these should have open_tag followed by constant encapsed string
    foreach ( const QString& code, QStringList() << "<?php ''" << "<?php \"\"" << "<?php '" << "<?php \"" ) {
        qDebug() << code;
        TokenStream* ts = tokenize(code, true);

        QCOMPARE((int)ts->size(), 2);

        COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
        COMPARE_TOKEN(ts, 1, Parser::Token_CONSTANT_ENCAPSED_STRING, 0, 6, 0, code.size() - 1);

        delete ts;
    }
}

void LexerTest::testNonTerminatedStringWithVar()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php \"$a"), true);

    QCOMPARE((int)ts->size(), 3);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_DOUBLE_QUOTE, 0, 6, 0, 6);
    COMPARE_TOKEN(ts, 2, Parser::Token_VARIABLE, 0, 7, 0, 8);
    delete ts;
}

void LexerTest::testPhpBlockWithComment()
{
    TokenStream* ts = tokenize(
        QStringLiteral("<?php\n"
        "//asdf\n"
        "?>\n"
        "<?php\n")
    , true);

    QCOMPARE((int)ts->size(), 5);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_COMMENT, 1, 0, 1, 6);
    COMPARE_TOKEN(ts, 2, Parser::Token_CLOSE_TAG, 2, 0, 2, 1);
    COMPARE_TOKEN(ts, 3, Parser::Token_INLINE_HTML, 2, 2, 2, 2);
    COMPARE_TOKEN(ts, 4, Parser::Token_OPEN_TAG, 3, 0, 3, 5);
    delete ts;
}

void LexerTest::testNamespaces()
{
    TokenStream* ts = tokenize(
        QStringLiteral("<?php\n"
        "namespace Foo;\n"
        "namespace Foo\\Bar;\n"
        "namespace Foo\\Bar\\Asd {\n"
        "}\n")
    , true);
    QCOMPARE((int)ts->size(), 25);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);

    COMPARE_TOKEN(ts, 1, Parser::Token_NAMESPACE, 1, 0, 1, 8);
    COMPARE_TOKEN(ts, 2, Parser::Token_WHITESPACE, 1, 9, 1, 9);
    COMPARE_TOKEN(ts, 3, Parser::Token_STRING, 1, 10, 1, 12);
    COMPARE_TOKEN(ts, 4, Parser::Token_SEMICOLON, 1, 13, 1, 13);

    COMPARE_TOKEN(ts, 6, Parser::Token_NAMESPACE, 2, 0, 2, 8);
    COMPARE_TOKEN(ts, 7, Parser::Token_WHITESPACE, 2, 9, 2, 9);
    COMPARE_TOKEN(ts, 8, Parser::Token_STRING, 2, 10, 2, 12);
    COMPARE_TOKEN(ts, 9, Parser::Token_BACKSLASH, 2, 13, 2, 13);
    COMPARE_TOKEN(ts, 10, Parser::Token_STRING, 2, 14, 2, 16);
    COMPARE_TOKEN(ts, 11, Parser::Token_SEMICOLON, 2, 17, 2, 17);

    COMPARE_TOKEN(ts, 13, Parser::Token_NAMESPACE, 3, 0, 3, 8);
    COMPARE_TOKEN(ts, 14, Parser::Token_WHITESPACE, 3, 9, 3, 9);
    COMPARE_TOKEN(ts, 15, Parser::Token_STRING, 3, 10, 3, 12);
    COMPARE_TOKEN(ts, 16, Parser::Token_BACKSLASH, 3, 13, 3, 13);
    COMPARE_TOKEN(ts, 17, Parser::Token_STRING, 3, 14, 3, 16);
    COMPARE_TOKEN(ts, 18, Parser::Token_BACKSLASH, 3, 17, 3, 17);
    COMPARE_TOKEN(ts, 19, Parser::Token_STRING, 3, 18, 3, 20);
    COMPARE_TOKEN(ts, 20, Parser::Token_WHITESPACE, 3, 21, 3, 21);
    COMPARE_TOKEN(ts, 21, Parser::Token_LBRACE, 3, 22, 3, 22);
    COMPARE_TOKEN(ts, 23, Parser::Token_RBRACE, 4, 0, 4, 0);

    delete ts;
}

void LexerTest::testCloseTagInComment()
{
    {
    TokenStream* ts = tokenize(
        QStringLiteral("<?php // asdf ?>")
    , true);
    QCOMPARE((int)ts->size(), 3);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_COMMENT, 0, 6, 0, 13);
    COMPARE_TOKEN(ts, 2, Parser::Token_CLOSE_TAG, 0, 14, 0, 15);

    delete ts;
    }
    {
    TokenStream* ts = tokenize(
        QStringLiteral("<?php #  asdf ?>")
    , true);
    QCOMPARE((int)ts->size(), 3);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_COMMENT, 0, 6, 0, 13);
    COMPARE_TOKEN(ts, 2, Parser::Token_CLOSE_TAG, 0, 14, 0, 15);

    delete ts;
    }
}

void LexerTest::testBinaryNumber()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\n0b01;\n0B01;"), true);
    QCOMPARE((int)ts->size(), 6);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_LNUMBER, 1, 0, 1, 3);
    COMPARE_TOKEN(ts, 2, Parser::Token_SEMICOLON, 1, 4, 1, 4);
    COMPARE_TOKEN(ts, 3, Parser::Token_WHITESPACE, 1, 5, 1, 5);
    COMPARE_TOKEN(ts, 4, Parser::Token_LNUMBER, 2, 0, 2, 3);
    COMPARE_TOKEN(ts, 5, Parser::Token_SEMICOLON, 2, 4, 2, 4);
    delete ts;
}

void LexerTest::testHexadecimalNumber()
{
    TokenStream* ts = tokenize(QStringLiteral("<?php\n0x01;\n0X01;\n0xABC12;\n0Xab10A;"), true);
    QCOMPARE((int)ts->size(), 12);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_LNUMBER, 1, 0, 1, 3);
    COMPARE_TOKEN(ts, 2, Parser::Token_SEMICOLON, 1, 4, 1, 4);
    COMPARE_TOKEN(ts, 3, Parser::Token_WHITESPACE, 1, 5, 1, 5);
    COMPARE_TOKEN(ts, 4, Parser::Token_LNUMBER, 2, 0, 2, 3);
    COMPARE_TOKEN(ts, 5, Parser::Token_SEMICOLON, 2, 4, 2, 4);
    COMPARE_TOKEN(ts, 6, Parser::Token_WHITESPACE, 2, 5, 2, 5);
    COMPARE_TOKEN(ts, 7, Parser::Token_LNUMBER, 3, 0, 3, 6);
    COMPARE_TOKEN(ts, 8, Parser::Token_SEMICOLON, 3, 7, 3, 7);
    COMPARE_TOKEN(ts, 9, Parser::Token_WHITESPACE, 3, 8, 3, 8);
    COMPARE_TOKEN(ts, 10, Parser::Token_LNUMBER, 4, 0, 4, 6);
    COMPARE_TOKEN(ts, 11, Parser::Token_SEMICOLON, 4, 7, 4, 7);
    delete ts;
}

void LexerTest::testTypeHintsOnFunction()
{
    QScopedPointer<TokenStream> ts(tokenize(QStringLiteral("<?php\nfunction a($a, array $b = [], callable $c) {}"), true));
    QCOMPARE((int)ts->size(), 25);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_FUNCTION, 1, 0, 1, 7);
    COMPARE_TOKEN(ts, 2, Parser::Token_WHITESPACE, 1, 8, 1, 8);
    COMPARE_TOKEN(ts, 3, Parser::Token_STRING, 1, 9, 1, 9);
    COMPARE_TOKEN(ts, 4, Parser::Token_LPAREN, 1, 10, 1, 10);
    COMPARE_TOKEN(ts, 5, Parser::Token_VARIABLE,  1, 11, 1, 12);
    COMPARE_TOKEN(ts, 6, Parser::Token_COMMA, 1, 13, 1, 13);
    COMPARE_TOKEN(ts, 7, Parser::Token_WHITESPACE, 1, 14, 1, 14);
    COMPARE_TOKEN(ts, 8, Parser::Token_ARRAY, 1, 15, 1, 19);
    COMPARE_TOKEN(ts, 9, Parser::Token_WHITESPACE, 1, 20, 1, 20);
    COMPARE_TOKEN(ts, 10, Parser::Token_VARIABLE,  1, 21, 1, 22);
    COMPARE_TOKEN(ts, 11, Parser::Token_WHITESPACE, 1, 23, 1, 23);
    COMPARE_TOKEN(ts, 12, Parser::Token_ASSIGN, 1, 24, 1, 24);
    COMPARE_TOKEN(ts, 13, Parser::Token_WHITESPACE, 1, 25, 1, 25);
    COMPARE_TOKEN(ts, 14, Parser::Token_LBRACKET, 1, 26, 1, 26);
    COMPARE_TOKEN(ts, 15, Parser::Token_RBRACKET, 1, 27, 1, 27);
    COMPARE_TOKEN(ts, 16, Parser::Token_COMMA, 1, 28, 1, 28);
    COMPARE_TOKEN(ts, 17, Parser::Token_WHITESPACE, 1, 29, 1, 29);
    COMPARE_TOKEN(ts, 18, Parser::Token_CALLABLE, 1, 30, 1, 37);
    COMPARE_TOKEN(ts, 19, Parser::Token_WHITESPACE, 1, 38, 1, 38);
    COMPARE_TOKEN(ts, 20, Parser::Token_VARIABLE,  1, 39, 1, 40);
    COMPARE_TOKEN(ts, 21, Parser::Token_RPAREN, 1, 41, 1, 41);
    COMPARE_TOKEN(ts, 22, Parser::Token_WHITESPACE, 1, 42, 1, 42);
    COMPARE_TOKEN(ts, 23, Parser::Token_LBRACE, 1, 43, 1, 43);
    COMPARE_TOKEN(ts, 24, Parser::Token_RBRACE, 1, 44, 1, 44);
}

void LexerTest::testExponentiation()
{
    QScopedPointer<TokenStream> ts(tokenize(QStringLiteral("<?php\n$a = 2 ** 3; $a **= 2;"), true));
    QCOMPARE((int)ts->size(), 18);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_VARIABLE, 1, 0, 1, 1);
    COMPARE_TOKEN(ts, 2, Parser::Token_WHITESPACE, 1, 2, 1, 2);
    COMPARE_TOKEN(ts, 3, Parser::Token_ASSIGN, 1, 3, 1, 3);
    COMPARE_TOKEN(ts, 4, Parser::Token_WHITESPACE, 1, 4, 1, 4);
    COMPARE_TOKEN(ts, 5, Parser::Token_LNUMBER, 1, 5, 1, 5);
    COMPARE_TOKEN(ts, 6, Parser::Token_WHITESPACE, 1, 6, 1, 6);
    COMPARE_TOKEN(ts, 7, Parser::Token_EXP, 1, 7, 1, 8);
    COMPARE_TOKEN(ts, 8, Parser::Token_WHITESPACE, 1, 9, 1, 9);
    COMPARE_TOKEN(ts, 9, Parser::Token_LNUMBER, 1, 10, 1, 10);
    COMPARE_TOKEN(ts, 10, Parser::Token_SEMICOLON, 1, 11, 1, 11);
    COMPARE_TOKEN(ts, 11, Parser::Token_WHITESPACE, 1, 12, 1, 12);
    COMPARE_TOKEN(ts, 12, Parser::Token_VARIABLE, 1, 13, 1, 14);
    COMPARE_TOKEN(ts, 13, Parser::Token_WHITESPACE, 1, 15, 1, 15);
    COMPARE_TOKEN(ts, 14, Parser::Token_EXP_ASSIGN, 1, 16, 1, 18);
    COMPARE_TOKEN(ts, 15, Parser::Token_WHITESPACE, 1, 19, 1, 19);
    COMPARE_TOKEN(ts, 16, Parser::Token_LNUMBER, 1, 20, 1, 20);
    COMPARE_TOKEN(ts, 17, Parser::Token_SEMICOLON, 1, 21, 1, 21);

}

void LexerTest::testExceptionFinally()
{
    QScopedPointer<TokenStream> ts(tokenize(QStringLiteral("<?php\ntry { $a = 1; } finally { }"), true));
    QCOMPARE((int)ts->size(), 19);

    COMPARE_TOKEN(ts, 0, Parser::Token_OPEN_TAG, 0, 0, 0, 5);
    COMPARE_TOKEN(ts, 1, Parser::Token_TRY, 1, 0, 1, 2);
    COMPARE_TOKEN(ts, 2, Parser::Token_WHITESPACE, 1, 3, 1, 3);
    COMPARE_TOKEN(ts, 3, Parser::Token_LBRACE, 1, 4, 1, 4);
    COMPARE_TOKEN(ts, 4, Parser::Token_WHITESPACE, 1, 5, 1, 5);
    COMPARE_TOKEN(ts, 5, Parser::Token_VARIABLE, 1, 6, 1, 7);
    COMPARE_TOKEN(ts, 6, Parser::Token_WHITESPACE, 1, 8, 1, 8);
    COMPARE_TOKEN(ts, 7, Parser::Token_ASSIGN, 1, 9, 1, 9);
    COMPARE_TOKEN(ts, 8, Parser::Token_WHITESPACE, 1, 10, 1, 10);
    COMPARE_TOKEN(ts, 9, Parser::Token_LNUMBER, 1, 11, 1, 11);
    COMPARE_TOKEN(ts, 10, Parser::Token_SEMICOLON, 1, 12, 1, 12);
    COMPARE_TOKEN(ts, 11, Parser::Token_WHITESPACE, 1, 13, 1, 13);
    COMPARE_TOKEN(ts, 12, Parser::Token_RBRACE, 1, 14, 1, 14);
    COMPARE_TOKEN(ts, 13, Parser::Token_WHITESPACE, 1, 15, 1, 15);
    COMPARE_TOKEN(ts, 14, Parser::Token_FINALLY, 1, 16, 1, 22);
    COMPARE_TOKEN(ts, 15, Parser::Token_WHITESPACE, 1, 23, 1, 23);
    COMPARE_TOKEN(ts, 16, Parser::Token_LBRACE, 1, 24, 1, 24);
    COMPARE_TOKEN(ts, 17, Parser::Token_WHITESPACE, 1, 25, 1, 25);
    COMPARE_TOKEN(ts, 18, Parser::Token_RBRACE, 1, 26, 1, 26);
}

TokenStream* LexerTest::tokenize(const QString& unit, bool debug, int initialState)
{
    TokenStream* tokenStream = new TokenStream;
    Lexer lexer(tokenStream, unit, initialState);
    int token;
    int i = 0;
    QList<Parser::Token> tokens;
    while ((token = lexer.nextTokenKind())) {
        Parser::Token &t = tokenStream->push();
        t.begin = lexer.tokenBegin();
        t.end = lexer.tokenEnd();
        t.kind = token;
        tokens << t;
    }
    if (debug) {
        Q_FOREACH(const Parser::Token &t, tokens) {
            qint64 beginLine;
            qint64 beginColumn;
            tokenStream->startPosition(i, &beginLine, &beginColumn);
            qint64 endLine;
            qint64 endColumn;
            tokenStream->endPosition(i, &endLine, &endColumn);
            qDebug() << tokenText(t.kind)
            << unit.mid(t.begin, t.end - t.begin + 1).replace('\n', QLatin1String("\\n"))
            << QStringLiteral("[%0-%1] - [%2-%3]").arg(beginLine).arg(beginColumn).arg(endLine).arg(endColumn);
            ++i;
        }
    }
    return tokenStream;
}
}

