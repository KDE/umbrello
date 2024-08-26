/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "nativeimportbase.h"

// app includes
#include "codeimpthread.h"
#define DBG_SRC QStringLiteral("NativeImportBase")
#include "debug_utils.h"
#include "import_utils.h"
#include "uml.h"  // only needed for log{Warn,Error}

// kde includes
#include <KLocalizedString>

// qt includes
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

DEBUG_REGISTER_DISABLED(NativeImportBase)

QStringList NativeImportBase::m_parsedFiles;  // static, see nativeimportbase.h

/**
 * Constructor
 * @param singleLineCommentIntro  "//" for IDL and Java, "--" for Ada
 * @param thread                  thread in which the code import runs
 */
NativeImportBase::NativeImportBase(const QString &singleLineCommentIntro, CodeImpThread* thread)
  : ClassImport(thread),
    m_singleLineCommentIntro(singleLineCommentIntro),
    m_srcIndex(0),
    m_klass(nullptr),
    m_currentAccess(Uml::Visibility::Public),
    m_inComment(false),
    m_isAbstract(false)
{
}

/**
 * Destructor.
 */
NativeImportBase::~NativeImportBase()
{
}

/**
 * Set the delimiter strings for a multi line comment.
 * @param intro  In languages with a C style multiline comment
 *               this is slash-star.
 * @param end    In languages with a C style multiline comment
 *               this is star-slash.
 */
void NativeImportBase::setMultiLineComment(const QString &intro, const QString &end)
{
    m_multiLineCommentIntro = intro;
    m_multiLineCommentEnd = end;
}

/**
 * Set the delimiter strings for an alternative form of
 * multi line comment. See setMultiLineComment().
 * @param intro   the start comment string
 * @param end     the end comment string
 */
void NativeImportBase::setMultiLineAltComment(const QString &intro, const QString &end)
{
    m_multiLineAltCommentIntro = intro;
    m_multiLineAltCommentEnd = end;
}

/**
 * Advance m_srcIndex until m_source[m_srcIndex] contains the lexeme
 * given by `until`.
 * @param until   the target string
 */
void NativeImportBase::skipStmt(const QString& until /* = ";" */)
{
    const int srcLength = m_source.count();
    while (m_srcIndex < srcLength && m_source[m_srcIndex] != until)
        m_srcIndex++;
}

/**
 * Advance m_srcIndex to the index of the corresponding closing character
 * of the given opening.  Nested opening/closing pairs are respected.
 * Valid openers are:    '{'  '['  '('  '<'
 * @param  opener   the opener string
 * @return  True for success, false for misuse (invalid opener) or
 *          if no matching closing character is found in m_source.
 */
bool NativeImportBase::skipToClosing(QChar opener)
{
    QString closing;
    switch (opener.toLatin1()) {
        case '{':
            closing = QStringLiteral("}");
            break;
        case '[':
            closing = QStringLiteral("]");
            break;
        case '(':
            closing = QStringLiteral(")");
            break;
        case '<':
            closing = QStringLiteral(">");
            break;
        default:
            logError1("NativeImportBase::skipToClosing opener='%1': illegal input character", opener);
            return false;
    }
    const QString opening(opener);
    skipStmt(opening);
    const int srcLength = m_source.count();
    int nesting = 0;
    while (m_srcIndex < srcLength) {
        QString nextToken = advance();
        if (nextToken.isEmpty())
            break;
        if (nextToken == closing) {
            if (nesting <= 0)
                break;
            nesting--;
        } else if (nextToken == opening) {
            nesting++;
        }
    }
    if (m_srcIndex == srcLength)
        return false;
    return true;
}

/**
 * Set package as current scope.

 * @param p UML package to set as current scope
 */
void NativeImportBase::pushScope(UMLPackage *p)
{
    m_scope.append(p);
}

/**
 * Return previously defined scope.
 *
 * @return previous scope
 */
UMLPackage *NativeImportBase::popScope()
{
    m_scope.takeLast();
    UMLPackage *p = m_scope.last();
    return p;
}

/**
 * Return current scope.
 * If the scope stack is empty then return nullptr.
 *
 * @return scope
 */
UMLPackage *NativeImportBase::currentScope()
{
    UMLPackage *p = m_scope.last();
    return p;
}

/**
 * Return current scope index.
 *
 * @return >= 0 index, -1 empty
 */
int NativeImportBase::scopeIndex()
{
    return m_scope.size() - 1;
}

/**
 * Get the current lexeme.
 * If the end of parse was reached then return an empty string.
 * @return  the current lexeme
 */
QString NativeImportBase::current()
{
    if (m_srcIndex < m_source.count() - 1)
        return m_source[m_srcIndex];
    return QString();
}

/**
 * Get the next lexeme without advancing.
 * @return  the next lexeme or an empty string
 */
QString NativeImportBase::lookAhead()
{
    if (m_srcIndex < m_source.count() - 1)
        return m_source[m_srcIndex+1];
    return QString();
}

/**
 * Advance m_srcIndex until m_source[m_srcIndex] contains a non-comment.
 * Comments encountered during advancement are accumulated in `m_comment`.
 * If m_srcIndex hits the end of m_source then QString() is returned.
 * @return the current lexeme or an empty string
 */
QString NativeImportBase::advance()
{
    while (m_srcIndex < m_source.count() - 1) {
        m_srcIndex++;
        if (m_source[m_srcIndex].startsWith(m_singleLineCommentIntro))
            m_comment += m_source[m_srcIndex].mid(m_singleLineCommentIntro.length());
        else
            break;
    }
    if (m_srcIndex >= m_source.count() - 1 ||
        // if last item in m_source is a comment then it is dropped too
        (m_srcIndex == m_source.count() - 1 &&
         m_source[m_srcIndex].startsWith(m_singleLineCommentIntro))) {
        return QString();
    }
    return m_source[m_srcIndex];
}

/**
 * Preprocess a line.
 * May modify the given line to remove items consumed by the
 * preprocessing such as comments or preprocessor directives.
 * The default implementation handles multi-line comments.
 * @param line  The line to preprocess.
 * @return      True if the line was completely consumed,
 *              false if there are still items left in the line
 *              for further analysis.
 */
bool NativeImportBase::preprocess(QString& line)
{
    if (line.isEmpty())
        return true;
    if (m_multiLineCommentIntro.isEmpty())
        return false;
    // Check for end of multi line comment.
    if (m_inComment) {
        int delimiterLen = 0;
        int pos = line.indexOf(m_multiLineCommentEnd);
        if (pos == -1) {
            if (! m_multiLineAltCommentEnd.isEmpty())
                pos = line.indexOf(m_multiLineAltCommentEnd);
            if (pos == -1) {
                m_comment += line + QLatin1Char('\n');
                return true;  // done
            }
            delimiterLen = m_multiLineAltCommentEnd.length();
        } else {
            delimiterLen = m_multiLineCommentEnd.length();
        }
        if (pos > 0) {
            QString text = line.mid(0, pos - 1);
            m_comment += text.trimmed();
        }
        m_source.append(m_singleLineCommentIntro + m_comment);  // denotes comments in `m_source`
        m_srcIndex++;
        m_comment = QString();
        m_inComment = false;
        pos += delimiterLen;  // pos now points behind the closed comment
        if (pos == (int)line.length())
            return true;  // done
        line = line.mid(pos);
    }
    // If we get here then m_inComment is false.
    // Check for start of multi line comment.
    int delimIntroLen = 0;
    int delimEndLen = 0;
    int pos = line.indexOf(m_multiLineCommentIntro);
    if (pos != -1) {
        delimIntroLen = m_multiLineCommentIntro.length();
    } else if (!m_multiLineAltCommentIntro.isEmpty()) {
        pos = line.indexOf(m_multiLineAltCommentIntro);
        if (pos != -1)
            delimIntroLen = m_multiLineAltCommentIntro.length();
    }
    if (pos != -1) {
        int sPos = line.indexOf(m_singleLineCommentIntro);
        if (sPos != -1 && sPos < pos) {
            // multi line comment intro found in single line comment
            pos = -1;      // is no multi line comment after all
        }
    }
    if (pos != -1) {
        int endpos = line.indexOf(m_multiLineCommentEnd, pos + delimIntroLen);
        if (endpos != -1) {
            delimEndLen = m_multiLineCommentEnd.length();
        } else if (!m_multiLineAltCommentEnd.isEmpty()) {
            endpos = line.indexOf(m_multiLineAltCommentEnd, pos + delimIntroLen);
            if (endpos != -1)
                delimEndLen = m_multiLineAltCommentEnd.length();
        }
        if (endpos == -1) {
            m_inComment = true;
            if (pos + delimIntroLen < (int)line.length()) {
                QString cmnt = line.mid(pos + delimIntroLen);
                m_comment += cmnt.trimmed() + QLatin1Char('\n');
            }
            if (pos == 0)
                return true;  // done
            line = line.left(pos);
        } else {   // It's a multiline comment on a single line.
            if (endpos > pos + delimIntroLen)  {
                QString cmnt = line.mid(pos + delimIntroLen, endpos - pos - delimIntroLen);
                cmnt = cmnt.trimmed();
                if (!cmnt.isEmpty())
                    m_source.append(m_singleLineCommentIntro + cmnt);
            }
            endpos++;  // endpos now points at the slash of "*/"
            QString pre;
            if (pos > 0)
                pre = line.left(pos);
            QString post;
            if (endpos + delimEndLen < (int)line.length())
                post = line.mid(endpos + 1);
            line = pre + post;
        }
    }
    return false;  // The input was not completely consumed by preprocessing.
}

/**
 * Split the line so that a string is returned as a single element of the list.
 * When not in a string then split at white space.
 * The default implementation is suitable for C style strings and char constants.
 * @param line   the line to split
 * @return the parts of the line
 */
QStringList NativeImportBase::split(const QString& line)
{
    QStringList list;
    QString listElement;
    QChar stringIntro;  // buffers the string introducer character
    bool seenSpace = false;
    QString ln = line.trimmed();
    for (int i = 0; i < ln.length(); ++i) {
        const QChar& c = ln[i];
        if (stringIntro.toLatin1()) {        // we are in a string
            listElement += c;
            if (c == stringIntro) {
                if (ln[i - 1] != QLatin1Char('\\')) {
                    list.append(listElement);
                    listElement.clear();
                    stringIntro = QChar();  // we are no longer in a string
                }
            }
        } else if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            if (!listElement.isEmpty()) {
                list.append(listElement);
            }
            listElement = stringIntro = c;
            seenSpace = false;
        } else if (c == QLatin1Char(' ') || c == QLatin1Char('\t')) {
            if (seenSpace)
                continue;
            seenSpace = true;
            if (!listElement.isEmpty()) {
                list.append(listElement);
                listElement.clear();
            }
        } else {
            listElement += c;
            seenSpace = false;
        }
    }
    if (!listElement.isEmpty())
        list.append(listElement);
    return list;
}

/**
 * Scan a single line.
 * parseFile() calls this for each line read from the input file.
 * This in turn calls other methods such as preprocess() and fillSource().
 * The lexer. Tokenizes the given string and fills `m_source`.
 * Stores possible comments in `m_comment`.
 * @param line  The line to scan.
 */
void NativeImportBase::scan(const QString& line)
{
    QString ln = line;
    if (preprocess(ln))
        return;
    // Check for single line comment.
    int pos = ln.indexOf(m_singleLineCommentIntro);
    if (pos != -1) {
        QString cmnt = ln.mid(pos);
        m_source.append(cmnt);
        if (pos == 0)
            return;
        ln = ln.left(pos);
    }
    if (ln.contains(QRegularExpression(QStringLiteral("^\\s*$"))))
        return;
    const QStringList words = split(ln);
    for (QStringList::ConstIterator it = words.begin(); it != words.end(); ++it) {
        QString word = *it;
        if (word[0] == QLatin1Char('"') || word[0] == QLatin1Char('\''))
            m_source.append(word);  // string constants are handled by split()
        else
            fillSource(word);
    }
}

/**
 * Initialize auxiliary variables.
 * This is called by the default implementation of parseFile()
 * after scanning (before parsing the QStringList m_source.)
 * The default implementation is empty.
 */
void NativeImportBase::initVars()
{
}

/**
 * Import a single file.
 * The default implementation should be feasible for languages that
 * don't depend on an external preprocessor.
 * @param filename  The file to import.
 * @return state of parsing - false means errors 
 */
bool NativeImportBase::parseFile(const QString& filename)
{
    QString nameWithoutPath = filename;
    nameWithoutPath.remove(QRegularExpression(QStringLiteral("^.*/")));
    if (m_parsedFiles.contains(nameWithoutPath))
        return true;
    m_parsedFiles.append(nameWithoutPath);
    QString fname = filename;
    const QString msgPrefix = filename + QStringLiteral(": ");
    if (filename.contains(QLatin1Char('/'))) {
        QString path = filename;
        path.remove(QRegularExpression(QStringLiteral("/[^/]+$")));
        logDebug2("NativeImportBase::parseFile %1 adding path %2", msgPrefix, path);
        Import_Utils::addIncludePath(path);
    }
    if (!QFile::exists(filename)) {
        QFileInfo fi(filename);
        if (fi.isAbsolute()) {
            logError1("NativeImportBase::parseFile: cannot find absolute file %1", filename);
            return false;
        }
        bool found = false;
        const QStringList includePaths = Import_Utils::includePathList();
        for (QStringList::ConstIterator pathIt = includePaths.begin();
                                   pathIt != includePaths.end(); ++pathIt) {
            QString path = (*pathIt);
            if (! path.endsWith(QLatin1Char('/'))) {
                path.append(QLatin1Char('/'));
            }
            if (QFile::exists(path + filename)) {
                fname.prepend(path);
                found = true;
                break;
            }
        }
        if (! found) {
            logError1("NativeImportBase::parseFile: cannot find file %1", filename);
            return false;
        }
    }
    QFile file(fname);
    if (! file.open(QIODevice::ReadOnly)) {
        logError1("NativeImportBase::parseFile: cannot open file %1", fname);
        return false;
    }
    log(nameWithoutPath, QStringLiteral("parsing..."));
    // Scan the input file into the QStringList m_source.
    m_source.clear();
    m_srcIndex = 0;
    initVars();
    QTextStream stream(&file);
    int lineCount = 0;
    while (! stream.atEnd()) {
        QString line = stream.readLine();
        lineCount++;
        scan(line);
    }
    log(nameWithoutPath, QStringLiteral("file size: ") + QString::number(file.size()) +
                         QStringLiteral(" / lines: ") + QString::number(lineCount));
    file.close();
    // Parse the QStringList m_source.
    m_klass = nullptr;
    m_currentAccess = Uml::Visibility::Public;
    m_scope.clear();
    pushScope(Import_Utils::globalScope()); // index 0 is reserved for the global scope
    const int srcLength = m_source.count();
    for (m_srcIndex = 0; m_srcIndex < srcLength; ++m_srcIndex) {
        const QString& firstToken = m_source[m_srcIndex];
        //uDebug() << '"' << firstToken << '"';
        if (firstToken.startsWith(m_singleLineCommentIntro)) {
            m_comment += firstToken.mid(m_singleLineCommentIntro.length());
            continue;
        }
        if (! parseStmt())
           skipStmt();
        m_comment.clear();
    }
    log(nameWithoutPath, QStringLiteral("...end of parse"));
    return true;
}

/**
 * Implement abstract operation from ClassImport.
 */
void NativeImportBase::initialize()
{
    m_parsedFiles.clear();
}
