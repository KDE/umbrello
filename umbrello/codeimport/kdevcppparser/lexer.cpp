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

Token::Token()
  : m_type( -1 ),
    m_position( 0 ),
    m_length( 0 ),
    m_text( 0 )
{
}

Token::Token( int type, int position, int length, Position const& start,
	      Position const& end, const QString& text)
  : m_type( type ),
    m_position( position ),
    m_length( length ),
    m_start( start),
    m_end( end),
    m_text( text )
{}

Token Lexer::Source::createToken( int type, int start, int len,
				  Position const& startPosition,
				  Position const& endPosition) const
{return Token( type, start, len, startPosition, endPosition, get_source());}

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
      m_preprocessorEnabled( true ),
      m_reportMessages( false )
{
    reset();
    d->beginScope();
}

Lexer::~Lexer()
{
    d->endScope();
    delete( d );
}

void Lexer::setSource( const QString& source )
{
    reset();
  m_source.set_source( source);
    m_inPreproc = false;

    tokenize();
}

void Lexer::reset()
{
    m_tokens.clear();
  m_source.reset();
    m_ifLevel = 0;
    m_skipping.resize( 200 );
    m_skipping.fill( 0 );
    m_trueTest.resize( 200 );
    m_trueTest.fill( 0 );
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

int Lexer::Source::findOperator3() const
{
  int n = length();

  if( n >= 3){
    QChar ch = currentChar(), ch1 = peekChar(), ch2 = peekChar(2);

    if( ch == '<' && ch1 == '<' && ch2 == '=' ) return Token_assign;
    else if( ch == '>' && ch1 == '>' && ch2 == '=' ) return Token_assign;
    else if( ch == '-' && ch1 == '>' && ch2 == '*' ) return Token_ptrmem;
    else if( ch == '.' && ch1 == '.' && ch2 == '.' ) return Token_ellipsis;
  }

  return -1;
}

Position const& Lexer::getTokenPosition( const Token& token) const
{
  return token.getStartPosition();
}

void Lexer::nextToken( Token& tk, bool stopOnNewline )
{
    int op = 0;

  m_source.readWhiteSpaces( !stopOnNewline, m_inPreproc);

  Position startPosition( currentPosition());

  QChar ch = m_source.currentChar();
  QChar ch1 = m_source.peekChar();

    if( ch.isNull() || ch.isSpace() ){
	/* skip */
  } else if( m_source.get_startLine() && ch == '#' ){

    m_source.nextChar(); // skip #
    m_source.readWhiteSpaces( false, m_inPreproc); // skip white spaces
    m_source.set_startLine( false);

    QString directive = m_source.readIdentifier(); // read the directive

	handleDirective( directive );
  } else if( m_source.get_startLine() && m_skipping[ m_ifLevel ] ){
	// skip line and continue
    m_source.set_startLine( false);
    bool ppe = m_preprocessorEnabled;
    m_preprocessorEnabled = false;
    while( !m_source.currentChar().isNull()
	   && m_source.currentChar() != '\n' ){
            Token tok;
            nextToken( tok, true );
        }
    m_source.set_startLine( true);
    m_preprocessorEnabled = ppe;
        return;
    } else if( ch == '/' && ch1 == '/' ){
    int start = m_source.get_ptr();
	readLineComment();
	if( recordComments() ){
      tk = m_source.createToken( Token_comment, start, startPosition);
	}
    } else if( ch == '/' && ch1 == '*' ){
    int start = m_source.get_ptr();
    m_source.nextChar( 2 );
	readMultiLineComment();

	if( recordComments() ){
      tk = m_source.createToken( Token_comment, start, startPosition);
	}
    } else if( ch == '\'' || (ch == 'L' && ch1 == '\'') ){
    int start = m_source.get_ptr();
    m_source.readCharLiteral();
    tk = m_source.createToken( Token_char_literal, start, startPosition);
    } else if( ch == '"' ){
    int start = m_source.get_ptr();
    m_source.readStringLiteral();
    tk = m_source.createToken( Token_string_literal, start, startPosition);
    } else if( ch.isLetter() || ch == '_' ){
    int start = m_source.get_ptr();
    QString ide = m_source.readIdentifier();
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
	int endIde = m_source.get_ptr();

	m_source.readWhiteSpaces( true, m_inPreproc);
	if( m_source.currentChar() == '(' ){
	  m_source.nextChar();
		    int argIdx = 0;
		    int argCount = m.argumentList().size();
	  while( !m_source.currentChar().isNull() && argIdx<argCount ){
	    m_source.readWhiteSpaces( true, m_inPreproc);

                        QString argName = m.argumentList()[ argIdx ];

                        bool ellipsis = argName == "...";

			QString arg = readArgument();

                        if( !ellipsis )
                            d->bind( argName, arg );
                        else
                            ellipsisArg += arg;

	    if( m_source.currentChar() == ',' ){
	      m_source.nextChar();
                            if( !ellipsis ){
                                ++argIdx;
                            } else {
                                ellipsisArg += ", ";
                            }
	    } else if( m_source.currentChar() == ')' ){
			    break;
			}
		    }
	  if( m_source.currentChar() == ')' ){
                        // valid macro
	    m_source.nextChar();
                    }
		} else {
	  Position l_newPosition( svPosition);
	  l_newPosition.column += (endIde - start);
	  tk = m_source.createToken( Token_identifier, start, endIde - start,
				     svPosition, l_newPosition );

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

	m_source.readWhiteSpaces( true, m_inPreproc);

                Token tok;
                nextToken( tok );

                bool stringify = !m_inPreproc && tok == '#';
                bool merge = !m_inPreproc && tok == Token_concat;

                if( stringify || merge )
                    nextToken( tok );

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
      tk = m_source.createToken( k, start, startPosition);
	} else if( m_skipWordsEnabled ){
	    QMap< QString, QPair<SkipType, QString> >::Iterator pos = m_words.find( ide );
	    if( pos != m_words.end() ){
		if( (*pos).first == SkipWordAndArguments ){
	  m_source.readWhiteSpaces( true, m_inPreproc);
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

	m_source.readWhiteSpaces( true, m_inPreproc);
	if( m_source.currentChar() == '(' )
		    skip( '(', ')' );
	    } else if( ide.startsWith("K_TYPELIST_") || ide.startsWith("TYPELIST_") ){
	tk = m_source.createToken( Token_identifier, start, startPosition);
	m_source.readWhiteSpaces( true, m_inPreproc);
	if( m_source.currentChar() == '(' )
		    skip( '(', ')' );
	    } else{
	tk = m_source.createToken( Token_identifier, start, startPosition);
	    }
	} else {
      tk = m_source.createToken( Token_identifier, start, startPosition);
	}
    } else if( ch.isNumber() ){
    int start = m_source.get_ptr();
    m_source.readNumberLiteral();
    tk = m_source.createToken( Token_number_literal, start, startPosition);
  } else if( -1 != (op = m_source.findOperator3()) ){
    int l_ptr = m_source.get_ptr();
    m_source.nextChar( 3 );
    tk = m_source.createToken( op, l_ptr, startPosition);
  } else if( -1 != (op = m_source.findOperator2()) ){
    int l_ptr = m_source.get_ptr();
    m_source.nextChar( 2 );
    tk = m_source.createToken( op, l_ptr, startPosition);
    } else {
    int l_ptr = m_source.get_ptr();
    m_source.nextChar();
    tk = m_source.createToken( ch.unicode(), l_ptr, startPosition);
    }

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

  Token tk = m_source.createToken( Token_eof, m_source.get_ptr(),
				   currentPosition());
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

  m_source.readWhiteSpaces( true, m_inPreproc);
  while( !m_source.currentChar().isNull() ){

    m_source.readWhiteSpaces( true, m_inPreproc);
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
	if( !m_skipping[ m_ifLevel ] ){
	    Macro m;
	    processDefine( m );
	}
    } else if( directive == "else" ){
        processElse();
    } else if( directive == "elif" ){
        processElif();
    } else if( directive == "endif" ){
        processEndif();
    } else if( directive == "if" ){
        processIf();
    } else if( directive == "ifdef" ){
        processIfdef();
    } else if( directive == "ifndef" ){
        processIfndef();
    } else if( directive == "include" ){
	if( !m_skipping[ m_ifLevel ] ){
            processInclude();
        }
    } else if( directive == "undef" ){
	if( !m_skipping[ m_ifLevel ] ){
            processUndef();
        }
    }

    // skip line
  while( !m_source.currentChar().isNull() && m_source.currentChar() != '\n' ){
        Token tk;
        nextToken( tk, true );
    }

  m_skipWordsEnabled = skip;
  m_preprocessorEnabled = preproc;

    m_inPreproc = false;
}

int Lexer::testIfLevel()
{
    int rtn = !m_skipping[ m_ifLevel++ ];
    m_skipping[ m_ifLevel ] = m_skipping[ m_ifLevel - 1 ];
    return rtn;
}

int Lexer::macroDefined()
{
  m_source.readWhiteSpaces( false, m_inPreproc);
  QString word = m_source.readIdentifier();
    bool r = m_driver->hasMacro( word );

    return r;
}

void Lexer::processDefine( Macro& m )
{
    m.setFileName( m_driver->currentFileName() );
  m_source.readWhiteSpaces( false, m_inPreproc);

  QString macroName = m_source.readIdentifier();
    m_driver->removeMacro( macroName );
    m.setName( macroName );

  if( m_source.currentChar() == '(' ){
	m.setHasArguments( true );
    m_source.nextChar();

    m_source.readWhiteSpaces( false, m_inPreproc);

    while( !m_source.currentChar().isNull() && m_source.currentChar() != ')' ){
      m_source.readWhiteSpaces( false, m_inPreproc);

      QString arg;
      if( m_source.currentChar() == '.' && m_source.peekChar() == '.' && m_source.peekChar(2) == '.' ) {
	int startArg = m_source.get_ptr();
	m_source.nextChar( 3 );
	arg = m_source.substrFrom( startArg);
      } else {
	arg = m_source.readIdentifier();
      }

	    m.addArgument( Macro::Argument(arg) );

      m_source.readWhiteSpaces( false, m_inPreproc);
      if( m_source.currentChar() != ',' )
		break;

      m_source.nextChar(); // skip ','
	}

    if( m_source.currentChar() == ')' )
      m_source.nextChar(); // skip ')'
    }

  m_preprocessorEnabled = true;

    QString body;
  while( !m_source.currentChar().isNull() && m_source.currentChar() != '\n' ){

    if( m_source.currentChar().isSpace() ){
      m_source.readWhiteSpaces( false, m_inPreproc);
	    body += ' ';
	} else {

	    Token tk;
	    nextToken( tk, true );

	    if( tk.type() != -1 ){
                QString s = tk.text();
		body += s;
	    }
	}
    }

    m.setBody( body );
    m_driver->addMacro( m );
}

void Lexer::processElse()
{
    if( m_ifLevel == 0 )
        /// @todo report error
	return;

    if( m_ifLevel > 0 && m_skipping[m_ifLevel-1] )
       m_skipping[ m_ifLevel ] = m_skipping[ m_ifLevel - 1 ];
    else
       m_skipping[ m_ifLevel ] = m_trueTest[ m_ifLevel ];
}

void Lexer::processElif()
{
    if( m_ifLevel == 0 )
	/// @todo report error
	return;

    if( !m_trueTest[m_ifLevel] ){
        /// @todo implement the correct semantic for elif!!
        bool inSkip = m_ifLevel > 0 && m_skipping[ m_ifLevel-1 ];
        m_trueTest[ m_ifLevel ] = macroExpression() != 0;
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
    else
	m_skipping[ m_ifLevel ] = true;
}

void Lexer::processEndif()
{
    if( m_ifLevel == 0 )
	/// @todo report error
	return;

    m_skipping[ m_ifLevel ] = 0;
    m_trueTest[ m_ifLevel-- ] = 0;
}

void Lexer::processIf()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if( testIfLevel() ) {
#if 0
	int n;
	if( (n = testDefined()) != 0 ) {
	    int isdef = macroDefined();
	    m_trueTest[ m_ifLevel ] = (n == 1 && isdef) || (n == -1 && !isdef);
	} else
#endif
        m_trueTest[ m_ifLevel ] = macroExpression() != 0;
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processIfdef()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if( testIfLevel() ){
	m_trueTest[ m_ifLevel ] = macroDefined();
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processIfndef()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if( testIfLevel() ){
	m_trueTest[ m_ifLevel ] = !macroDefined();
	m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processInclude()
{
    if( m_skipping[m_ifLevel] )
	return;

  m_source.readWhiteSpaces( false, m_inPreproc);
  if( !m_source.currentChar().isNull() ){
    QChar ch = m_source.currentChar();
	if( ch == '"' || ch == '<' ){
      m_source.nextChar();
	    QChar ch2 = ch == QChar('"') ? QChar('"') : QChar('>');

      int startWord = m_source.get_ptr();
      while( !m_source.currentChar().isNull() && m_source.currentChar() != ch2 )
	m_source.nextChar();
      if( !m_source.currentChar().isNull() ){
	QString word = m_source.substrFrom( startWord);
		m_driver->addDependence( m_driver->currentFileName(),
					 Dependence(word, ch == '"' ? Dep_Local : Dep_Global) );
	m_source.nextChar();
	    }
	}
    }
}

void Lexer::processUndef()
{
  m_source.readWhiteSpaces( true, m_inPreproc);
  QString word = m_source.readIdentifier();
    m_driver->removeMacro( word );
}

int Lexer::macroPrimary()
{
  m_source.readWhiteSpaces( false, m_inPreproc);
    int result = 0;
  switch( m_source.currentChar().unicode() ) {
    case '(':
    m_source.nextChar();
	result = macroExpression();
    if( m_source.currentChar() != ')' ){
	    /// @todo report error
	    return 0;
	}
    m_source.nextChar();
	return result;

    case '+':
    case '-':
    case '!':
    case '~':
	{
      QChar tk = m_source.currentChar();
      m_source.nextChar();
	    int result = macroPrimary();
	    if( tk == '-' ) return -result;
	    else if( tk == '!' ) return !result;
	    else if( tk == '~' ) return ~result;
	}
	break;

    default:
	{
	    Token tk;
	    nextToken( tk, false );
	    switch( tk.type() ){
	    case Token_identifier:
		if( tk.text() == "defined" ){
		    return macroPrimary();
		}
		/// @todo implement
		return m_driver->hasMacro( tk.text() );
	    case Token_number_literal:
	    case Token_char_literal:
		return toInt( tk );
            default:
		break;
	    } // end switch

	} // end default

    } // end switch

    return 0;
}

int Lexer::macroMultiplyDivide()
{
    int result = macroPrimary();
    int iresult, op;
    for (;;)    {
    m_source.readWhiteSpaces( false, m_inPreproc);
    if( m_source.currentChar() == '*' )
            op = 0;
    else if( m_source.currentChar() == '/' && !(m_source.peekChar() == '*' || m_source.peekChar() == '/') )
            op = 1;
    else if( m_source.currentChar() == '%' )
            op = 2;
        else
            break;
    m_source.nextChar();
        iresult = macroPrimary();
        result = op == 0 ? (result * iresult) :
                 op == 1 ? (iresult == 0 ? 0 : (result / iresult)) :
                           (iresult == 0 ? 0 : (result % iresult)) ;
    }
    return result;
}

int Lexer::macroAddSubtract()
{
    int result = macroMultiplyDivide();
    int iresult, ad;
  m_source.readWhiteSpaces( false, m_inPreproc);
  while( m_source.currentChar() == '+' || m_source.currentChar() == '-')    {
    ad = m_source.currentChar() == '+';
    m_source.nextChar();
        iresult = macroMultiplyDivide();
        result = ad ? (result+iresult) : (result-iresult);
    }
    return result;
}

int Lexer::macroRelational()
{
    int result = macroAddSubtract();
    int iresult;
  m_source.readWhiteSpaces( false, m_inPreproc);
  while( m_source.currentChar() == '<' || m_source.currentChar() == '>') {
    int lt = m_source.currentChar() == '<';
    m_source.nextChar();
    if( m_source.currentChar() == '=') {
      m_source.nextChar();

	    iresult = macroAddSubtract();
	    result = lt ? (result <= iresult) : (result >= iresult);
	}
	else {
	    iresult = macroAddSubtract();
	    result = lt ? (result < iresult) : (result > iresult);
	}
    }

    return result;
}

int Lexer::macroEquality()
{
    int result = macroRelational();
    int iresult, eq;
  m_source.readWhiteSpaces( false, m_inPreproc);
  while ((m_source.currentChar() == '=' || m_source.currentChar() == '!') && m_source.peekChar() == '=')  {
    eq = m_source.currentChar() == '=';
    m_source.nextChar( 2 );
	iresult = macroRelational();
	result = eq ? (result==iresult) : (result!=iresult);
    }
    return result;
}

int Lexer::macroBoolAnd()
{
    int result = macroEquality();
  m_source.readWhiteSpaces( false, m_inPreproc);
  while( m_source.currentChar() == '&' && m_source.peekChar() != '&')    {
    m_source.nextChar();
	result &= macroEquality();
    }
    return result;
}

int Lexer::macroBoolXor()
{
    int result = macroBoolAnd();
  m_source.readWhiteSpaces( false, m_inPreproc);
  while( m_source.currentChar() == '^')    {
    m_source.nextChar();
	result ^= macroBoolAnd();
    }
    return result;
}

int Lexer::macroBoolOr()
{
    int result = macroBoolXor();
  m_source.readWhiteSpaces( false, m_inPreproc);
  while( m_source.currentChar() == '|' && m_source.peekChar() != '|')    {
    m_source.nextChar();
	result |= macroBoolXor();
    }
    return result;
}

int Lexer::macroLogicalAnd()
{
    int result = macroBoolOr();
  m_source.readWhiteSpaces( false, m_inPreproc);
  while( m_source.currentChar() == '&' && m_source.peekChar() == '&')    {
    m_source.nextChar( 2 );
        result = macroBoolOr() && result;
    }
    return result;
}

int Lexer::macroLogicalOr()
{
    int result = macroLogicalAnd();
  m_source.readWhiteSpaces( false, m_inPreproc);
  while( m_source.currentChar() == '|' && m_source.peekChar() == '|')    {
    m_source.nextChar( 2 );
        result = macroLogicalAnd() || result;
    }
    return result;
}

int Lexer::macroExpression()
{
  m_source.readWhiteSpaces( false, m_inPreproc);
    return macroLogicalOr();
}

// *IMPORTANT*
// please, don't include lexer.moc here, because Lexer isn't a QObject class!!
// if you have problem while recompiling try to remove cppsupport/.deps,
// cppsupport/Makefile.in and rerun automake/autoconf

