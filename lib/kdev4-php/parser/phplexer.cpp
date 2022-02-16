/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "phplexer.h"

#include "phpparser.h"
#include "tokenstream.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>

#include <iostream>

namespace Php
{

Lexer::Lexer(TokenStream* tokenStream, const QString& content, int initialState):
        m_content(content), m_tokenStream(tokenStream),
        m_curpos(0), m_contentSize(m_content.size()),
        m_tokenBegin(0), m_tokenEnd(0), m_haltCompiler(0)
{
    pushState(ErrorState);
    if (initialState == DefaultState) {
        pushState(HtmlState);
    }
    pushState(initialState);
}

int Lexer::state(int deepness) const
{
    return m_state.at(m_state.size() - deepness - 1);
}
void Lexer::printState()
{
    int s = state();
    if (s == ErrorState)
        qDebug() << "ErrorState";
    else if (s == HtmlState)
        qDebug() << "HtmlState";
    else if (s == DefaultState)
        qDebug() << "DefaultState";
    else if (s == String)
        qDebug() << "String";
    else if (s == StringVariable)
        qDebug() << "StringVariable";
    else if (s == StringVariableBracket)
        qDebug() << "StringVariableBracket";
    else if (s == StringVariableObjectOperator)
        qDebug() << "StringVariableObjectOperator";
    else if (s == StringVariableCurly)
        qDebug() << "StringVariableCurly";
    else if (s == StringVarname)
        qDebug() << "StringVarname";
    else if (s == StringHeredoc)
        qDebug() << "StringHeredoc";
    else if (s == StringBacktick)
        qDebug() << "StringBacktick";
}

void Lexer::pushState(int state)
{
    m_state.push(state);
}

void Lexer::popState()
{
    m_state.pop();
}

int Lexer::nextTokenKind()
{
    int token = Parser::Token_INVALID;
    if (m_curpos >= m_contentSize) {
        m_tokenBegin = -1;
        m_tokenEnd = -1;
        createNewline(m_curpos);
        return 0;
    }
    QChar* it = m_content.data();
    it += m_curpos;
    m_tokenBegin = m_curpos;
    switch (state()) {
    case HtmlState:
        if (it->unicode() == '<' && (it + 1)->unicode() == '?'
            ///TODO: per-project configuration to set whether we use shortags
            ///      or not. In the former case we'd need to rise an error here
            && !( (it + 2)->toLower().unicode() == 'x'
                 && (it + 3)->toLower().unicode() == 'm'
                 && (it + 4)->toLower().unicode() == 'l' ) )
        {
            token = Parser::Token_OPEN_TAG;
            if ((it + 2)->unicode() == '=') {
                token = Parser::Token_OPEN_TAG_WITH_ECHO;
                m_curpos++;
                it++;
            } else if ((it + 2)->toLower().unicode() == 'p'
                    && (it + 3)->toLower().unicode() == 'h'
                    && (it + 4)->toLower().unicode() == 'p'
                    && (it + 5)->isSpace()) {
                m_curpos += 4;
                if ((it + 5)->unicode() == '\n') createNewline(m_curpos + 1);
            }
            m_curpos++;
            pushState(DefaultState);
        } else {
            token = Parser::Token_INLINE_HTML;
            while (m_curpos < m_contentSize) {
                if (it->unicode() == '\n') createNewline(m_curpos);
                if ((it + 1)->unicode() == '<' && (it + 2)->unicode() == '?') {
                    break;
                }
                it++;
                m_curpos++;
            }
        }
        break;
    case DefaultState:
    case StringVariableCurly: {
        if (it->isSpace()) {
            token = Parser::Token_WHITESPACE;
            while (m_curpos < m_contentSize && it->isSpace()) {
                if (it->unicode() == '\n') createNewline(m_curpos);
                it++;
                m_curpos++;
            }
            m_curpos--;
        } else if (it->isDigit() || (it->unicode() == '.' && (it + 1)->isDigit())) {
            QString num;bool hasPoint = false;
            bool hex = false;
            bool bin = false;
            if (it->unicode() == '0' && (it + 1)->toLower() == 'x') {
                it += 2;
                m_curpos += 2;
                hex = true;
            }
            if (it->unicode() == '0' && (it + 1)->toLower() == 'b') {
                it += 2;
                m_curpos += 2;
                bin = true;
            }
            while (m_curpos < m_contentSize && (
                        it->isDigit()
                        || (!hex && !hasPoint && it->unicode() == '.')
                        || (bin && (it->unicode() == '0' || it->unicode() == '1'))
                        || (hex && (it->toLower() == 'a' || it->toLower() == 'b' ||
                                    it->toLower() == 'c' || it->toLower() == 'd' ||
                                    it->toLower() == 'e' || it->toLower() == 'f')))) {
                if (it->unicode() == '.') hasPoint = true;
                num.append(*it);
                it++;
                m_curpos++;
            }
            if (!hex && !bin && it->toLower() == 'e' &&
                    ((it + 1)->isDigit() ||
                     (((it + 1)->unicode() == '-' || (it + 1)->unicode() == '+') && (it + 2)->isDigit()))) {
                //exponential number
                token = Parser::Token_DNUMBER;
                m_curpos++;
                it++;
                if (it->unicode() == '-' || it->unicode() == '+') {
                    it++;
                    m_curpos++;
                }
                while (m_curpos < m_contentSize && (it->isDigit())) {
                    it++;
                    m_curpos++;
                }
                m_curpos--;
            } else {
                m_curpos--;
                if (hasPoint) {
                    token = Parser::Token_DNUMBER;
                } else {
                    bool ok;
                    //check if string can be converted to long
                    //if we get an overflow use double
                    num.toLong(&ok, hex ? 16 : 10);
                    if (ok) {
                        token = Parser::Token_LNUMBER;
                    } else {
                        token = Parser::Token_DNUMBER;
                    }
                }
            }

        } else if (processVariable(it)) {
            token = Parser::Token_VARIABLE;
        } else if (it->unicode() == '$') {
            //when it was not recognized as variable
            token = Parser::Token_DOLLAR;
        } else if (it->unicode() == '}') {
            token = Parser::Token_RBRACE;
            if (state() == StringVariableCurly) {
                popState();
            }
        } else if (it->unicode() == '{') {
            token = Parser::Token_LBRACE;
            if (state() == StringVariableCurly) {
                pushState(StringVariableCurly);
            }
        } else if (it->unicode() == ')') {
            token = Parser::Token_RPAREN;
        } else if (it->unicode() == '(') {
            it++;
            int pos = m_curpos + 1;
            while (pos < m_contentSize && it->isSpace()) {
                it++;
                pos++;
            }
            QString name;
            while (pos < m_contentSize && it->isLetter()) {
                name.append(*it);
                it++;
                pos++;
            }
            while (pos < m_contentSize && it->isSpace()) {
                it++;
                pos++;
            }
            name = name.toLower();
            if (it->unicode() == ')') {
                if (name == "int" || name == "integer") {
                    token = Parser::Token_INT_CAST;
                } else if (name == "real" || name == "double" || name == "float") {
                    token = Parser::Token_DOUBLE_CAST;
                } else if (name == "string") {
                    token = Parser::Token_STRING_CAST;
                } else if (name == "binary") {
                    //as in php
                    token = Parser::Token_STRING_CAST;
                } else if (name == "array") {
                    token = Parser::Token_ARRAY_CAST;
                } else if (name == "object") {
                    token = Parser::Token_OBJECT_CAST;
                } else if (name == "bool" || name == "boolean") {
                    token = Parser::Token_BOOL_CAST;
                } else if (name == "unset") {
                    token = Parser::Token_UNSET_CAST;
                } else {
                    token = Parser::Token_LPAREN;
                }

                if (token != Parser::Token_LPAREN) {
                    m_curpos = pos;
                }
            } else {
                token = Parser::Token_LPAREN;
            }
        } else if (it->unicode() == ']') {
            token = Parser::Token_RBRACKET;
        } else if (it->unicode() == '[') {
            token = Parser::Token_LBRACKET;
        } else if (it->unicode() == ',') {
            token = Parser::Token_COMMA;
        } else if (it->unicode() == '@') {
            token = Parser::Token_AT;
        } else if (it->unicode() == '!') {
            if ((it + 1)->unicode() == '=') {
                m_curpos++;
                if ((it + 2)->unicode() == '=') {
                    m_curpos++;
                    token = Parser::Token_IS_NOT_IDENTICAL;
                } else {
                    token = Parser::Token_IS_NOT_EQUAL;
                }
            } else {
                token = Parser::Token_BANG;
            }
        } else if (it->unicode() == '<') {
            if ((it + 1)->unicode() == '<') {
                m_curpos++;
                if ((it + 2)->unicode() == '<' && state() != StringVariableCurly) {
                    //HEREDOC string (<<< EOD\nfoo\nEOD;\n)
                    int pos = 3;
                    while (m_curpos + pos < m_contentSize &&
                            ((it + pos)->unicode() == ' ' || (it + pos)->unicode() == '\t')) {
                        pos++;
                    }
                    bool isNowdoc = (it + pos)->unicode() == '\'';
                    bool foundQuote = isNowdoc || (it + pos)->unicode() == '"';
                    if (foundQuote) {
                        ++pos;
                    }
                    if ((it + pos)->isLetter() || (it + pos)->unicode() == '_') { //identifier must start with a letter
                        m_hereNowDocIdentifier.clear();
                        while (m_curpos + pos < m_contentSize &&
                                ((it + pos)->isDigit() || (it + pos)->isLetter() || (it + pos)->unicode() == '_')) {
                            m_hereNowDocIdentifier.append(*(it + pos));
                            pos++;
                        }
                        if (foundQuote && (m_curpos + pos) < m_contentSize) {
                            if (isNowdoc && (it+pos)->unicode() == '\'') {
                                ++pos;
                            } else if ((it+pos)->unicode() == '"') {
                                ++pos;
                            }
                        }
                        if (m_curpos + pos < m_contentSize && (it + pos)->unicode() == '\n') {
                            //identifier must be followed by newline, newline is part of HEREDOC token
                            if (isNowdoc) {
                                token = Parser::Token_START_NOWDOC;
                                pushState(StringNowdoc);
                            } else {
                                token = Parser::Token_START_HEREDOC;
                                pushState(StringHeredoc);
                            }
                            m_curpos += pos - 1;
                            createNewline(m_curpos);
                        }
                    }
                }

                if (token != Parser::Token_START_HEREDOC && token != Parser::Token_START_NOWDOC) {
                    if ((it + 2)->unicode() == '=') {
                        m_curpos++;
                        token = Parser::Token_SL_ASSIGN;
                    } else {
                        token = Parser::Token_SL;
                    }
                }
            } else if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_IS_SMALLER_OR_EQUAL;
            } else if ((it + 1)->unicode() == '>') {
                m_curpos++;
                token = Parser::Token_IS_NOT_EQUAL;
            } else {
                token = Parser::Token_IS_SMALLER;
            }
        } else if (it->unicode() == '>') {
            if ((it + 1)->unicode() == '>') {
                m_curpos++;
                if ((it + 2)->unicode() == '=') {
                    m_curpos++;
                    token = Parser::Token_SR_ASSIGN;
                } else {
                    token = Parser::Token_SR;
                }
            } else if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_IS_GREATER_OR_EQUAL;
            } else {
                token = Parser::Token_IS_GREATER;
            }
        } else if (it->unicode() == '~') {
            token = Parser::Token_TILDE;
        } else if (it->unicode() == ':') {
            if ((it + 1)->unicode() == ':') {
                m_curpos++;
                token = Parser::Token_PAAMAYIM_NEKUDOTAYIM;
            } else {
                token = Parser::Token_COLON;
            }
        } else if (it->unicode() == '?') {
            if ((it + 1)->unicode() == '>') {
                //accept CLOSE_TAG inside StringVariableCurly too, as php does
                token = Parser::Token_CLOSE_TAG;
                m_curpos++;
                while (state() != HtmlState) popState();
            } else {
                token = Parser::Token_QUESTION;
            }
        } else if (it->unicode() == '-' && (it + 1)->unicode() == '>') {
            m_curpos++;
            token = Parser::Token_OBJECT_OPERATOR;
            if (isValidVariableIdentifier(it + 2)) {
                pushState(StringVariableObjectOperator);
            }
        } else if (it->unicode() == '%') {
            if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_MOD_ASSIGN;
            } else {
                token = Parser::Token_MOD;
            }
        } else if (it->unicode() == '/') {
            if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_DIV_ASSIGN;
            } else if ((it + 1)->unicode() == '/') {
                //accept COMMENT inside StringVariableCurly too, as php does
                if ((it + 2)->unicode() == '/') {
                    token = Parser::Token_DOC_COMMENT;
                } else {
                    token = Parser::Token_COMMENT;
                }
                while (m_curpos < m_contentSize) {
                    if (m_curpos + 1 < m_contentSize && it->unicode() == '?' && (it + 1)->unicode() == '>') {
                        --it;
                        --m_curpos;
                        break;
                    }
                    if ( it->unicode() == '\n' ) {
                        createNewline(m_curpos);
                        if ( token == Parser::Token_COMMENT ) {
                            break;
                        } else {
                            // lookahead to check whether this doc comment spans multiple lines
                            QChar* it2 = it + 1;
                            int pos = m_curpos + 1;
                            while ( pos < m_contentSize && (it2)->isSpace() && (it2)->unicode() != '\n' ) {
                                ++it2;
                                ++pos;
                            }
                            if ( it2->unicode() == '/' && (it2 + 1)->unicode() == '/'
                                 && (it2 + 2)->unicode() == '/' ) {
                                // seems to be a multi-line doc-comment
                                it = it2 + 2;
                                m_curpos = pos + 2;
                                continue;
                            } else {
                                // not a multi-line doc-comment
                                break;
                            }
                        }
                    }
                    it++;
                    m_curpos++;
                }
            } else if ((it + 1)->unicode() == '*') {
                //accept COMMENT inside StringVariableCurly too, as php does
                if ((it + 2)->unicode() == '*' && (it + 3)->isSpace()) {
                    token = Parser::Token_DOC_COMMENT;
                } else {
                    token = Parser::Token_COMMENT;
                }
                it += 2;
                m_curpos += 2;
                while (m_curpos < m_contentSize && !(it->unicode() == '*' && (it + 1)->unicode() == '/')) {
                    if (it->unicode() == '\n') {
                        createNewline(m_curpos);
                    }
                    it++;
                    m_curpos++;
                }
                m_curpos++;
            } else {
                token = Parser::Token_DIV;
            }
        } else if (it->unicode() == '#') {
            //accept COMMENT inside StringVariableCurly too, as php does
            token = Parser::Token_COMMENT;
            while (m_curpos < m_contentSize) {
                if (m_curpos + 1 < m_contentSize && it->unicode() == '?' && (it + 1)->unicode() == '>') {
                    --it;
                    --m_curpos;
                    break;
                }
                if (it->unicode() == '\n') {
                    createNewline(m_curpos);
                    break;
                }
                it++;
                m_curpos++;
            }
        } else if (it->unicode() == '^') {
            if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_XOR_ASSIGN;
            } else {
                token = Parser::Token_BIT_XOR;
            }
        } else if (it->unicode() == '*') {
            if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_MUL_ASSIGN;
            } else {
                token = Parser::Token_MUL;
            }
        } else if (it->unicode() == '|') {
            if ((it + 1)->unicode() == '|') {
                m_curpos++;
                token = Parser::Token_BOOLEAN_OR;
            } else if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_OR_ASSIGN;
            } else {
                token = Parser::Token_BIT_OR;
            }
        } else if (it->unicode() == '&') {
            if ((it + 1)->unicode() == '&') {
                m_curpos++;
                token = Parser::Token_BOOLEAN_AND;
            } else if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_AND_ASSIGN;
            } else {
                token = Parser::Token_BIT_AND;
            }
        } else if (it->unicode() == '+') {
            if ((it + 1)->unicode() == '+') {
                m_curpos++;
                token = Parser::Token_INC;
            } else if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_PLUS_ASSIGN;
            } else {
                token = Parser::Token_PLUS;
            }
        } else if (it->unicode() == '-') {
            if ((it + 1)->unicode() == '-') {
                m_curpos++;
                token = Parser::Token_DEC;
            } else if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_MINUS_ASSIGN;
            } else {
                token = Parser::Token_MINUS;
            }
        } else if (it->unicode() == '.') {
            if ((it + 1)->unicode() == '=') {
                m_curpos++;
                token = Parser::Token_CONCAT_ASSIGN;
            } else {
                token = Parser::Token_CONCAT;
            }
        } else if (it->unicode() == '\\') {
            token = Parser::Token_BACKSLASH;
        } else if (it->unicode() == ';') {
            token = Parser::Token_SEMICOLON;
        } else if (it->unicode() == '\'') {
            token = Parser::Token_CONSTANT_ENCAPSED_STRING;
            it++;
            m_curpos++;
            int startPos = m_curpos;
            while (m_curpos < m_contentSize
                    && (it->unicode() != '\'' || isEscapedWithBackslash(it, m_curpos, startPos))) {
                if (it->unicode() == '\n') createNewline(m_curpos);
                it++;
                m_curpos++;
            }
            // if the string is never terminated, make sure we don't overflow the boundaries
            if ( m_curpos == m_contentSize ) {
                --m_curpos;
            }
        } else if (it->unicode() == '"') {
            it++;
            m_curpos++;
            int stringSize = 0;
            bool foundVar = false;
            while (m_curpos + stringSize < m_contentSize
                    && (it->unicode() != '"' || isEscapedWithBackslash(it, m_curpos + stringSize, m_curpos)))
            {
                if (it->unicode() == '$'  && !isEscapedWithBackslash(it, m_curpos + stringSize, m_curpos)
                        && ((it + 1)->unicode() == '{'
                            || (isValidVariableIdentifier(it + 1) && !(it + 1)->isDigit()))) {
                    foundVar = true;
                    break;
                }
                it++;
                stringSize++;
            }
            if (!foundVar) {
                // if the string is never terminated, make sure we don't overflow the boundaries
                if ( m_curpos + stringSize == m_contentSize ) {
                    m_curpos--;
                }
                token = Parser::Token_CONSTANT_ENCAPSED_STRING;
                it -= stringSize;
                for (int j = 0; j < stringSize; j++) {
                    if (it->unicode() == '\n') {
                        createNewline(m_curpos + j);
                    }
                    it++;
                }
                m_curpos += stringSize;
            } else {
                // properly set the token pos to the starting double quote
                m_curpos--;
                token = Parser::Token_DOUBLE_QUOTE;
                pushState(String);
            }
        } else if (it->unicode() == '`') {
            token = Parser::Token_BACKTICK;
            pushState(StringBacktick);
        } else if (it->unicode() == '=') {
            if ((it + 1)->unicode() == '=') {
                m_curpos++;
                if ((it + 2)->unicode() == '=') {
                    m_curpos++;
                    token = Parser::Token_IS_IDENTICAL;
                } else {
                    token = Parser::Token_IS_EQUAL;
                }
            } else if ((it + 1)->unicode() == '>') {
                m_curpos++;
                token = Parser::Token_DOUBLE_ARROW;
            } else {
                token = Parser::Token_ASSIGN;
            }
        } else if (isValidVariableIdentifier(it) && !it->isDigit()) {
            QString name;
            while (m_curpos < m_contentSize && (isValidVariableIdentifier(it))) {
                name.append(*it);
                it++;
                m_curpos++;
            }
            m_curpos--;
            name = name.toLower();
            if (name == "echo") {
                token = Parser::Token_ECHO;
            } else if (name == "include") {
                token = Parser::Token_INCLUDE;
            } else if (name == "include_once") {
                token = Parser::Token_INCLUDE_ONCE;
            } else if (name == "require") {
                token = Parser::Token_REQUIRE;
            } else if (name == "require_once") {
                token = Parser::Token_REQUIRE_ONCE;
            } else if (name == "eval") {
                token = Parser::Token_EVAL;
            } else if (name == "print") {
                token = Parser::Token_PRINT;
            } else if (name == "abstract") {
                token = Parser::Token_ABSTRACT;
            } else if (name == "break") {
                token = Parser::Token_BREAK;
            } else if (name == "case") {
                token = Parser::Token_CASE;
            } else if (name == "catch") {
                token = Parser::Token_CATCH;
            } else if (name == "class") {
                token = Parser::Token_CLASS;
            } else if (name == "const") {
                token = Parser::Token_CONST;
            } else if (name == "continue") {
                token = Parser::Token_CONTINUE;
            } else if (name == "default") {
                token = Parser::Token_DEFAULT;
            } else if (name == "do") {
                token = Parser::Token_DO;
            } else if (name == "else") {
                token = Parser::Token_ELSE;
            } else if (name == "extends") {
                token = Parser::Token_EXTENDS;
            } else if (name == "final") {
                token = Parser::Token_FINAL;
            } else if (name == "for") {
                token = Parser::Token_FOR;
            } else if (name == "if") {
                token = Parser::Token_IF;
            } else if (name == "implements") {
                token = Parser::Token_IMPLEMENTS;
            } else if (name == "instanceof") {
                token = Parser::Token_INSTANCEOF;
            } else if (name == "insteadof") {
                token = Parser::Token_INSTEADOF;
            } else if (name == "interface") {
                token = Parser::Token_INTERFACE;
            } else if (name == "trait") {
                token = Parser::Token_TRAIT;
            } else if (name == "new") {
                token = Parser::Token_NEW;
            } else if (name == "private") {
                token = Parser::Token_PRIVATE;
            } else if (name == "protected") {
                token = Parser::Token_PROTECTED;
            } else if (name == "public") {
                token = Parser::Token_PUBLIC;
            } else if (name == "return") {
                token = Parser::Token_RETURN;
            } else if (name == "static") {
                QChar* lookAhead = it;
                int pos = m_curpos;
                while (pos < m_contentSize && lookAhead->isSpace()) {
                    ++lookAhead;
                    ++pos;
                }
                if (pos + 1 < m_contentSize && lookAhead->unicode() == ':' && (++lookAhead)->unicode() == ':') {
                    // PHP 5.3 - late static
                    token = Parser::Token_STRING;
                } else {
                    token = Parser::Token_STATIC;
                }
            } else if (name == "switch") {
                token = Parser::Token_SWITCH;
            } else if (name == "throw") {
                token = Parser::Token_THROW;
            } else if (name == "try") {
                token = Parser::Token_TRY;
            } else if (name == "while") {
                token = Parser::Token_WHILE;
            } else if (name == "clone") {
                token = Parser::Token_CLONE;
            } else if (name == "exit" || name == "die") {
                token = Parser::Token_EXIT;
            } else if (name == "elseif") {
                token = Parser::Token_ELSEIF;
            } else if (name == "endif") {
                token = Parser::Token_ENDIF;
            } else if (name == "endwhile") {
                token = Parser::Token_ENDWHILE;
            } else if (name == "endfor") {
                token = Parser::Token_ENDFOR;
            } else if (name == "foreach") {
                token = Parser::Token_FOREACH;
            } else if (name == "endforeach") {
                token = Parser::Token_ENDFOREACH;
            } else if (name == "declare") {
                token = Parser::Token_DECLARE;
            } else if (name == "enddeclare") {
                token = Parser::Token_ENDDECLARE;
            } else if (name == "as") {
                token = Parser::Token_AS;
            } else if (name == "endswitch") {
                token = Parser::Token_ENDSWITCH;
            } else if (name == "function") {
                token = Parser::Token_FUNCTION;
            } else if (name == "use") {
                token = Parser::Token_USE;
            } else if (name == "goto") {
                token = Parser::Token_GOTO;
            } else if (name == "global") {
                token = Parser::Token_GLOBAL;
            } else if (name == "var") {
                token = Parser::Token_VAR;
            } else if (name == "unset") {
                token = Parser::Token_UNSET;
            } else if (name == "isset") {
                token = Parser::Token_ISSET;
            } else if (name == "empty") {
                token = Parser::Token_EMPTY;
            } else if (name == "__halt_compiler") {
                token = Parser::Token_HALT_COMPILER;
            } else if (name == "list") {
                token = Parser::Token_LIST;
            } else if (name == "array") {
                token = Parser::Token_ARRAY;
            } else if (name == "__class__") {
                token = Parser::Token_CLASS_C;
            } else if (name == "__method__") {
                token = Parser::Token_METHOD_C;
            } else if (name == "__function__") {
                token = Parser::Token_FUNC_C;
            } else if (name == "__line__") {
                token = Parser::Token_LINE;
            } else if (name == "__file__") {
                token = Parser::Token_FILE;
            } else if (name == "or") {
                token = Parser::Token_LOGICAL_OR;
            } else if (name == "and") {
                token = Parser::Token_LOGICAL_AND;
            } else if (name == "xor") {
                token = Parser::Token_LOGICAL_XOR;
            } else if (name == "namespace") {
                token = Parser::Token_NAMESPACE;
            } else if (name == "__namespace__") {
                token = Parser::Token_NAMESPACE_C;
            } else {
                token = Parser::Token_STRING;
            }
        }
        break;
    }

    case StringVariable:
    case String:
    case StringHeredoc:
    case StringBacktick:
        if ((state() == String || state(1) == String) && it->unicode() == '"') {
            token = Parser::Token_DOUBLE_QUOTE;
            if (state() == StringVariable) popState();
            popState();
        } else if ((state() == StringBacktick || state(1) == StringBacktick) && it->unicode() == '`') {
            token = Parser::Token_BACKTICK;
            if (state() == StringVariable) popState();
            popState();
        } else if ((state() == StringHeredoc || state(1) == StringHeredoc) && isHereNowDocEnd(it)) {
            token = Parser::Token_END_HEREDOC;
            m_curpos += m_hereNowDocIdentifier.length() - 1;
            if (state() == StringVariable) popState();
            popState();
        } else if (processVariable(it)) {
            token = Parser::Token_VARIABLE;
            if (state() != StringVariable) pushState(StringVariable);
        } else if (state() != StringVariable  && it->unicode() == '$' && (it + 1)->unicode() == '{') {
            token = Parser::Token_DOLLAR_OPEN_CURLY_BRACES;
            m_curpos++;
            it += 2;
            //check if a valid variable follows
            if ((isValidVariableIdentifier(it) && !it->isDigit())) {
                pushState(StringVarname);
            }

        } else if (state() == StringVariable && it->unicode() == '[') {
            token = Parser::Token_LBRACKET;
            pushState(StringVariableBracket);
        } else if (state() != StringVariable && it->unicode() == '{' && (it + 1)->unicode() == '$'
                   && ((isValidVariableIdentifier(it + 2) && !(it + 2)->isDigit()) || (it + 2)->unicode() == '{')) {
            token = Parser::Token_CURLY_OPEN;
            pushState(StringVariableCurly);
        } else if (state() == StringVariable
                   && it->unicode() == '-' && (it + 1)->unicode() == '>'
                   && isValidVariableIdentifier(it + 2) && !(it + 2)->isDigit()) {
            token = Parser::Token_OBJECT_OPERATOR;
            m_curpos++;
            pushState(StringVariableObjectOperator);
        } else {
            if (state() == StringVariable) popState();
            token = Parser::Token_ENCAPSED_AND_WHITESPACE;
            int startPos = m_curpos;
            while (m_curpos < m_contentSize) {
                if (!isEscapedWithBackslash(it, m_curpos, startPos) &&
                        ((it->unicode() == '$' && (it + 1)->unicode() == '{') ||
                         (it->unicode() == '{' && (it + 1)->unicode() == '$' && isValidVariableIdentifier(it + 2)) ||
                         (it->unicode() == '$' && isValidVariableIdentifier(it + 1) && !(it + 1)->isDigit()))) {
                    //variable is next ${var} or {$var}
                    break;
                }
                if (state() == String && it->unicode() == '"'
                        && !isEscapedWithBackslash(it, m_curpos, startPos)) {
                    //end of string
                    break;
                }
                if (state() == StringBacktick && it->unicode() == '`'
                        && !isEscapedWithBackslash(it, m_curpos, startPos)) {
                    //end of string
                    break;
                }

                if (it->unicode() == '\n') createNewline(m_curpos);
                m_curpos++;
                it++;

                if (state() == StringHeredoc && (it - 1)->unicode() == '\n') {
                    //check for end of heredoc (\nEOD;\n)
                    if (state() == StringHeredoc && isHereNowDocEnd(it)) {
                        break;
                    }
                }
            }
            m_curpos--;
        }
        break;
    case StringNowdoc:
        if (isHereNowDocEnd(it)) {
            token = Parser::Token_END_NOWDOC;
            m_curpos += m_hereNowDocIdentifier.length() - 1;
            popState();
        } else {
            token = Parser::Token_STRING;
            while (m_curpos < m_contentSize) {
                if (it->unicode() == '\n') createNewline(m_curpos);
                m_curpos++;
                it++;

                if ((it - 1)->unicode() == '\n' && isHereNowDocEnd(it)) {
                    //check for end of nowdoc (\nEOD;\n)
                    break;
                }
            }
            m_curpos--;
        }
        break;
    case StringVariableBracket:
        if (it->unicode() == ']') {
            token = Parser::Token_RBRACKET;
            popState();
            popState();
        } else if (it->isDigit()) {
            token = Parser::Token_NUM_STRING;
            while (m_curpos < m_contentSize && it->isDigit()) {
                it++;
                m_curpos++;
            }
            m_curpos--;
        } else {
            token = Parser::Token_STRING;
            while (m_curpos < m_contentSize && (it->unicode() != ']')) {
                if (it->unicode() == '\n') createNewline(m_curpos);
                it++;
                m_curpos++;
            }
            m_curpos--;
        }
        break;
    case StringVariableObjectOperator:
        token = Parser::Token_STRING;
        while (m_curpos < m_contentSize && isValidVariableIdentifier(it)) {
            it++;
            m_curpos++;
        }
        m_curpos--;
        popState();
        if (state() == StringVariable) popState();
        break;
    case StringVarname:
        popState();
        pushState(StringVariableCurly);
        token = Parser::Token_STRING_VARNAME;
        while (m_curpos < m_contentSize && isValidVariableIdentifier(it)) {
            it++;
            m_curpos++;
        }
        m_curpos--;
        break;
    default:
        token = Parser::Token_INVALID;
        break;
    }
    if (m_curpos > m_contentSize) {
        m_tokenBegin = -1;
        m_tokenEnd = -1;
        return 0;
    }
    m_tokenEnd = m_curpos;
    m_curpos++;

    if (m_haltCompiler) {
        //look for __halt_compiler(); and stop lexer there
        if (m_haltCompiler == 4) {
            token = 0; //EOF
        } else if (token == Parser::Token_WHITESPACE || token == Parser::Token_COMMENT || token == Parser::Token_DOC_COMMENT) {
            //ignore
        } else if (m_haltCompiler == 1 && token == Parser::Token_LPAREN) {
            m_haltCompiler++;
        } else if (m_haltCompiler == 2 && token == Parser::Token_RPAREN) {
            m_haltCompiler++;
        } else if (m_haltCompiler == 3 && token == Parser::Token_SEMICOLON) {
            m_haltCompiler++;
        } else {
            m_haltCompiler = 0;
        }
    }
    if (token == Parser::Token_HALT_COMPILER && !m_haltCompiler) {
        m_haltCompiler = 1;
    }
    return token;
}

qint64 Lexer::tokenBegin() const
{
    return m_tokenBegin;
}

qint64 Lexer::tokenEnd() const
{
    return m_tokenEnd;
}

bool Lexer::isHereNowDocEnd(QChar* it)
{
    int identiferLen = m_hereNowDocIdentifier.length();
    QString lineStart;
    for (int i = 0; i < identiferLen; i++) {
        if (m_curpos + i >= m_contentSize) break;
        lineStart.append(*(it + i));
    }
    if (lineStart == m_hereNowDocIdentifier &&
            ((it + identiferLen)->unicode() == '\n'
             || ((it + identiferLen)->unicode() == ';' &&
                 (it + identiferLen + 1)->unicode() == '\n'))) {
        return true;
    }
    return false;
}

//used for strings, to check if " is escaped (\" is, \\" not)
bool Lexer::isEscapedWithBackslash(QChar* it, int curPos, int startPos)
{
    int cnt = 0;
    it--;
    while (curPos > startPos && it->unicode() == '\\') {
        cnt++;
        it--;
    }
    return (cnt % 2) == 1;
}

bool Lexer::processVariable(QChar* it)
{
    QChar* c2 = it + 1;
    if (it->unicode() == '$' && (isValidVariableIdentifier(c2) && !c2->isDigit())) {
        it++;
        m_curpos++;
        while (m_curpos < m_contentSize
                && (isValidVariableIdentifier(it))) {
            it++;
            m_curpos++;
        }
        m_curpos--;
        return true;
    } else {
        return false;
    }
}
bool Lexer::isValidVariableIdentifier(QChar* it)
{
    return it->isLetter() || it->isDigit() || it->unicode() == '_' || it->unicode() > 0x7f;
}

void Lexer::createNewline(int pos)
{
    if (m_tokenStream) m_tokenStream->locationTable()->newline(pos);
}

}

