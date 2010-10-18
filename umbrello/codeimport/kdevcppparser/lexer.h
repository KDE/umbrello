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

#ifndef LEXER_H
#define LEXER_H

#include "preprocesslexer.h"
/*
#include <QChar>


namespace boost { namespace spirit { namespace classic { namespace impl {
  bool isalnum_( QChar const& c);
  bool isalpha_( QChar const& c);
  bool isblank_( QChar const& c);
  bool isdigit_( QChar const& c);
}}}}
*/

#include "driver.h"
#include "position.h"

#include <list>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <assert.h>

#include "skip_rule.hpp"

using boost::spirit::classic::parse_info;
using boost::spirit::classic::rule;
using boost::spirit::classic::scanner;
using boost::spirit::classic::ext::skip_rule_parser;

typedef boost::spirit::classic::position_iterator<QChar const*> CharIterator;
typedef rule<scanner<CharIterator> > SkipRule;
typedef skip_rule_parser<SkipRule, CharIterator> CharParser;
typedef scanner<CharIterator> CharScanner;
typedef CharParser::skip_scanner_policies_t CharPolicies;

struct LexerData;

class Lexer
{
    typedef std::list<Token> TokenList;
public:
    typedef rule<scanner<CharIterator, CharPolicies> > CharRule;
    typedef TokenList::const_iterator TokenIterator;

    Lexer( Driver* driver );
    ~Lexer();

    void addSkipWord(const QString& word, SkipType skipType = SkipWord,
                     const QString& str = QString())
    { m_preprocessLexer.addSkipWord( word, skipType, str); }

    void setSource( const QString& source, PositionFilename const& p_filename);
    void setRecordComments( bool record );
    Position currentPosition() const { return m_source.get_currentPosition(); }

    Token const& lookAhead( TokenIterator p_it, int n ) const {
        std::advance( p_it, n);
        assert( p_it != m_tokens.end());
        return *p_it;
    }
    Position const& getTokenPosition(const Token& token) const;
    TokenIterator tokenBegin() const {return m_tokens.begin();}
private:
    void tokenize();
    void nextToken(Token& token);
    bool recordComments() const;
    void reset();
    void handleDirective(const QString& directive);
private:
    static SkipRule m_SkipRule;

    Driver* m_driver;
    TokenList m_tokens;

    class Source
    {
    public:
        Source() {}

        Token createToken( int type, CharIterator start, CharIterator end) const;
        Token createToken( int type, CharIterator start) const {
            return createToken( type, start, m_ptr);
        }
        QChar currentChar() const {
            return m_ptr != m_endPtr ? *m_ptr : QChar::null;
        }
        bool eof() const {return m_ptr == m_endPtr;}
        int length() const {return std::distance(m_ptr, m_endPtr);}
        void nextChar() {
            QChar l_current = *m_ptr++;
            switch( l_current.toAscii()) {
                case '\n':
                case '\r':
                    m_startLine = true;
                    break;
            }
        }

        template <typename _RuleT>
        parse_info<CharIterator> parse( _RuleT const& p_rule) {
            parse_info<CharIterator> l_return =
                CharParser::parse( m_ptr, m_endPtr, p_rule, m_SkipRule);
            if (l_return.hit)
                m_ptr = l_return.stop;
            return l_return;
        }

        void reset() {
            m_ptr = CharIterator();
            m_startLine = true;
        }

        void set_filename( PositionFilename const& p_filename) { m_filename = p_filename; }
        void set_source( QString const& source) {
            m_ptr = CharIterator(source.data(),
                                 source.data() + source.length(),
                                 Position( m_filename));
        }
        // getters
        Position get_currentPosition() const {return m_ptr.get_position();}
        CharIterator get_ptr() const {return m_ptr;}
        bool get_startLine() const {return m_startLine;}
        // setters
        void set_startLine( bool p) {m_startLine = p;}
    private:
        PositionFilename m_filename;
        CharIterator m_ptr;
        const CharIterator m_endPtr;
        bool m_startLine;
    };

    Source m_source;
    bool m_recordComments;
    PreprocessLexer m_preprocessLexer;
};

inline bool Lexer::recordComments() const
{
    return m_recordComments;
}

inline void Lexer::setRecordComments( bool record)
{
    m_preprocessLexer.setRecordComments( record);
    m_recordComments = record;
}

#endif
