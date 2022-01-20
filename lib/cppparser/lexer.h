/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LEXER_H
#define LEXER_H

#undef QT_NO_CAST_TO_ASCII
#undef QT_NO_CAST_FROM_ASCII

#include "driver.h"
#include "debug_utils.h"

#include <qglobal.h>
#include <QString>
#include <qmap.h>
#include <qpair.h>
#include <hashedstring.h>

#define CHARTYPE QChar
#define DBG_LEXER DEBUG_N(QLatin1String("Lexer"))

enum Type {
    Token_eof = 0,
    Token_identifier = 1000,
    Token_number_literal,
    Token_char_literal,
    Token_string_literal,
    Token_whitespaces,
    Token_comment,
    Token_preproc,

    Token_assign = 2000,
    Token_ptrmem,
    Token_ellipsis,
    Token_scope,
    Token_shift,
    Token_eq,
    Token_leq,
    Token_geq,
    Token_incr,
    Token_decr,
    Token_arrow,

    Token_concat,

    Token_K_DCOP,
    Token_k_dcop,
    Token_k_dcop_signals,

    Token_Q_OBJECT,
    Token_signals,
    Token_slots,
    Token_emit,
    Token_foreach, // qt4 [erbsland]

    Token_and,
    Token_and_eq,
    Token_asm,
    Token_auto,
    Token_bitand,
    Token_bitor,
    Token_bool,
    Token_break,
    Token_case,
    Token_catch,
    Token_char,
    Token_class,
    Token_compl,
    Token_const,
    Token_const_expr,
    Token_const_cast,
    Token_continue,
    Token_default,
    Token_delete,
    Token_do,
    Token_double,
    Token_dynamic_cast,
    Token_else,
    Token_enum,
    Token_explicit,
    Token_export,
    Token_extern,
    Token_false,
    Token_float,
    Token_for,
    Token_friend,
    Token_goto,
    Token_if,
    Token_inline,
    Token_int,
    Token_long,
    Token_mutable,
    Token_namespace,
    Token_new,
    Token_noexcept,
    Token_not,
    Token_not_eq,
    Token_operator,
    Token_or,
    Token_or_eq,
    Token_override,
    Token_private,
    Token_protected,
    Token_public,
    Token_register,
    Token_reinterpret_cast,
    Token_return,
    Token_short,
    Token_signed,
    Token_sizeof,
    Token_static,
    Token_static_cast,
    Token_struct,
    Token_switch,
    Token_template,
    Token_this,
    Token_throw,
    Token_true,
    Token_try,
    Token_typedef,
    Token_typeid,
    Token_typename,
    Token_union,
    Token_unsigned,
    Token_using,
    Token_virtual,
    Token_void,
    Token_volatile,
    Token_wchar_t,
    Token_while,
    Token_xor,
    Token_xor_eq
};

enum SkipType {
    SkipWord,
    SkipWordAndArguments
};

struct LexerData;

class Token
{
    explicit Token(const QString &);
    Token(int type, int position, int length, const QString& text);
    Token(const Token& source);

    Token& operator = (const Token& source);
    bool operator == (const Token& token) const;
    operator int () const;

public:
    bool isNull() const;

    int type() const;
    void setType(int type);

    void getStartPosition(int* line, int* column) const;
    void setStartPosition(int line, int column);
    void getEndPosition(int* line, int* column) const;
    void setEndPosition(int line, int column);

    unsigned int length() const;
    void setLength(unsigned int length);

    int position() const;
    void setPosition(int position);

    QString text() const;

private:
    int m_type;
    int m_position;
    int m_length;
    int m_startLine;
    int m_startColumn;
    int m_endLine;
    int m_endColumn;
    const QString & m_text;

    friend class Lexer;
    friend class Parser;
}; // class Token

class Lexer
{
public:
    explicit Lexer(Driver* driver);
    ~Lexer();

    bool recordComments() const;
    void setRecordComments(bool record);

    bool recordWhiteSpaces() const;
    void setRecordWhiteSpaces(bool record);

    bool reportWarnings() const;
    void setReportWarnings(bool enable);

    bool reportMessages() const;
    void setReportMessages(bool enable);

    bool skipWordsEnabled() const;
    void setSkipWordsEnabled(bool enabled);

    bool preprocessorEnabled() const;
    void setPreprocessorEnabled(bool enabled);

    void resetSkipWords();
    void addSkipWord(const QString& word, SkipType skipType=SkipWord, const QString& str = QString());

    QString source() const;
    void setSource(const QString& source);

    int index() const;
    void setIndex(int index);

    //returns the count of lines that wer skipped due to #ifdef's
    int skippedLines() const;

    void reset();

    const Token& tokenAt(int position) const;
    const Token& nextToken();
    const Token& lookAhead(int n) const;

    static int toInt(const Token& token);

    int tokenPosition(const Token& token) const;
    void getTokenPosition(const Token& token, int* line, int* col);

    int currentLine() const
    {
        return m_currentLine;
    }
    int currentColumn() const
    {
        return m_currentColumn;
    }

private:
    const QChar currentChar() const;
    QChar peekChar(int n=1) const;
    int currentPosition() const;

    void insertCurrent(const QString& str);

    void tokenize();
    void nextToken(Token& token, bool stopOnNewline=false);
    void nextChar();
    void nextChar(int n);
    void skip(int l, int r);
    void readIdentifier();
    bool readWhiteSpaces(bool skipNewLine=true, bool skipOnlyOnce=false);
    void readLineComment();
    void readMultiLineComment();
    void readCharLiteral();
    void readStringLiteral();
    void readNumberLiteral();

    int findOperator3() const;
    int findOperator2() const;
    bool eof() const;

    // preprocessor (based on an article of Al Stevens on Dr.Dobb's journal)
    int testIfLevel();
    int macroDefined();
    QString readArgument();

    int macroPrimary();
    int macroMultiplyDivide();
    int macroAddSubtract();
    int macroRelational();
    int macroEquality();
    int macroBoolAnd();
    int macroBoolXor();
    int macroBoolOr();
    int macroLogicalAnd();
    int macroLogicalOr();
    int macroExpression();

    void handleDirective(const QString& directive);
    void processDefine(Macro& macro);
    void processElse();
    void processElif();
    void processEndif();
    void processIf();
    void processIfdef();
    void processIfndef();
    void processInclude();
    void processUndef();

private:
    LexerData* d;
    Driver* m_driver;
    QVector<Token*> m_tokens;
    int m_size;
    int m_index;
    QString m_source;
    int m_idx;
    int m_endIdx;
    QChar m_currentChar;
    bool m_recordComments;
    bool m_recordWhiteSpaces;
    bool m_startLine;
    QHash< HashedString, QPair<SkipType, QString> > m_words;


    int m_skippedLines;
    int m_currentLine;
    int m_currentColumn;
    bool m_skipWordsEnabled;

    // preprocessor
    QVector<bool> m_skipping;
    QVector<bool> m_trueTest;
    int m_ifLevel;
    bool m_preprocessorEnabled;
    bool m_inPreproc;

    bool m_reportWarnings;
    bool m_reportMessages;

private:
    Lexer(const Lexer& source);
    void operator = (const Lexer& source);
};

#endif
