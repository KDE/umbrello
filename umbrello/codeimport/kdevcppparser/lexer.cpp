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
#include "lookup.h"
#include "keywords.lut.h"

#include <kdebug.h>
#include <klocale.h>

#include <qregexp.h>
#include <qmap.h>
#include <q3valuelist.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/spirit/dynamic/if.hpp>
#include <boost/spirit/phoenix/functions.hpp>

#include "assignFunctor.hpp"

namespace boost { namespace spirit { namespace impl {
  bool isalnum_( QChar const& c) {return isalnum_( c.toAscii());}
  bool isblank_( QChar const& c) {return isblank_( c.toAscii());}
  bool isdigit_( QChar const& c) {return isdigit_( c.toAscii());}
}}}

template <class _Tp>
struct tilde : public std::unary_function<_Tp,_Tp> {
  _Tp operator()(_Tp& __x) const { return ~__x; }
};

using namespace boost::spirit;
using phoenix::arg1;
using phoenix::arg2;
using phoenix::arg3;
using phoenix::construct_;
using phoenix::function;
using phoenix::var;

SkipRule Lexer::m_SkipRule = nothing_p;

#if defined( KDEVELOP_BGPARSER )
#include <qthread.h>

class KDevTread: public QThread
{
public:
    static void yield()
    {
	msleep( 0 );
    }
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
  QString operator()( _Arg1 const& first, _Arg2 const& last) {
    return QString( &*first, &*last - &*first);
  }
};

const function<constructQString_impl> constructQString =
  constructQString_impl();

struct identifier :
  grammar<identifier, result_closure<QString>::context_t>
{
  template < typename ScannerT >
  struct definition {
    typedef rule<ScannerT> rule_t;
    rule_t main;

    rule_t const& start() const {return main;}

    definition( identifier const& self) {
      main = (lexeme_d[
		       (+(alnum_p | '_'))
		       [self.result_ = constructQString(arg1, arg2)]
		       ]);
    }
  };
} identifier_g;

struct operator_ :
  grammar<operator_, result_closure<Token>::context_t>
{
  template < typename ScannerT >
  struct definition {
    typedef rule<ScannerT, result_closure<int>::context_t> rule_t;
    rule_t main;

    rule_t const& start() const {return main;}

    definition( operator_ const& self) {
      main =
	(str_p("::")[ main.result_ = Token_scope]
	 | (str_p("->*") | ".*")[ main.result_ = Token_ptrmem]
	 | (str_p("<<=") | ">>=" | "+=" | "-=" | "*=" | "/=" | "%=" | "^="
	    | "&=" | "|=")
	 [ main.result_ = Token_assign]
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
	[ self.result_ = construct_<Token>( main.result_, arg1, arg2)];
    }
  };
} operator_g;

struct charLiteral :
  grammar<charLiteral, result_closure<Token>::context_t>
{
  template < typename ScannerT >
  struct definition {
    typedef rule<ScannerT, result_closure<int>::context_t> rule_t;
    rule_t main;

    rule_t const& start() const {return main;}

    definition( charLiteral const& self) {
      main =
	(!ch_p('L') >> ch_p('\'')
	 >> *((anychar_p - '\'' - '\\')
	      | (ch_p('\\') >> (ch_p('\'') | '\\')))
	 >> '\'')
	[ self.result_ = construct_<Token>( Token_char_literal, arg1, arg2)];
    }
  };
} charLiteral_g;

struct numberLiteral :
  grammar<numberLiteral, result_closure<Token>::context_t>
{
  template < typename ScannerT >
  struct definition {
    typedef rule<ScannerT, result_closure<int>::context_t> rule_t;
    rule_t main;

    rule_t const& start() const {return main;}

    definition( numberLiteral const& self) {
      main =
	(+digit_p)
	[ self.result_ = construct_<Token>( Token_number_literal, arg1, arg2)];
    }
  };
} numberLiteral_g;

Lexer::CharRule gr_stringLiteral =
  ch_p('"') >> *((anychar_p - '"' - '\\') | str_p("\\\"") | "\\\\") >> '"';
Lexer::CharRule gr_whiteSpaces = *(blank_p | (ch_p('\\') >> eol_p));
Lexer::CharRule gr_lineComment = (str_p("//") >> (*(anychar_p - eol_p)));
Lexer::CharRule gr_multiLineComment = confix_p( "/*", *anychar_p, "*/");

Token::Token()
  : m_type( -1 ),
    m_start(),
    m_end(),
    m_text()
{
}

Token::Token( int type, CharIterator start, CharIterator end)
  : m_type( type ),
    m_start( start.get_position()),
    m_end( end.get_position()),
    m_text( &*start, &*end - &*start)
{}

Token& Token::operator=( Token const& p) {
  if( this != &p) {
    m_type = p.m_type;
    m_start = p.m_start;
    m_end = p.m_end;
    m_text = p.m_text;
  }
  return *this;
}

Token Lexer::Source::createToken( int type, CharIterator start,
				  CharIterator end) const
{return Token( type, start, end);}

using namespace std;

struct LexerData
{
    typedef QMap<QString, QString> Scope;
    typedef Q3ValueList<Scope> StaticChain;

    StaticChain staticChain;

    void beginScope()
    {
        Scope scope;
        staticChain.push_front( scope );
    }

    void endScope()
    {
        staticChain.pop_front();
    }

    void bind( const QString& name, const QString& value )
    {
        Q_ASSERT( staticChain.size() > 0 );
        staticChain.front().insert( name, value );
    }

    bool hasBind( const QString& name ) const
    {
        StaticChain::ConstIterator it = staticChain.begin();
        while( it != staticChain.end() ){
            const Scope& scope = *it;
            ++it;

            if( scope.contains(name) )
                return true;
        }

        return false;
    }

    QString apply( const QString& name ) const
    {
        StaticChain::ConstIterator it = staticChain.begin();
        while( it != staticChain.end() ){
            const Scope& scope = *it;
            ++it;

            if( scope.contains(name) )
                return scope[ name ];
        }

        return QString();
    }

};

Lexer::Lexer( Driver* driver )
  : d( new LexerData),
    m_driver( driver ),
    m_recordComments( false ),
    m_skipWordsEnabled( true ),
    m_preprocessorEnabled( true )
{
  reset();
  d->beginScope();
}

Lexer::~Lexer()
{
    d->endScope();
    delete( d );
}

void Lexer::setSource( const QString& source,
		       PositionFilename const& p_filename)
{
  reset();
  m_source.set_source( source, p_filename);
  m_inPreproc = false;

  tokenize();
}

void Lexer::reset()
{
  m_tokens.clear();
  m_source.reset();
  m_preprocessor.reset();
}

void Lexer::Preprocessor::reset() {
  m_skipping.clear();
  m_trueTest.clear();
}

// ### should all be done with a "long" type IMO
int Lexer::toInt( const Token& token )
{
    QString s = token.text();
    if( token.type() == Token_number_literal ){
        // hex literal ?
	if( s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
	    return s.mid( 2 ).toInt( 0, 16 );
        QString n;
        int i = 0;
        while( i < int(s.length()) && s[i].isDigit() )
            n += s[i++];
        // ### respect more prefixes and suffixes ?
        return n.toInt();
    } else if( token.type() == Token_char_literal ){
	int i = s[0] == 'L' ? 2 : 1; // wide char ?
	if( s[i] == '\\' ){
	    // escaped char
	    int c = s[i+1].unicode();
	    switch( c ) {
	    case '0':
		return 0;
	    case 'r':
		return '\r';
	    case 'n':
		return '\n';
	    // ### more
	    default:
		return c;
	    }
	} else {
	    return s[i].unicode();
	}
    } else {
	return 0;
    }
}

Position const& Lexer::getTokenPosition( const Token& token) const
{
  return token.getStartPosition();
}

void Lexer::nextToken( Token& tk)
{
  m_source.parse( gr_whiteSpaces);

  Position startPosition( currentPosition());

  QChar ch = m_source.currentChar();
  if( ch.isNull() || ch.isSpace() ){
    /* skip */
  } else if( m_source.get_startLine()
	     && m_source.parse( ch_p('#') >> gr_whiteSpaces).hit) {
    m_source.set_startLine( false);
    
    QString directive;
    m_source.parse( identifier_g[ assign(directive)]); // read the directive

    handleDirective( directive );
  } else if( m_source.get_startLine() && m_preprocessor.inSkip()) {
    // skip line and continue
    m_source.set_startLine( false);
    bool ppe = m_preprocessorEnabled;
    m_preprocessorEnabled = false;
    while( !m_source.currentChar().isNull()
	   && m_source.currentChar() != '\n'
	   && m_source.currentChar() != '\r') {
      Token tok;
      nextToken( tok);
    }
    m_source.set_startLine( true);
    m_preprocessorEnabled = ppe;
    return;
  } else if( m_source.parse
	     (
	      if_p(var( m_recordComments))
	      [ gr_lineComment | gr_multiLineComment
		[var( tk) = construct_<Token>(Token_comment, arg1, arg2)]
		]
	      .else_p[ gr_lineComment | gr_multiLineComment]
	      | charLiteral_g[assign(tk)]
	      |
	      gr_stringLiteral
	      [var(tk) = construct_<Token>( Token_string_literal, arg1, arg2)]
	      ).hit) {
  } else if( ch.isLetter() || ch == '_' ){
    CharIterator start = m_source.get_ptr();
    QString ide;
    m_source.parse( identifier_g[assign(ide)]);
    int k = Lookup::find( &keyword, ide );
    if( m_preprocessorEnabled && m_driver->hasMacro(ide) &&
	(k == -1 || !m_driver->macro(ide).body().isEmpty()) ){


      bool preproc = m_preprocessorEnabled;
      m_preprocessorEnabled = false;

      d->beginScope();

      Position svPosition = currentPosition();

      //	    Macro& m = m_driver->macro( ide );
      Macro m = m_driver->macro( ide );
      //m_driver->removeMacro( m.name() );

      QString ellipsisArg;

      if( m.hasArguments() ){
	CharIterator endIde = m_source.get_ptr();

	m_source.parse( gr_whiteSpaces);
	if( m_source.parse( ch_p('(')).hit) {
	  int argIdx = 0;
	  int argCount = m.argumentList().size();
	  while( !m_source.currentChar().isNull() && argIdx<argCount ){
	    m_source.parse( gr_whiteSpaces);

	    QString argName = m.argumentList()[ argIdx ];

	    bool ellipsis = argName == "...";

	    QString arg = readArgument();

	    if( !ellipsis )
	      d->bind( argName, arg );
	    else
	      ellipsisArg += arg;

	    if( m_source.parse( ch_p(',')).hit) {
	      if( !ellipsis ){
		++argIdx;
	      } else {
		ellipsisArg += ", ";
	      }
	    } else if( m_source.currentChar() == ')' ){
	      break;
	    }
	  }
	  if( m_source.parse(ch_p(')')).hit) {
	    // valid macro
	  }
	} else {
	  Position l_newPosition( svPosition);
	  l_newPosition.column += (endIde - start);
	  tk = m_source.createToken( Token_identifier, start, endIde);

	  m_source.set_startLine( false);

	  d->endScope();        // OPS!!
	  m_preprocessorEnabled = preproc;
	  return;
	}
      }

      Position argsEndAtPosition = currentPosition();

#if defined( KDEVELOP_BGPARSER )
      qthread_yield();
#endif
      m_source.insert( m.body());

      // tokenize the macro body

      QString textToInsert;

      while( !m_source.currentChar().isNull() ){

	m_source.parse( gr_whiteSpaces);

	Token tok;
	nextToken( tok);

	bool stringify = !m_inPreproc && tok == '#';
	bool merge = !m_inPreproc && tok == Token_concat;

	if( stringify || merge )
	  nextToken( tok);

	if( tok == Token_eof )
	  break;

	QString tokText = tok.text();
	QString str = (tok == Token_identifier && d->hasBind(tokText)) ? d->apply( tokText ) : tokText;
	if( str == ide ){
	  //Problem p( i18n("unsafe use of macro '%1'").arg(ide), m_currentLine, m_currentColumn );
	  //m_driver->addProblem( m_driver->currentFileName(), p );
	  m_driver->removeMacro( ide );
	  // str = QString();
	}

	if( stringify ) {
	  textToInsert.append( QString::fromLatin1("\"") + str + QString::fromLatin1("\" ") );
	} else if( merge ){
	  textToInsert.truncate( textToInsert.length() - 1 );
	  textToInsert.append( str );
	} else if( tok == Token_ellipsis && d->hasBind("...") ){
	  textToInsert.append( ellipsisArg );
	} else {
	  textToInsert.append( str + QString::fromLatin1(" ") );
	}
      }

#if defined( KDEVELOP_BGPARSER )
      qthread_yield();
#endif
      m_source.insert( textToInsert);

      d->endScope();
      m_preprocessorEnabled = preproc;
      //m_driver->addMacro( m );
      m_source.set_currentPosition( argsEndAtPosition);
    } else if( k != -1 ){
      tk = m_source.createToken( k, start);
    } else if( m_skipWordsEnabled ){
      QMap< QString, QPair<SkipType, QString> >::Iterator pos = m_words.find( ide );
      if( pos != m_words.end() ){
	if( (*pos).first == SkipWordAndArguments ){
	  m_source.parse( gr_whiteSpaces);
	  if( m_source.currentChar() == '(' )
	    skip( '(', ')' );
	}
	if( !(*pos).second.isEmpty() ){
#if defined( KDEVELOP_BGPARSER )
	  qthread_yield();
#endif
	  m_source.insert( QString(" ") + (*pos).second + QString(" "));
	}
      } else if( /*qt_rx.exactMatch(ide) ||*/
		ide.endsWith("EXPORT") ||
		(ide.startsWith("Q_EXPORT") && ide != "Q_EXPORT_INTERFACE") ||
		ide.startsWith("QM_EXPORT") ||
		ide.startsWith("QM_TEMPLATE")){

	m_source.parse( gr_whiteSpaces);
	if( m_source.currentChar() == '(' )
	  skip( '(', ')' );
      } else if( ide.startsWith("K_TYPELIST_") || ide.startsWith("TYPELIST_") ){
	tk = m_source.createToken( Token_identifier, start);
	m_source.parse( gr_whiteSpaces);
	if( m_source.currentChar() == '(' )
	  skip( '(', ')' );
      } else{
	tk = m_source.createToken( Token_identifier, start);
      }
    } else {
      tk = m_source.createToken( Token_identifier, start);
    }
  } else if( m_source.parse( numberLiteral_g[assign(tk)]
			     | operator_g[ assign(tk)]
			     ).hit) {
  } else {
    CharIterator l_ptr = m_source.get_ptr();
    m_source.nextChar();
    tk = m_source.createToken( ch.unicode(), l_ptr);
  }
  if( m_source.parse( eol_p).hit)
    m_source.set_startLine( true);
  else
    m_source.set_startLine( false);
}

void Lexer::tokenize()
{
  m_source.set_startLine( true);
    for( ;; ) {
	Token tk;
	nextToken( tk );

        if( tk.type() != -1 )
      m_tokens.push_back( tk);

    if( m_source.currentChar().isNull() )
	    break;
    }

  Token tk = m_source.createToken( Token_eof, m_source.get_ptr());
  m_tokens.push_back( tk);
}

void Lexer::addSkipWord( const QString& word, SkipType skipType, const QString& str )
{
    m_words[ word ] = qMakePair( skipType, str );
}

void Lexer::skip( int l, int r )
{
  Position svCurrentPosition = currentPosition();

    int count = 0;

  while( !m_source.eof() ){
	Token tk;
	nextToken( tk );

	if( (int)tk == l )
            ++count;
        else if( (int)tk == r )
            --count;

        if( count == 0 )
            break;
    }

  m_source.set_currentPosition( svCurrentPosition);
}

QString Lexer::readArgument()
{
    int count = 0;

    QString arg;

  m_source.parse( gr_whiteSpaces);
  while( !m_source.currentChar().isNull() ){

    m_source.parse( gr_whiteSpaces);
    QChar ch = m_source.currentChar();

	if( ch.isNull() || (!count && (ch == ',' || ch == ')')) )
	    break;

	Token tk;
	nextToken( tk );

	if( tk == '(' ){
	    ++count;
	} else if( tk == ')' ){
	    --count;
	}

	if( tk != -1 )
            arg += tk.text() + ' ';
    }

    return arg.trimmed();
}

void Lexer::handleDirective( const QString& directive )
{
  m_inPreproc = true;

  bool skip = m_skipWordsEnabled;
  bool preproc = m_preprocessorEnabled;

  m_skipWordsEnabled = false;
  m_preprocessorEnabled = false;

  if( directive == "define" ){
    if( !m_preprocessor.inSkip()) {
      Macro m;
      processDefine( m );
    }
  } else if( directive == "else" ){
    if( !m_preprocessor.empty())
      m_preprocessor.processElse();
  } else if( directive == "elif" ){
    if( !m_preprocessor.empty())
      m_preprocessor.processElif( macroExpression());
  } else if( directive == "endif" ){
    if( !m_preprocessor.empty())
      m_preprocessor.decrement();
  } else if( directive == "if") {
    m_preprocessor.processIf( macroExpression());
  } else if( directive == "ifdef") {
    m_preprocessor.processIf( macroDefined());
  } else if( directive == "ifndef" ){
    m_preprocessor.processIf( !macroDefined());
  } else if( directive == "include" ){
    if( !m_preprocessor.inSkip())
      processInclude();
  } else if( directive == "undef" ){
    if( !m_preprocessor.inSkip())
      processUndef();
  }

  // skip line
  while( !m_source.currentChar().isNull()
	 && m_source.currentChar() != '\n'
	 && m_source.currentChar() != '\r') {
    Token tk;
    nextToken( tk);
  }

  m_skipWordsEnabled = skip;
  m_preprocessorEnabled = preproc;

  m_inPreproc = false;
}

int Lexer::macroDefined()
{
  QString word;
  m_source.parse( gr_whiteSpaces >> identifier_g[assign(word)]);
  return m_driver->hasMacro( word );
}

void Lexer::processDefine( Macro& m )
{
  m.setFileName( m_driver->currentFileName() );
  m_source.parse( gr_whiteSpaces);

  QString macroName;
  m_source.parse( identifier_g[assign(macroName)]);
  m_driver->removeMacro( macroName );
  m.setName( macroName );

  if( m_source.parse( ch_p('(') >> gr_whiteSpaces).hit) {
    m.setHasArguments( true );
    while( !m_source.currentChar().isNull() && m_source.currentChar() != ')' ){
      m_source.parse( gr_whiteSpaces);

      QString arg;
      m_source.parse( str_p("...")[var(arg) = constructQString( arg1, arg2)]
		      | identifier_g[assign(arg)]
		      );
      m.addArgument( Macro::Argument(arg) );

      m_source.parse( gr_whiteSpaces);
      if( !m_source.parse( ch_p(',')).hit)
	break;
    }
    bool l_status = m_source.parse(ch_p(')')).hit;
    assert( l_status);
  }

  m_preprocessorEnabled = true;

  QString body;
  while( !m_source.currentChar().isNull()
	 && m_source.currentChar() != '\n'
	 && m_source.currentChar() != '\r' ){

    if( m_source.currentChar().isSpace() ){
      m_source.parse( gr_whiteSpaces);
      body += ' ';
    } else {

      Token tk;
      nextToken( tk);

      if( tk.type() != -1 ){
	QString s = tk.text();
	body += s;
      }
    }
  }

  m.setBody( body );
  m_driver->addMacro( m );
}

typedef std::pair<QString, int> Dependency;

struct DependencyClosure
  : boost::spirit::closure<DependencyClosure, QString, int>
{
  member1 m_word;
  member2 m_scope;
};

struct header :
  grammar<header, result_closure<Dependency>::context_t>
{
  template < typename ScannerT >
  struct definition {
    typedef rule<ScannerT, DependencyClosure::context_t> rule_t;
    rule_t main;

    rule_t const& start() const {return main;}

    definition( header const& self) {
      main =
	( confix_p( ch_p('"') [main.m_scope = (int)Dep_Local],
		    (*anychar_p) [main.m_word = constructQString( arg1, arg2)],
		    '"')
	  |
	  confix_p( ch_p('<') [main.m_scope = (int)Dep_Global],
		    (*anychar_p) [main.m_word = constructQString( arg1, arg2)],
		    '>')
	  )
	[self.result_ = construct_<Dependency>( main.m_word, main.m_scope)]
	;
    }
  };
} header_g;

void Lexer::processInclude() {
  if( !m_preprocessor.inSkip())
    m_source.parse( gr_whiteSpaces >>
		    header_g
		    [boost::bind( &Lexer::addDependence, this, _1)]
		    );
}

void Lexer::processUndef()
{
  m_source.parse( gr_whiteSpaces);
  QString word;
  m_source.parse( identifier_g[assign(word)]);
  m_driver->removeMacro( word );
}

int Lexer::macroPrimary()
{
  m_source.parse( gr_whiteSpaces);
  int result = 0;
  if( m_source.parse( ch_p('(')).hit) {
    result = macroExpression();
    bool l_hit = m_source.parse( ch_p(')')).hit;
    /** \todo hit must be true */
    if( !l_hit)
      result = 0;
  } else {
#warning use locals
    boost::function<int (int)> l_op = _Identity<int>();
    if( m_source.parse( ch_p('+')
			| ch_p('-')[var(l_op) = std::negate<int>()]
			| ch_p('!')[var(l_op) = std::logical_not<int>()]
			| ch_p('~')[var(l_op) = tilde<int>()]
			).hit) {
      result = l_op( macroPrimary());
    } else if( m_source.parse( str_p("defined")).hit) {
      /** \todo Strange : should result in an identifier (after
	  preprocessor variable substitution) ! */
      result = macroPrimary();
    } else {
      m_source.parse(
		     identifier_g[assignFunctorResult<1>
				  ( result,
				    boost::bind( &Driver::hasMacro, m_driver,
						 _1))]
		     | numberLiteral_g[assignFunctorResult<1>
				       ( result,
					 boost::bind( &Lexer::toInt, _1))]
		     | charLiteral_g[assignFunctorResult<1>
				     ( result,
				       boost::bind( &Lexer::toInt, _1))]
		     );
    }
  }
  return result;
}

int Lexer::macroMultiplyDivide()
{
  int result = macroPrimary();
  int iresult, op;
  for (;;) {
    m_source.parse( gr_whiteSpaces);
    if( m_source.parse(
		       ch_p('*')[var(op) = 0]
		       |
		       (ch_p('/') >> eps_p( anychar_p - '*' - '/'))
		       [var(op) = 1]
		       | ch_p('%')[var(op) = 2]
		       ).hit) {
      iresult = macroPrimary();
      switch( op) {
      case 0:
	result = (result * iresult);
	break;
      case 1:
	result = (iresult == 0 ? 0 : (result / iresult));
	break;
      case 2:
	result = (iresult == 0) ? 0 : (result % iresult);
      }
    } else
      break;
  }
  return result;
}

int Lexer::macroAddSubtract() {
  int result = macroMultiplyDivide();
  int iresult;
  bool ad = false;
  m_source.parse( gr_whiteSpaces);
  while( m_source.parse(
			ch_p('+')[var(ad) = true] | ch_p('-')[var(ad) = false]
			).hit
	 ) {
    iresult = macroMultiplyDivide();
    result = ad ? (result+iresult) : (result-iresult);
  }
  return result;
}

int Lexer::macroRelational() {
  int result = macroAddSubtract();
  m_source.parse( gr_whiteSpaces);
  boost::function<bool (int, int)> l_op;
  while( m_source.parse(
			str_p("<=")[var(l_op) = less_equal<int>()]
			| ch_p('<')[var(l_op) = less<int>()]
			| str_p(">=")[var(l_op) = greater_equal<int>()]
			| ch_p('>')[var(l_op) = greater<int>()]
			).hit
	 ) {
    int iresult = macroAddSubtract();
    result = l_op( result, iresult);
  }
  return result;
}

int Lexer::macroEquality()
{
  int result = macroRelational();
  m_source.parse( gr_whiteSpaces);
  boost::function<bool( int, int)> l_op;
  while( m_source.parse( str_p("==")[var(l_op) = equal_to<int>()]
			 | str_p("!=")[var(l_op) = not_equal_to<int>()]
			 ).hit) {
    result = l_op( result, macroRelational());
  }
  return result;
}

int Lexer::macroBoolAnd()
{
  int result = macroEquality();
  m_source.parse( gr_whiteSpaces);
  while( m_source.parse( ch_p('&') >> eps_p( anychar_p - '&')).hit)
    result &= macroEquality();
  return result;
}

int Lexer::macroBoolXor() {
  int result = macroBoolAnd();
  m_source.parse( gr_whiteSpaces);
  while( m_source.parse(ch_p('^')).hit)
    result ^= macroBoolAnd();
  return result;
}

int Lexer::macroBoolOr()
{
  int result = macroBoolXor();
  m_source.parse( gr_whiteSpaces);
  while( m_source.parse( ch_p('|') >> eps_p( anychar_p - '|')).hit)
    result |= macroBoolXor();
  return result;
}

int Lexer::macroLogicalAnd()
{
  int result = macroBoolOr();
  m_source.parse( gr_whiteSpaces);
  while( m_source.parse( str_p("&&")).hit)
    result = macroBoolOr() && result;
  return result;
}

int Lexer::macroLogicalOr()
{
  int result = macroLogicalAnd();
  m_source.parse( gr_whiteSpaces);
  while( m_source.parse( str_p("||")).hit)
    result = macroLogicalAnd() || result;
  return result;
}

int Lexer::macroExpression()
{
  m_source.parse( gr_whiteSpaces);
  return macroLogicalOr();
}

// *IMPORTANT*
// please, don't include lexer.moc here, because Lexer isn't a QObject class!!
// if you have problem while recompiling try to remove cppsupport/.deps,
// cppsupport/Makefile.in and rerun automake/autoconf

