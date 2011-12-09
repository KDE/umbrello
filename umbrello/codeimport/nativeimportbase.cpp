/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "nativeimportbase.h"

// app includes
#include "codeimpthread.h"
#include "debug_utils.h"
#include "import_utils.h"

// kde includes
#include <klocale.h>

// qt includes
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

/**
 * Constructor
 * @param singleLineCommentIntro  "//" for IDL and Java, "--" for Ada
 * @param thread                  thread in which the code import runs
 */
NativeImportBase::NativeImportBase(const QString &singleLineCommentIntro, CodeImpThread* thread)
  : ClassImport(thread),
    m_singleLineCommentIntro(singleLineCommentIntro),
    m_srcIndex(0),
    m_scopeIndex(0),  // index 0 is reserved for global scope
    m_klass(0),
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
 * given by `until'.
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
            closing = '}';
            break;
        case '[':
            closing = ']';
            break;
        case '(':
            closing = ')';
            break;
        case '<':
            closing = '>';
            break;
        default:
            uError() << "opener='" << opener << "': illegal input character";
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
 * Advance m_srcIndex until m_source[m_srcIndex] contains a non-comment.
 * Comments encountered during advancement are accumulated in `m_comment'.
 * If m_srcIndex hits the end of m_source then QString() is returned.
 * @return the current lexeme or an empty string
 */
QString NativeImportBase::advance()
{
    while (m_srcIndex < m_source.count() - 1) {
        m_srcIndex++;
        if (m_source[m_srcIndex].startsWith(m_singleLineCommentIntro))
            m_comment += m_source[m_srcIndex];
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
                m_comment += line + '\n';
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
        m_source.append(m_singleLineCommentIntro + m_comment);  // denotes comments in `m_source'
        m_srcIndex++;
        m_comment = "";
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
        int endpos = line.indexOf(m_multiLineCommentEnd);
        if (endpos != -1) {
            delimEndLen = m_multiLineCommentEnd.length();
        } else if (!m_multiLineAltCommentEnd.isEmpty()) {
            endpos = line.indexOf(m_multiLineAltCommentEnd);
            if (endpos != -1)
                delimEndLen = m_multiLineAltCommentEnd.length();
        }
        if (endpos == -1) {
            m_inComment = true;
            if (pos + delimIntroLen < (int)line.length()) {
                QString cmnt = line.mid(pos + delimIntroLen);
                m_comment += cmnt.trimmed() + '\n';
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
    QChar stringIntro = 0;  // buffers the string introducer character
    bool seenSpace = false;
    QString ln = line.trimmed();
    for (int i = 0; i < ln.length(); ++i) {
        const QChar& c = ln[i];
        if (stringIntro.toLatin1()) {        // we are in a string
            listElement += c;
            if (c == stringIntro) {
                if (ln[i - 1] != '\\') {
                    list.append(listElement);
                    listElement.clear();
                    stringIntro = 0;  // we are no longer in a string
                }
            }
        } else if (c == '"' || c == '\'') {
            if (!listElement.isEmpty()) {
                list.append(listElement);
            }
            listElement = stringIntro = c;
            seenSpace = false;
        } else if (c == ' ' || c == '\t') {
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
 * The lexer. Tokenizes the given string and fills `m_source'.
 * Stores possible comments in `m_comment'.
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
    if (ln.contains(QRegExp("^\\s*$")))
        return;
    const QStringList words = split(ln);
    for (QStringList::ConstIterator it = words.begin(); it != words.end(); ++it) {
        QString word = *it;
        if (word[0] == '"' || word[0] == '\'')
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
    nameWithoutPath.remove(QRegExp("^.*/"));
    if (m_parsedFiles.contains(nameWithoutPath))
        return true;
    m_parsedFiles.append(nameWithoutPath);
    QString fname = filename;
    const QString msgPrefix = filename + ": ";
    if (filename.contains('/')) {
        QString path = filename;
        path.remove( QRegExp("/[^/]+$") );
        uDebug() << msgPrefix << "adding path " << path;
        Import_Utils::addIncludePath(path);
    }
    if (! QFile::exists(filename)) {
        if (filename.startsWith('/')) {
            uError() << msgPrefix << "cannot find file";
            return false;
        }
        bool found = false;
        const QStringList includePaths = Import_Utils::includePathList();
        for (QStringList::ConstIterator pathIt = includePaths.begin();
                                   pathIt != includePaths.end(); ++pathIt) {
            QString path = (*pathIt);
            if (! path.endsWith('/')) {
                path.append('/');
            }
            if (QFile::exists(path + filename)) {
                fname.prepend(path);
                found = true;
                break;
            }
        }
        if (! found) {
            uError() << msgPrefix << "cannot find file";
            return false;
        }
    }
    QFile file(fname);
    if (! file.open(QIODevice::ReadOnly)) {
        uError() << msgPrefix << "cannot open file";
        return false;
    }
    log(nameWithoutPath, "parsing...");
    // Scan the input file into the QStringList m_source.
    m_source.clear();
    m_srcIndex = 0;
    initVars();
    QTextStream stream(&file);
    while (! stream.atEnd()) {
        QString line = stream.readLine();
        scan(line);
    }
    file.close();
    // Parse the QStringList m_source.
    m_klass = NULL;
    m_currentAccess = Uml::Visibility::Public;
    m_scopeIndex = 0;
    m_scope[0] = NULL;  // index 0 is reserved for global scope
    const int srcLength = m_source.count();
    for (m_srcIndex = 0; m_srcIndex < srcLength; ++m_srcIndex) {
        const QString& firstToken = m_source[m_srcIndex];
        //uDebug() << '"' << firstToken << '"';
        if (firstToken.startsWith(m_singleLineCommentIntro)) {
            m_comment = firstToken.mid(m_singleLineCommentIntro.length());
            continue;
        }
        if (! parseStmt())
           skipStmt();
        m_comment.clear();
    }
    log(nameWithoutPath, "...end of parse");
    return true;
}

/**
 * Implement abstract operation from ClassImport.
 */
void NativeImportBase::initialize()
{
    m_parsedFiles.clear();
}
