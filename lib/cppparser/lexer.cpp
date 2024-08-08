/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lexer.h"
#include "lookup.h"
#define DBG_SRC QLatin1String("Lexer")
#include "debug_utils.h"

#include <KLocalizedString>

#include <QRegExp>
#include <QMap>
#include <QList>

DEBUG_REGISTER_DISABLED(Lexer)

#if defined(KDEVELOP_BGPARSER)
#include <QThread>

class KDevTread: public QThread
{
public:
    static void yield()
    {
        msleep(0);
    }
};

inline void qthread_yield()
{
    KDevTread::yield();
}

#endif

#define CREATE_TOKEN(type, start, len) Token((type), (start), (len), m_source)
#define ADD_TOKEN(tk) m_tokens.insert(m_size++, new Token(tk));

using namespace std;

Token::Token(const QString & text)
    : m_type(-1),
      m_position(0),
      m_length(0),
      m_startLine(0),
      m_startColumn(0),
      m_endLine(0),
      m_endColumn(0),
      m_text(text)
{
}

Token::Token(int type, int position, int length, const QString& text)
    : m_type(type),
      m_position(position),
      m_length(length),
      m_startLine(0),
      m_startColumn(0),
      m_endLine(0),
      m_endColumn(0),
      m_text(text)
{
    DEBUG() << type << position << length << text.mid(position, length);
}

Token::Token(const Token& source)
    : m_type(source.m_type),
      m_position(source.m_position),
      m_length(source.m_length),
      m_startLine(source.m_startLine),
      m_startColumn(source.m_startColumn),
      m_endLine(source.m_endLine),
      m_endColumn(source.m_endColumn),
      m_text(source.m_text)
{
}

Token& Token::operator = (const Token& source)
{
    m_type = source.m_type;
    m_position = source.m_position;
    m_length = source.m_length;
    m_startLine = source.m_startLine;
    m_startColumn = source.m_startColumn;
    m_endLine = source.m_endLine;
    m_endColumn = source.m_endColumn;
//    m_text = source.m_text;
    return (*this);
}

Token::operator int () const
{
    return m_type;
}

bool Token::operator == (const Token& token) const
{
    return m_type == token.m_type &&
           m_position == token.m_position &&
           m_length == token.m_length &&
           m_startLine == token.m_startLine &&
           m_startColumn == token.m_startColumn &&
           m_endLine == token.m_endLine &&
           m_endColumn == token.m_endColumn &&
           m_text == token.m_text;
}

bool Token::isNull() const
{
    return m_type == Token_eof || m_length == 0;
}

int Token::type() const
{
    return m_type;
}

void Token::setType(int type)
{
    m_type = type;
}

int Token::position() const
{
    return m_position;
}

QString Token::text() const
{
    return m_text.mid(m_position, m_length);
}

void Token::setStartPosition(int line, int column)
{
    m_startLine = line;
    m_startColumn = column;
}

void Token::setEndPosition(int line, int column)
{
    m_endLine = line;
    m_endColumn = column;
}

void Token::getStartPosition(int* line, int* column) const
{
    if (line) *line = m_startLine;
    if (column) *column = m_startColumn;
}

void Token::getEndPosition(int* line, int* column) const
{
    if (line) *line = m_endLine;
    if (column) *column = m_endColumn;
}

void Token::setPosition(int position)
{
    m_position = position;
}

unsigned int Token::length() const
{
    return m_length;
}

void Token::setLength(unsigned int length)
{
    m_length = length;
}


struct LexerData {
    typedef QMap<QString, QString> Scope;
    typedef QList<Scope> StaticChain;

    StaticChain staticChain;

    void beginScope()
    {
        Scope scope;
        staticChain.push_front(scope);
    }

    void endScope()
    {
        staticChain.pop_front();
    }

    void bind(const QString& name, const QString& value)
    {
        Q_ASSERT(staticChain.size() > 0);
        staticChain.front().insert(name, value);
    }

    bool hasBind(const QString& name) const
    {
        StaticChain::ConstIterator it = staticChain.begin();
        while (it != staticChain.end()) {
            const Scope& scope = *it;
            ++it;

            if (scope.contains(name))
                return true;
        }

        return false;
    }

    QString apply(const QString& name) const
    {
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

bool Lexer::recordComments() const
{
    return m_recordComments;
}

void Lexer::setRecordComments(bool record)
{
    m_recordComments = record;
}

bool Lexer::recordWhiteSpaces() const
{
    return m_recordWhiteSpaces;
}

void Lexer::setRecordWhiteSpaces(bool record)
{
    m_recordWhiteSpaces = record;
}

QString Lexer::source() const
{
    return m_source;
}

int Lexer::index() const
{
    return m_index;
}

void Lexer::setIndex(int index)
{
    m_index = index;
}

const Token& Lexer::nextToken()
{
    if (m_index < m_size)
        return *m_tokens[ m_index++ ];

    return *m_tokens[ m_index ];
}

const Token& Lexer::tokenAt(int n) const
{
    return *m_tokens[ qMin(n, m_size-1) ];
}

const Token& Lexer::lookAhead(int n) const
{
    Token &t = *m_tokens[ qMin(m_index + n, m_size-1) ];
    DEBUG() << t;
    return t;
}

int Lexer::tokenPosition(const Token& token) const
{
    return token.position();
}

void Lexer::nextChar()
{
    if (m_idx >= m_endIdx) {
        m_currentChar = QChar();
        return;
    }
    if (m_source[m_idx] == QLatin1Char('\n')) {
        ++m_currentLine;
        m_currentColumn = 0;
        m_startLine = true;
    } else {
        ++m_currentColumn;
    }
    ++m_idx;

    if (m_idx < m_endIdx)
        m_currentChar = m_source[m_idx];
    else
        m_currentChar = QChar();
}

void Lexer::nextChar(int n)
{
    if (m_idx + n >= m_endIdx) {
        m_idx = m_endIdx;
        m_currentChar = QChar();
        return;
    }
    m_currentColumn += n;
    m_idx += n;

    if (m_idx < m_endIdx)
        m_currentChar = m_source[m_idx];
    else
        m_currentChar = QChar();
}

void Lexer::readIdentifier()
{
    while (currentChar().isLetterOrNumber() || currentChar() == QLatin1Char('_'))
        nextChar();
}

/**
 * Return true on success, false on error (EOF encountered).
 * The return value does not indicate whether spaces were skipped or not.
 */
bool Lexer::readWhiteSpaces(bool skipNewLine, bool skipOnlyOnce)
{
    while (1) {
        QChar ch = currentChar();
        if (ch.isNull())
            return false;

        if (ch == QLatin1Char('\n') && !skipNewLine) {
            break;
        } else if (ch.isSpace()) {
            nextChar();
            if (currentChar().isNull())
                return false;
        } else if (m_inPreproc && currentChar() == QLatin1Char('\\')) {
            nextChar();
            if (currentChar().isNull())
                return false;
            if (!readWhiteSpaces(true, true))
                return false;
        } else {
            break;
        }
        if (skipOnlyOnce && ch == QLatin1Char('\n')) {
            skipNewLine = false;
        }
    }
    return true;
}

//little hack for better performance
static bool isTodo(const QString& txt, int position)
{
    if (txt.length() < position + 4) return false;
    return (txt[ position ] == QLatin1Char('t') || txt[ position ] == QLatin1Char('T'))
           && (txt[ position+1 ] == QLatin1Char('o') || txt[ position+1 ] == QLatin1Char('O'))
           && (txt[ position+2 ] == QLatin1Char('d') || txt[ position+2 ] == QLatin1Char('D'))
           && (txt[ position+3 ] == QLatin1Char('o') || txt[ position+3 ] == QLatin1Char('O'));
}

static bool isFixme(const QString& txt, int position)
{
    if (txt.length() < position + 5) return false;
    return (txt[ position ] == QLatin1Char('f') || txt[ position ] == QLatin1Char('F'))
           && (txt[ position+1 ] == QLatin1Char('i') || txt[ position+1 ] == QLatin1Char('I'))
           && (txt[ position+2 ] == QLatin1Char('x') || txt[ position+2 ] == QLatin1Char('X'))
           && (txt[ position+3 ] == QLatin1Char('m') || txt[ position+3 ] == QLatin1Char('M'))
           && (txt[ position+4 ] == QLatin1Char('e') || txt[ position+4 ] == QLatin1Char('E'));
}

void Lexer::readLineComment()
{
    while (!currentChar().isNull() && currentChar() != QLatin1Char('\n')) {
        if (currentPosition() < 0)
            break;
        if (m_reportMessages && isTodo(m_source, currentPosition())) {
            nextChar(4);
            QString msg;
            int line = m_currentLine;
            int col = m_currentColumn;

            while (!currentChar().isNull()) {
                if (currentChar() == QLatin1Char('*') && peekChar() == QLatin1Char('/'))
                    break;
                else if (currentChar() == QLatin1Char('\n'))
                    break;

                msg += currentChar();
                nextChar();
            }
            m_driver->addProblem(m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Todo));
        } else if (m_reportMessages && isFixme(m_source, currentPosition())) {
            nextChar(5);
            QString msg;
            int line = m_currentLine;
            int col = m_currentColumn;

            while (!currentChar().isNull()) {
                if (currentChar() == QLatin1Char('*') && peekChar() == QLatin1Char('/'))
                    break;
                else if (currentChar() == QLatin1Char('\n'))
                    break;

                msg += currentChar();
                nextChar();
            }
            m_driver->addProblem(m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Fixme));
        } else
            nextChar();
    }
}

void Lexer::readMultiLineComment()
{
    while (!currentChar().isNull()) {
        if (currentPosition() < 0)
            break;
        if (currentChar() == QLatin1Char('*') && peekChar() == QLatin1Char('/')) {
            nextChar(2);
            return;
        } else if (m_reportMessages && isTodo(m_source, currentPosition())) {
            nextChar(4);
            QString msg;
            int line = m_currentLine;
            int col = m_currentColumn;

            while (!currentChar().isNull()) {
                if (currentChar() == QLatin1Char('*') && peekChar() == QLatin1Char('/'))
                    break;
                else if (currentChar() == QLatin1Char('\n'))
                    break;
                msg += currentChar();
                nextChar();
            }
            m_driver->addProblem(m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Todo));
        } else if (m_reportMessages && isFixme(m_source, currentPosition())) {
            nextChar(5);
            QString msg;
            int line = m_currentLine;
            int col = m_currentColumn;

            while (!currentChar().isNull()) {
                if (currentChar() == QLatin1Char('*') && peekChar() == QLatin1Char('/'))
                    break;
                else if (currentChar() == QLatin1Char('\n'))
                    break;

                msg += currentChar();
                nextChar();
            }
            m_driver->addProblem(m_driver->currentFileName(), Problem(msg, line, col, Problem::Level_Fixme));
        } else
            nextChar();
    }
}

void Lexer::readCharLiteral()
{
    if (currentChar() == QLatin1Char('\''))
        nextChar(); // skip '
    else if (currentChar() == QLatin1Char('L') && peekChar() == QLatin1Char('\''))
        nextChar(2); // slip L'
    else
        return;

    while (!currentChar().isNull()) {
        if (currentPosition() < 0)
            break;
        int len = m_endIdx - m_idx;

        if (len>=2 && (currentChar() == QLatin1Char('\\') && peekChar() == QLatin1Char('\''))) {
            nextChar(2);
        } else if (len>=2 && (currentChar() == QLatin1Char('\\') && peekChar() == QLatin1Char('\\'))) {
            nextChar(2);
        } else if (currentChar() == QLatin1Char('\'')) {
            nextChar();
            break;
        } else {
            nextChar();
        }
    }
}

void Lexer::readStringLiteral()
{
    if (currentChar() != QLatin1Char('"'))
        return;

    nextChar(); // skip "

    while (!currentChar().isNull()) {
        if (currentPosition() < 0)
            break;
        int len = m_endIdx - m_idx;

        if (len>=2 && currentChar() == QLatin1Char('\\') && peekChar() == QLatin1Char('"')) {
            nextChar(2);
        } else if (len>=2 && currentChar() == QLatin1Char('\\') && peekChar() == QLatin1Char('\\')) {
            nextChar(2);
        } else if (currentChar() == QLatin1Char('"')) {
            nextChar();
            break;
        } else {
            nextChar();
        }
    }
}

void Lexer::readNumberLiteral()
{
    while (currentChar().isLetterOrNumber() || currentChar() == QLatin1Char('.'))
        nextChar();
}

int Lexer::findOperator3() const
{
    if (currentPosition() < 0)
        return -1;
    int n = m_endIdx - m_idx;

    if (n >= 3) {
        char ch  = currentChar().toLatin1();
        char ch1 = peekChar().toLatin1();
        char ch2 = peekChar(2).toLatin1();

        if (ch == '<' && ch1 == '<' && ch2 == '=') return Token_assign;
        else if (ch == '>' && ch1 == '>' && ch2 == '=') return Token_assign;
        else if (ch == '-' && ch1 == '>' && ch2 == '*') return Token_ptrmem;
        else if (ch == '.' && ch1 == '.' && ch2 == '.') return Token_ellipsis;
    }

    return -1;
}

int Lexer::findOperator2() const
{
    if (currentPosition() < 0)
        return -1;
    int n = m_endIdx - m_idx;

    if (n>=2) {
        char ch = currentChar().toLatin1(), ch1 = peekChar().toLatin1();

        if (ch == ':' && ch1 == ':') return Token_scope;
        else if (ch == '.' && ch1 == '*') return Token_ptrmem;
        else if (ch == '+' && ch1 == '=') return Token_assign;
        else if (ch == '-' && ch1 == '=') return Token_assign;
        else if (ch == '*' && ch1 == '=') return Token_assign;
        else if (ch == '/' && ch1 == '=') return Token_assign;
        else if (ch == '%' && ch1 == '=') return Token_assign;
        else if (ch == '^' && ch1 == '=') return Token_assign;
        else if (ch == '&' && ch1 == '=') return Token_assign;
        else if (ch == '|' && ch1 == '=') return Token_assign;
        else if (ch == '<' && ch1 == '<') return Token_shift;
        //else if(ch == '>' && ch1 == '>') return Token_shift;
        else if (ch == '=' && ch1 == '=') return Token_eq;
        else if (ch == '!' && ch1 == '=') return Token_eq;
        else if (ch == '<' && ch1 == '=') return Token_leq;
        else if (ch == '>' && ch1 == '=') return Token_geq;
        else if (ch == '&' && ch1 == '&') return Token_and;
        else if (ch == '|' && ch1 == '|') return Token_or;
        else if (ch == '+' && ch1 == '+') return Token_incr;
        else if (ch == '-' && ch1 == '-') return Token_decr;
        else if (ch == '-' && ch1 == '>') return Token_arrow;
        else if (ch == '#' && ch1 == '#') return Token_concat;
    }

    return -1;
}

bool Lexer::skipWordsEnabled() const
{
    return m_skipWordsEnabled;
}

void Lexer::setSkipWordsEnabled(bool enabled)
{
    m_skipWordsEnabled = enabled;
}

bool Lexer::preprocessorEnabled() const
{
    return m_preprocessorEnabled;
}

void Lexer::setPreprocessorEnabled(bool enabled)
{
    m_preprocessorEnabled = enabled;
}

int Lexer::currentPosition() const
{
    return m_idx;
}

const QChar Lexer::currentChar() const
{
    return m_currentChar;
}

QChar Lexer::peekChar(int n) const
{
    if (m_idx + n >= m_endIdx)
        return QChar();
    return m_source[m_idx + n];
}

bool Lexer::eof() const
{
    return m_idx >= m_endIdx;
}

bool Lexer::reportWarnings() const
{
    return m_reportWarnings;
}

void Lexer::setReportWarnings(bool enable)
{
    m_reportWarnings = enable;
}

bool Lexer::reportMessages() const
{
    return m_reportMessages;
}

void Lexer::setReportMessages(bool enable)
{
    m_reportMessages = enable;
}

void Lexer::insertCurrent(const QString& str)
{
    if (currentPosition() < 0)
        return;
    m_source.insert(m_idx, str);

    m_endIdx = m_source.length();
    m_currentChar = m_source[m_idx];
}

Lexer::Lexer(Driver* driver)
  : d(new LexerData),
    m_driver(driver),
    m_recordComments(true),
    m_recordWhiteSpaces(false),
    m_skipWordsEnabled(true),
    m_preprocessorEnabled(true),
    m_inPreproc(false),
    m_reportWarnings(false),
    m_reportMessages(false)
{
    reset();
    d->beginScope();
}

Lexer::~Lexer()
{
    d->endScope();
    delete(d);
    qDeleteAll(m_tokens);
    m_tokens.clear();
}

void Lexer::setSource(const QString& source)
{
    reset();
    m_source = source;
    m_idx = 0;
    m_endIdx = m_source.length();
    m_inPreproc = false;
    if (m_source.isEmpty()) {
        m_currentChar = QChar();
        return;
    }
    m_currentChar = m_source[0];

    tokenize();
}

int Lexer::skippedLines() const
{
    return m_skippedLines;
}

void Lexer::reset()
{
    m_skippedLines = 0;
    m_index = 0;
    m_size = 0;
    m_tokens.clear();
    m_source = QString();
    m_idx = 0;
    m_endIdx = 0;
    m_startLine = false;
    m_ifLevel = 0;
    m_skipping.resize(200);
    m_skipping.fill(0);
    m_trueTest.resize(200);
    m_trueTest.fill(0);

    m_currentLine = 0;
    m_currentColumn = 0;
}

// ### should all be done with a "long" type IMO
int Lexer::toInt(const Token& token)
{
    QString s = token.text();
    if (token.type() == Token_number_literal) {
        // hex literal ?
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            return s.mid(2).toInt(nullptr, 16);
        QString n;
        int i = 0;
        while (i < int(s.length()) && s[i].isDigit())
            n += s[i++];
        // ### respect more prefixes and suffixes ?
        return n.toInt();
    } else if (token.type() == Token_char_literal) {
        int i = s[0] == 'L' ? 2 : 1; // wide char ?
        if (s[i] == '\\') {
            // escaped char
            int c = s[i+1].unicode();
            switch (c) {
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

void Lexer::getTokenPosition(const Token& token, int* line, int* col)
{
    token.getStartPosition(line, col);
}

void Lexer::nextToken(Token& tk, bool stopOnNewline)
{
    int op = 0;

    if (m_size == (int)m_tokens.size()) {
        m_tokens.resize(m_tokens.size() + 5000 + 1);
    }

    if (!readWhiteSpaces(!stopOnNewline))
        return;
    if (currentPosition() < 0)
        return;

    int startLine = m_currentLine;
    int startColumn = m_currentColumn;

    QChar ch = currentChar();
    QChar ch1 = peekChar();

    if (ch.isNull() || ch.isSpace()) {
        /* skip */
    } else if (m_startLine && ch == '#') {

        nextChar(); // skip #
        if (!readWhiteSpaces(false))     // skip white spaces
            return;
        m_startLine = false;

        int start = currentPosition();
        readIdentifier(); // read the directive
        QString directive = m_source.mid(start, currentPosition() - start);

        handleDirective(directive);
    } else if (m_startLine && m_skipping[ m_ifLevel ]) {
        // skip line and continue
        m_startLine = false;
        int ppe = preprocessorEnabled();
        setPreprocessorEnabled(false);
        while (!currentChar().isNull() && currentChar() != '\n') {
            Token tok(m_source);
            nextToken(tok, true);
        }
        ++m_skippedLines;
        m_startLine = true;
        setPreprocessorEnabled(ppe);
        return;
    } else if (ch == '/' && ch1 == '/') {
        int start = currentPosition();
        readLineComment();
        if (recordComments()) {
            tk = CREATE_TOKEN(Token_comment, start, currentPosition() - start);
            tk.setStartPosition(startLine, startColumn);
            tk.setEndPosition(m_currentLine, m_currentColumn);
        }
    } else if (ch == '/' && ch1 == '*') {
        int start = currentPosition();
        nextChar(2);
        readMultiLineComment();

        if (recordComments()) {
            tk = CREATE_TOKEN(Token_comment, start, currentPosition() - start);
            tk.setStartPosition(startLine, startColumn);
            tk.setEndPosition(m_currentLine, m_currentColumn);
        }
    } else if (ch == '\'' || (ch == 'L' && ch1 == '\'')) {
        int start = currentPosition();
        readCharLiteral();
        tk = CREATE_TOKEN(Token_char_literal, start, currentPosition() - start);
        tk.setStartPosition(startLine, startColumn);
        tk.setEndPosition(m_currentLine, m_currentColumn);
    } else if (ch == '"') {
        int start = currentPosition();
        readStringLiteral();
        tk = CREATE_TOKEN(Token_string_literal, start, currentPosition() - start);
        tk.setStartPosition(startLine, startColumn);
        tk.setEndPosition(m_currentLine, m_currentColumn);
    } else if (ch.isLetter() || ch == '_') {
        int start = currentPosition();
        readIdentifier();
        HashedString ide = m_source.mid(start, currentPosition() - start);
        int k = Lookup::find(ide);
        if (k == -1 && m_preprocessorEnabled) m_driver->usingString(ide);

        if (m_preprocessorEnabled && m_driver->hasMacro(ide) &&
            (k == -1 || !m_driver->macro(ide).body().isEmpty())) {

            bool preproc = m_preprocessorEnabled;
            m_preprocessorEnabled = false;

            d->beginScope();

            int svLine = currentLine();
            int svColumn = currentColumn();

            Macro m = m_driver->macro(ide);
            m_driver->usingMacro(m);

            QString ellipsisArg;

            if (m.hasArguments()) {
                int endIde = currentPosition();

                readWhiteSpaces();
                if (currentChar() == '(') {
                    nextChar();
                    int argIdx = 0;
                    int argCount = m.argumentList().size();
                    while (!currentChar().isNull() && argIdx<argCount) {
                        readWhiteSpaces();

                        QString argName = m.argumentList()[ argIdx ];

                        bool ellipsis = argName == "...";

                        QString arg = readArgument();

                        if (!ellipsis)
                            d->bind(argName, arg);
                        else
                            ellipsisArg += arg;

                        if (currentChar() == ',') {
                            nextChar();
                            if (!ellipsis) {
                                ++argIdx;
                            } else {
                                ellipsisArg += ", ";
                            }
                        } else if (currentChar() == ')') {
                            break;
                        }
                    }
                    if (currentChar() == ')') {
                        // valid macro
                        nextChar();
                    }
                } else {
                    tk = CREATE_TOKEN(Token_identifier, start, endIde - start);
                    tk.setStartPosition(svLine, svColumn);
                    tk.setEndPosition(svLine, svColumn + (endIde - start));

                    m_startLine = false;

                    d->endScope();        // OPS!!
                    m_preprocessorEnabled = preproc;
                    return;
                }
            }

            int argsEndAtLine = currentLine();
            int argsEndAtColumn = currentColumn();

#if defined(KDEVELOP_BGPARSER)
            qthread_yield();
#endif
            insertCurrent(m.body());

            // tokenize the macro body

            QString textToInsert;

            m_endIdx = m_idx + m.body().length();

            while (!currentChar().isNull()) {

                readWhiteSpaces();

                Token tok(m_source);
                nextToken(tok);

                bool stringify = !m_inPreproc && tok == '#';
                bool merge = !m_inPreproc && tok == Token_concat;

                if (stringify || merge)
                    nextToken(tok);

                if (tok == Token_eof)
                    break;

                QString tokText = tok.text();
                HashedString str = (tok == Token_identifier && d->hasBind(tokText)) ? d->apply(tokText) : tokText;
                if (str == ide) {
                    //Problem p(i18n("unsafe use of macro '%1', macro is ignored").arg(ide.str()), m_currentLine, m_currentColumn, Problem::Level_Warning);
                    //m_driver->addProblem(m_driver->currentFileName(), p);
                    m_driver->removeMacro(ide);
                    // str = QString::null;
                }

                if (stringify) {
                    textToInsert.append(QString::fromLatin1("\"") + str.str() + QString::fromLatin1("\" "));
                } else if (merge) {
                    textToInsert.truncate(textToInsert.length() - 1);
                    textToInsert.append(str.str()  + QString::fromLatin1(" "));
                } else if (tok == Token_ellipsis && d->hasBind("...")) {
                    textToInsert.append(ellipsisArg);
                } else {
                    textToInsert.append(str.str() + QString::fromLatin1(" "));
                }
            }

#if defined(KDEVELOP_BGPARSER)
            qthread_yield();
#endif
            insertCurrent(textToInsert); //also corrects the end-pointer

            d->endScope();
            m_preprocessorEnabled = preproc;
            //m_driver->addMacro(m);
            m_currentLine = argsEndAtLine;
            m_currentColumn = argsEndAtColumn;
        } else if (k != -1) {
            tk = CREATE_TOKEN(k, start, currentPosition() - start);
            tk.setStartPosition(startLine, startColumn);
            tk.setEndPosition(m_currentLine, m_currentColumn);
        } else if (m_skipWordsEnabled) {
            QHash< HashedString, QPair<SkipType, QString> >::iterator pos = m_words.find(ide);
            if (pos != m_words.end()) {
                if ((*pos).first == SkipWordAndArguments) {
                    readWhiteSpaces();
                    if (currentChar() == '(')
                        skip('(', ')');
                }
                if (!(*pos).second.isEmpty()) {
#if defined(KDEVELOP_BGPARSER)
                    qthread_yield();
#endif
                    insertCurrent(QString(" ") + (*pos).second + QString(" "));
                }
            } else if ( /*qt_rx.exactMatch(ide) ||*/
                ide.str().endsWith(QLatin1String("EXPORT")) ||
                (ide.str().startsWith(QLatin1String("Q_EXPORT")) && ide.str() != QLatin1String("Q_EXPORT_INTERFACE")) ||
                ide.str().startsWith(QLatin1String("QM_EXPORT")) ||
                ide.str().startsWith(QLatin1String("QM_TEMPLATE"))) {

                readWhiteSpaces();
                if (currentChar() == '(')
                    skip('(', ')');
            } else if (ide.str().startsWith(QLatin1String("K_TYPELIST_")) || ide.str().startsWith(QLatin1String("TYPELIST_"))) {
                tk = CREATE_TOKEN(Token_identifier, start, currentPosition() - start);
                tk.setStartPosition(startLine, startColumn);
                tk.setEndPosition(m_currentLine, m_currentColumn);
                readWhiteSpaces();
                if (currentChar() == '(')
                    skip('(', ')');
            } else {
                tk = CREATE_TOKEN(Token_identifier, start, currentPosition() - start);
                tk.setStartPosition(startLine, startColumn);
                tk.setEndPosition(m_currentLine, m_currentColumn);
            }
        } else {
            tk = CREATE_TOKEN(Token_identifier, start, currentPosition() - start);
            tk.setStartPosition(startLine, startColumn);
            tk.setEndPosition(m_currentLine, m_currentColumn);
        }
    } else if (ch.isNumber()) {
        int start = currentPosition();
        readNumberLiteral();
        tk = CREATE_TOKEN(Token_number_literal, start, currentPosition() - start);
        tk.setStartPosition(startLine, startColumn);
        tk.setEndPosition(m_currentLine, m_currentColumn);
    } else if (-1 != (op = findOperator3())) {
        tk = CREATE_TOKEN(op, currentPosition(), 3);
        nextChar(3);
        tk.setStartPosition(startLine, startColumn);
        tk.setEndPosition(m_currentLine, m_currentColumn);
    } else if (-1 != (op = findOperator2())) {
        tk = CREATE_TOKEN(op, currentPosition(), 2);
        nextChar(2);
        tk.setStartPosition(startLine, startColumn);
        tk.setEndPosition(m_currentLine, m_currentColumn);
    } else {
        tk = CREATE_TOKEN(ch.unicode(), currentPosition(), 1);
        nextChar();
        tk.setStartPosition(startLine, startColumn);
        tk.setEndPosition(m_currentLine, m_currentColumn);
    }

    m_startLine = false;
}


void Lexer::tokenize()
{
    m_startLine = true;
    m_size = 0;

    while (currentPosition() >= 0) {
        Token tk(m_source);
        nextToken(tk);

        if (tk.type() != -1)
            ADD_TOKEN(tk);

        if (currentChar().isNull())
            break;
    }

    Token tk = CREATE_TOKEN(Token_eof, currentPosition(), 0);
    tk.setStartPosition(m_currentLine, m_currentColumn);
    tk.setEndPosition(m_currentLine, m_currentColumn);
    ADD_TOKEN(tk);
}

void Lexer::resetSkipWords()
{
    m_words.clear();
}

void Lexer::addSkipWord(const QString& word, SkipType skipType, const QString& str)
{
    m_words[ word ] = qMakePair(skipType, str);
}

void Lexer::skip(int l, int r)
{
    int svCurrentLine = m_currentLine;
    int svCurrentColumn = m_currentColumn;

    int count = 0;

    while (!eof()) {
        Token tk(m_source);
        nextToken(tk);

        if ((int)tk == l)
            ++count;
        else if ((int)tk == r)
            --count;

        if (count == 0)
            break;
    }

    m_currentLine = svCurrentLine;
    m_currentColumn = svCurrentColumn;
}

QString Lexer::readArgument()
{
    int count = 0;

    QString arg;

    if (!readWhiteSpaces())
        return QString();
    while (!currentChar().isNull()) {

        readWhiteSpaces();
        QChar ch = currentChar();

        if (ch.isNull() || (!count && (ch == ',' || ch == ')')))
            break;

        Token tk(m_source);
        nextToken(tk);

        if (tk == '(') {
            ++count;
        } else if (tk == ')') {
            --count;
        }

        if (tk != -1)
            arg += tk.text() + ' ';
    }

    return arg.trimmed();
}

void Lexer::handleDirective(const QString& directive)
{
    m_inPreproc = true;

    bool skip = skipWordsEnabled();
    bool preproc = preprocessorEnabled();

    setSkipWordsEnabled(false);
    setPreprocessorEnabled(false);

    if (directive == "define") {
        if (!m_skipping[ m_ifLevel ]) {
            Macro m;
            processDefine(m);
        }
    } else if (directive == "else") {
        processElse();
    } else if (directive == "elif") {
        processElif();
    } else if (directive == "endif") {
        processEndif();
    } else if (directive == "if") {
        processIf();
    } else if (directive == "ifdef") {
        processIfdef();
    } else if (directive == "ifndef") {
        processIfndef();
    } else if (directive == "include") {
        if (!m_skipping[ m_ifLevel ]) {
            processInclude();
        }
    } else if (directive == "undef") {
        if (!m_skipping[ m_ifLevel ]) {
            processUndef();
        }
    }

    // skip line
    while (!currentChar().isNull() && currentChar() != '\n') {
        Token tk(m_source);
        nextToken(tk, true);
    }

    setSkipWordsEnabled(skip);
    setPreprocessorEnabled(preproc);

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
    if (!readWhiteSpaces(false))
        return 0;
    if (currentPosition() < 0)
        return 0;
    int startWord = currentPosition();
    readIdentifier();
    HashedString word = m_source.mid(startWord, currentPosition() - startWord);
    m_driver->usingString(word);
    bool r = m_driver->hasMacro(word);

    if (r) m_driver->usingMacro(m_driver->macro(word));

    return r;
}

void Lexer::processDefine(Macro& m)
{
    m.setFileName(m_driver->currentFileName());
    m.setLine(m_currentLine);
    m.setColumn(m_currentColumn);
    if (!readWhiteSpaces(false))
        return;
    if (currentPosition() < 0)
        return;

    int startMacroName = currentPosition();
    readIdentifier();
    QString macroName = m_source.mid(startMacroName, int(currentPosition()-startMacroName));
    m.setName(macroName);

    if (currentChar() == '(') {
        m.setHasArguments(true);
        nextChar();

        readWhiteSpaces(false);

        while (!currentChar().isNull() && currentChar() != ')') {
            readWhiteSpaces(false);

            int startArg = currentPosition();

            if (currentChar() == '.' && peekChar() == '.' && peekChar(2) == '.')
                nextChar(3);
            else
                readIdentifier();

            QString arg = m_source.mid(startArg, int(currentPosition()-startArg));

            m.addArgument(Macro::Argument(arg));

            readWhiteSpaces(false);
            if (currentChar() != ',')
                break;

            nextChar(); // skip ','
        }

        if (currentChar() == ')')
            nextChar(); // skip ')'
    }

    setPreprocessorEnabled(true);

    QString body;
    while (!currentChar().isNull() && currentChar() != '\n') {

        if (currentChar().isSpace()) {
            readWhiteSpaces(false);
            body += ' ';
        } else {

            Token tk(m_source);
            nextToken(tk, true);

            //Do not ignore c-style comments, those may be useful in the body, and ignoring them using this check causes problems
            if (tk.type() != -1 && (tk.type() != Token_comment || (tk.text().length() >= 2 && tk.text()[1] == '*'))) {
                QString s = tk.text();
                body += s;
            }
        }
    }

    m.setBody(body);
    m_driver->addMacro(m);
}

void Lexer::processElse()
{
    if (m_ifLevel == 0)
        /// @todo report error
        return;

    if (m_ifLevel > 0 && m_skipping[m_ifLevel-1])
        m_skipping[ m_ifLevel ] = m_skipping[ m_ifLevel - 1 ];
    else
        m_skipping[ m_ifLevel ] = m_trueTest[ m_ifLevel ];
}

void Lexer::processElif()
{
    if (m_ifLevel == 0)
        /// @todo report error
        return;

    if (!m_trueTest[m_ifLevel]) {
        /// @todo implement the correct semantic for elif!!
        bool inSkip = m_ifLevel > 0 && m_skipping[ m_ifLevel-1 ];
        m_trueTest[ m_ifLevel ] = macroExpression() != 0;
        m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    } else
        m_skipping[ m_ifLevel ] = true;
}

void Lexer::processEndif()
{
    if (m_ifLevel == 0)
        /// @todo report error
        return;

    m_skipping[ m_ifLevel ] = 0;
    m_trueTest[ m_ifLevel-- ] = 0;
}

void Lexer::processIf()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if (testIfLevel()) {
#if 0
        int n;
        if ((n = testDefined()) != 0) {
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

    if (testIfLevel()) {
        m_trueTest[ m_ifLevel ] = macroDefined();
        m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processIfndef()
{
    bool inSkip = m_skipping[ m_ifLevel ];

    if (testIfLevel()) {
        m_trueTest[ m_ifLevel ] = !macroDefined();
        m_skipping[ m_ifLevel ] = inSkip ? inSkip : !m_trueTest[ m_ifLevel ];
    }
}

void Lexer::processInclude()
{
    if (m_skipping[m_ifLevel])
        return;

    readWhiteSpaces(false);
    if (!currentChar().isNull()) {
        QChar ch = currentChar();
        if (ch == '"' || ch == '<') {
            nextChar();
            QChar ch2 = ch == QChar('"') ? QChar('"') : QChar('>');

            int startWord = currentPosition();
            if (startWord < 0)
                return;
            while (!currentChar().isNull() && currentChar() != ch2)
                nextChar();
            if (currentPosition() < 0)
                return;
            if (!currentChar().isNull()) {
                QString word = m_source.mid(startWord, int(currentPosition()-startWord));
                m_driver->addDependence(m_driver->currentFileName(),
                                        Dependence(word, ch == '"' ? Dep_Local : Dep_Global));
                nextChar();
            }
        }
    }
}

void Lexer::processUndef()
{
    readWhiteSpaces();
    int startWord = currentPosition();
    readIdentifier();
    QString word = m_source.mid(startWord, currentPosition() - startWord);

    Macro m(word, "");
    m.setFileName(m_driver->currentFileName());
    m.setUndef();

    ///Adds an undef-macro that shadows the previous macro
    m_driver->addMacro(m);
}

int Lexer::macroPrimary()
{
    if (!readWhiteSpaces(false))
        return 0;
    int result = 0;
    switch (currentChar().toLatin1()) {
    case '(':
        nextChar();
        result = macroExpression();
        if (currentChar() != ')') {
            /// @todo report error
            return 0;
        }
        nextChar();
        return result;

    case '+':
    case '-':
    case '!':
    case '~': {
        QChar tk = currentChar();
        nextChar();
        int result = macroPrimary();
        if (tk == '-') return -result;
        else if (tk == '!') return !result;
        else if (tk == '~') return ~result;
    }
    break;

    default: {
        Token tk(m_source);
        nextToken(tk, false);
        switch (tk.type()) {
        case Token_identifier:
            if (tk.text() == "defined") {
                return macroPrimary();
            }
            /// @todo implement
            {
                HashedString h(tk.text());
                m_driver->usingString(h);
                if (m_driver->hasMacro(h)) {
                    m_driver->usingMacro(m_driver->macro(h));
                    Macro &m = m_driver->macro(h);
                    Lexer lexer(m_driver);
                    lexer.setSource(m.body());
                    int result = lexer.macroExpression();
                    return result;
                } else {
                    return false;
                }
            }
        case Token_number_literal:
        case Token_char_literal:
            return toInt(tk);
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
    while (readWhiteSpaces(false)) {
        if (currentChar() == '*')
            op = 0;
        else if (currentChar() == '/' && !(peekChar() == '*' || peekChar() == '/'))
            op = 1;
        else if (currentChar() == '%')
            op = 2;
        else
            break;
        nextChar();
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
    if (!readWhiteSpaces(false))
        return result;
    while (currentChar() == '+' || currentChar() == '-') {
        ad = currentChar() == '+';
        nextChar();
        iresult = macroMultiplyDivide();
        result = ad ? (result+iresult) : (result-iresult);
    }
    return result;
}

int Lexer::macroRelational()
{
    int result = macroAddSubtract();
    int iresult;
    if (!readWhiteSpaces(false))
        return result;
    while (currentChar() == '<' || currentChar() == '>') {
        int lt = currentChar() == '<';
        nextChar();
        if (currentChar() == '=') {
            nextChar();

            iresult = macroAddSubtract();
            result = lt ? (result <= iresult) : (result >= iresult);
        } else {
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
    if (!readWhiteSpaces(false))
        return result;
    while ((currentChar() == '=' || currentChar() == '!') && peekChar() == '=') {
        eq = currentChar() == '=';
        nextChar(2);
        iresult = macroRelational();
        result = eq ? (result==iresult) : (result!=iresult);
    }
    return result;
}

int Lexer::macroBoolAnd()
{
    int result = macroEquality();
    if (!readWhiteSpaces(false))
        return result;
    while (currentChar() == '&' && peekChar() != '&') {
        nextChar();
        result &= macroEquality();
    }
    return result;
}

int Lexer::macroBoolXor()
{
    int result = macroBoolAnd();
    if (!readWhiteSpaces(false))
        return result;
    while (currentChar() == '^') {
        nextChar();
        result ^= macroBoolAnd();
    }
    return result;
}

int Lexer::macroBoolOr()
{
    int result = macroBoolXor();
    if (!readWhiteSpaces(false))
        return result;
    while (currentChar() == '|' && peekChar() != '|') {
        nextChar();
        result |= macroBoolXor();
    }
    return result;
}

int Lexer::macroLogicalAnd()
{
    int result = macroBoolOr();
    if (!readWhiteSpaces(false))
        return result;
    while (currentChar() == '&' && peekChar() == '&') {
        nextChar(2);
        int start = currentPosition();
        result = macroBoolOr() && result;
        QString s = m_source.mid(start, currentPosition() - start);
    }
    return result;
}

int Lexer::macroLogicalOr()
{
    int result = macroLogicalAnd();
    if (!readWhiteSpaces(false))
        return result;
    while (currentChar() == '|' && peekChar() == '|') {
        nextChar(2);
        result = macroLogicalAnd() || result;
    }
    return result;
}

int Lexer::macroExpression()
{
    if (!readWhiteSpaces(false))
        return 0;
    return macroLogicalOr();
}
