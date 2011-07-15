/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "lexer.h"

#include "debug_utils.h"
#include "lookup.h"
#include "keywords.lut.h"

#include <klocale.h>

#include <QtCore/QRegExp>
#include <QtCore/QMap>
#include <QtCore/QList>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/spirit/include/classic_if.hpp>
#include <boost/spirit/include/phoenix1_functions.hpp>

#include "assignFunctor.hpp"

namespace boost {
    namespace spirit {
        namespace classic {
            namespace impl {
                bool isalnum_(QChar const& c) { return isalnum_(c.toAscii()); }
                bool isalpha_(QChar const& c) { return isalpha_(c.toAscii()); }
                bool isblank_(QChar const& c) { return isblank_(c.toAscii()); }
                bool isdigit_(QChar const& c) { return isdigit_(c.toAscii()); }
            }
        }
    }
}

template <class _Tp>
struct tilde : public std::unary_function<_Tp, _Tp> {
    _Tp operator()(_Tp& __x) const {
        return ~__x;
    }
};

using namespace boost::spirit::classic;
using phoenix::arg1;
using phoenix::arg2;
using phoenix::arg3;
using phoenix::construct_;
using phoenix::function;
using phoenix::var;

SkipRule Lexer::m_SkipRule = nothing_p;

#if defined( KDEVELOP_BGPARSER )
#include <QThread>

class KDevTread: public QThread
{
public:
    static void yield() { msleep(0); }
};

inline void qthread_yield()
{
    KDevTread::yield();
}

#endif

/** Utility closure that defines a result value.
    Used to return values from grammars (copied from cpp lexer in spirit
    repository). */
template < typename ResultT >
struct result_closure : closure<result_closure<ResultT>, ResultT> {
    typedef closure<result_closure<ResultT>, ResultT> base_t;
    typename base_t::member1 result_;
};

/** Stuff to construct a QString from iterators */
struct constructQString_impl {
    template <typename _Arg1, typename _Arg2>
    struct result {
        typedef QString type;
    };

    template <typename _Arg1, typename _Arg2>
    QString operator()(_Arg1 const& first, _Arg2 const& last) {
        return QString(&*first, &*last - &*first);
    }
};

const function<constructQString_impl> constructQString =
    constructQString_impl();

struct identifier :
            grammar<identifier, result_closure<QString>::context_t> {
    template < typename ScannerT >
    struct definition {
        typedef rule<ScannerT> rule_t;
        rule_t main;

        rule_t const& start() const {
            return main;
        }

        definition(identifier const& self) {
            main = (lexeme_d[
                        ((alpha_p | '_') >> *(alnum_p | '_'))
                        [self.result_ = constructQString(arg1, arg2)]
                    ]);
        }
    };
} identifier_g;

struct operator_ :
            grammar<operator_, result_closure<Token>::context_t> {
    template < typename ScannerT >
    struct definition {
        typedef rule<ScannerT, result_closure<int>::context_t> rule_t;
        rule_t main;

        rule_t const& start() const {
            return main;
        }

        definition(operator_ const& self) {
            main =
                (str_p("::")[ main.result_ = Token_scope]
                 | (str_p("->*") | ".*")[ main.result_ = Token_ptrmem]
                 | (str_p("<<=") | ">>=" | "+=" | "-=" | "*=" | "/=" | "%=" | "^=" | "&=" | "|=")[ main.result_ = Token_assign]
                 | (str_p("<<") | ">>")[ main.result_ = Token_shift]
                 | (str_p("==") | "!=")[ main.result_ = Token_eq]
                 | str_p("<=")[ main.result_ = Token_leq]
                 | str_p(">=")[ main.result_ = Token_geq]
                 | str_p("&&")[ main.result_ = Token_and]
                 | str_p("||")[ main.result_ = Token_or]
                 | str_p("++")[ main.result_ = Token_incr]
                 | str_p("--")[ main.result_ = Token_decr]
                 | str_p("->")[ main.result_ = Token_arrow]
                 | str_p("##")[ main.result_ = Token_concat]
                 | str_p("...")[ main.result_ = Token_ellipsis]
                )
                [ self.result_ = construct_<Token>(main.result_, arg1, arg2)];
        }
    };
} operator_g;

struct charLiteral :
            grammar<charLiteral, result_closure<Token>::context_t> {
    template < typename ScannerT >
    struct definition {
        typedef rule<ScannerT, result_closure<int>::context_t> rule_t;
        rule_t main;

        rule_t const& start() const {
            return main;
        }

        definition(charLiteral const& self) {
            main =
                (!ch_p('L') >> ch_p('\'')
                 >> *((anychar_p - '\'' - '\\')
                      | (ch_p('\\') >> (ch_p('\'') | '\\')))
                 >> '\'')
                [ self.result_ = construct_<Token>(Token_char_literal, arg1, arg2)];
        }
    };
} charLiteral_g;

struct numberLiteral :
            grammar<numberLiteral, result_closure<Token>::context_t> {
    template < typename ScannerT >
    struct definition {
        typedef rule<ScannerT, result_closure<int>::context_t> rule_t;
        rule_t main;

        rule_t const& start() const {
            return main;
        }

        definition(numberLiteral const& self) {
            main =
                ( + digit_p)
                [ self.result_ = construct_<Token>(Token_number_literal, arg1, arg2)];
        }
    };
} numberLiteral_g;

typedef std::pair<QString, int> Dependency;

struct DependencyClosure
            : boost::spirit::classic::closure<DependencyClosure, QString, int> {
    member1 m_word;
    member2 m_scope;
};

Lexer::CharRule gr_stringLiteral =
    ch_p('"') >> *((anychar_p - '"' - '\\') | str_p("\\\"") | "\\\\") >> '"';
Lexer::CharRule gr_whiteSpace = blank_p | (ch_p('\\') >> eol_p);
Lexer::CharRule gr_lineComment = (str_p("//") >> (*(anychar_p - eol_p)));
Lexer::CharRule gr_multiLineComment = confix_p("/*", *anychar_p, "*/");
Lexer::CharRule gr_skipTillEol =
    *(gr_whiteSpace | gr_lineComment | gr_multiLineComment | charLiteral_g
      | gr_stringLiteral | (anychar_p - '\\' - eol_p) | (ch_p('\\') >> eol_p));

Token::Token()
        : m_type(-1),
        m_start(),
        m_end(),
        m_text()
{
}

Token::Token(int type, CharIterator start, CharIterator end)
        : m_type(type),
        m_start(start.get_position()),
        m_end(end.get_position()),
        m_text(&*start, &*end - &*start)
{
}

Token& Token::operator=(Token const & p)
{
    if (this != &p) {
        m_type = p.m_type;
        m_start = p.m_start;
        m_end = p.m_end;
        m_text = p.m_text;
    }
    return *this;
}

Token Lexer::Source::createToken(int type, CharIterator start,
                                 CharIterator end) const
{
    return Token(type, start, end);
}

using namespace std;

struct LexerData {
    typedef QMap<QString, QString> Scope;
    typedef QList<Scope> StaticChain;

    StaticChain staticChain;

    void beginScope() {
        Scope scope;
        staticChain.push_front(scope);
    }

    void endScope() {
        staticChain.pop_front();
    }

    void bind(const QString& name, const QString& value) {
        Q_ASSERT(staticChain.size() > 0);
        staticChain.front().insert(name, value);
    }

    bool hasBind(const QString& name) const {
        StaticChain::ConstIterator it = staticChain.begin();
        while (it != staticChain.end()) {
            const Scope& scope = *it;
            ++it;

            if (scope.contains(name))
                return true;
        }

        return false;
    }

    QString apply(const QString& name) const {
        StaticChain::ConstIterator it = staticChain.begin();
        while (it != staticChain.end()) {
            const Scope& scope = *it;
            ++it;

            if (scope.contains(name))
                return scope[ name ];
        }

        return QString();
    }

};

Lexer::Lexer(Driver* driver)
        : m_driver(driver),
        m_recordComments(false),
        m_preprocessLexer(m_driver)
{
    reset();
}

Lexer::~Lexer()
{
}

void Lexer::setSource(const QString& source,
                      PositionFilename const& p_filename)
{
    reset();
    m_preprocessLexer.setSource(source, p_filename);
    m_source.set_filename(p_filename);
    tokenize();
}

void Lexer::reset()
{
    m_tokens.clear();
    m_source.reset();
}

Position const& Lexer::getTokenPosition(const Token& token) const
{
    return token.getStartPosition();
}

void Lexer::nextToken(Token& tk)
{
    m_source.parse(*gr_whiteSpace);

    Position startPosition(currentPosition());

    QChar ch = m_source.currentChar();
    if (ch.isNull() || ch.isSpace()) {
        /* skip */
    } else if (m_source.get_startLine()
               && m_source.parse(ch_p('#') >> *gr_whiteSpace).hit) {
        m_source.set_startLine(false);

        QString directive;
        m_source.parse(identifier_g[ assign(directive)]);  // read the directive

        handleDirective(directive);
    } else if (m_source.parse
               (
                   if_p(var(m_recordComments))
                   [ gr_lineComment | gr_multiLineComment
                     [var(tk) = construct_<Token>(Token_comment, arg1, arg2)]
                   ]
                   .else_p[ gr_lineComment | gr_multiLineComment]
                   | charLiteral_g[assign(tk)]
                   |
                   gr_stringLiteral
                   [var(tk) = construct_<Token>(Token_string_literal, arg1, arg2)]
               ).hit) {
    } else if (ch.isLetter() || ch == '_') {
        CharIterator start = m_source.get_ptr();
        QString ide;
        m_source.parse(identifier_g[assign(ide)]);
        int k = Lookup::find(&keyword, ide);
        if (k != -1) {
            tk = m_source.createToken(k, start);
        } else {
            tk = m_source.createToken(Token_identifier, start);
        }
    } else if (m_source.parse(numberLiteral_g[assign(tk)]
                              | operator_g[ assign(tk)]
                             ).hit) {
    } else {
        CharIterator l_ptr = m_source.get_ptr();
        m_source.nextChar();
        tk = m_source.createToken(ch.unicode(), l_ptr);
    }
    if (m_source.parse(eol_p).hit)
        m_source.set_startLine(true);
    else
        m_source.set_startLine(false);
}

void Lexer::tokenize()
{
    m_preprocessLexer.preprocess();
#if 0
    QByteArray l_tmp = m_preprocessLexer.preprocessedString().toAscii();
    for (int i = 0; i < l_tmp.size(); ++i)
        std::cout << l_tmp.at(i);
    std::cout << std::endl;
#endif
    m_source.set_source(m_preprocessLexer.preprocessedString());
    m_source.set_startLine(true);
    for (;;) {
        Token tk;
        nextToken(tk);

        if (tk.type() != -1)
            m_tokens.push_back(tk);

        if (m_source.currentChar().isNull())
            break;
    }

    Token tk = m_source.createToken(Token_eof, m_source.get_ptr());
    m_tokens.push_back(tk);
}

void Lexer::handleDirective(const QString& directive)
{
    Q_UNUSED(directive);
    assert(directive != "define");
    assert(directive != "else");
    assert(directive != "elif");
    assert(directive != "endif");
    assert(directive != "if");
    assert(directive != "ifdef");
    assert(directive != "ifndef");
    assert(directive != "include");
    assert(directive != "undef");
    assert(0);
    // skip line
    m_source.parse(gr_skipTillEol);
}

// *IMPORTANT*
// please, don't include lexer.moc here, because Lexer isn't a QObject class!!
// if you have problem while recompiling try to remove cppsupport/.deps,
// cppsupport/Makefile.in and rerun automake/autoconf

