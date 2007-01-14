/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "nativeimportbase.h"

// qt/kde includes
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <klocale.h>
#include <kdebug.h>
// app includes
#include "import_utils.h"

NativeImportBase::NativeImportBase(const QString &singleLineCommentIntro) {
    m_singleLineCommentIntro = singleLineCommentIntro;
    m_srcIndex = 0;
    m_scopeIndex = 0;  // index 0 is reserved for global scope
    m_klass = NULL;
    m_currentAccess = Uml::Visibility::Public;
    m_isAbstract = false;
    m_inComment = false;
}

NativeImportBase::~NativeImportBase() {
}

void NativeImportBase::setMultiLineComment(const QString &intro, const QString &end) {
    m_multiLineCommentIntro = intro;
    m_multiLineCommentEnd = end;
}

void NativeImportBase::setMultiLineAltComment(const QString &intro, const QString &end) {
    m_multiLineAltCommentIntro = intro;
    m_multiLineAltCommentEnd = end;
}

void NativeImportBase::skipStmt(QString until /* = ";" */) {
    const uint srcLength = m_source.count();
    while (m_srcIndex < srcLength && m_source[m_srcIndex] != until)
        m_srcIndex++;
}

bool NativeImportBase::skipToClosing(QChar opener) {
    QString closing;
    switch (opener) {
        case '{':
            closing = "}";
            break;
        case '[':
            closing = "]";
            break;
        case '(':
            closing = ")";
            break;
        case '<':
            closing = ">";
            break;
        default:
            kError() << "NativeImportBase::skipToClosing(" << opener
                << "): " << "illegal input character" << endl;
            return false;
    }
    const QString opening(opener);
    skipStmt(opening);
    const uint srcLength = m_source.count();
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

QString NativeImportBase::advance() {
    while (m_srcIndex < m_source.count() - 1) {
        if (m_source[++m_srcIndex].startsWith(m_singleLineCommentIntro))
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

bool NativeImportBase::preprocess(QString& line) {
    if (m_multiLineCommentIntro.isEmpty())
        return false;
    // Check for end of multi line comment.
    if (m_inComment) {
        int delimiterLen = 0;
        int pos = line.find(m_multiLineCommentEnd);
        if (pos == -1) {
            if (! m_multiLineAltCommentEnd.isEmpty())
                pos = line.find(m_multiLineAltCommentEnd);
            if (pos == -1) {
                m_comment += line + "\n";
                return true;  // done
            }
            delimiterLen = m_multiLineAltCommentEnd.length();
        } else {
            delimiterLen = m_multiLineCommentEnd.length();
        }
        if (pos > 0) {
            QString text = line.mid(0, pos - 1);
            m_comment += text.stripWhiteSpace();
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
    int pos = line.find(m_multiLineCommentIntro);
    if (pos != -1) {
        delimIntroLen = m_multiLineCommentIntro.length();
    } else if (!m_multiLineAltCommentIntro.isEmpty()) {
        pos = line.find(m_multiLineAltCommentIntro);
        if (pos != -1)
            delimIntroLen = m_multiLineAltCommentIntro.length();
    }
    if (pos != -1) {
        int endpos = line.find(m_multiLineCommentEnd);
        if (endpos != -1) {
            delimEndLen = m_multiLineCommentEnd.length();
        } else if (!m_multiLineAltCommentEnd.isEmpty()) {
            endpos = line.find(m_multiLineAltCommentEnd);
            if (endpos != -1)
                delimEndLen = m_multiLineAltCommentEnd.length();
        }
        if (endpos == -1) {
            m_inComment = true;
            if (pos + delimIntroLen < (int)line.length()) {
                QString cmnt = line.mid(pos + delimIntroLen);
                m_comment += cmnt.stripWhiteSpace() + "\n";
            }
            if (pos == 0)
                return true;  // done
            line = line.left(pos);
        } else {   // It's a multiline comment on a single line.
            if (endpos > pos + delimIntroLen)  {
                QString cmnt = line.mid(pos + delimIntroLen, endpos - pos - delimIntroLen);
                cmnt = cmnt.stripWhiteSpace();
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

/// Split the line so that a string is returned as a single element of the list,
/// when not in a string then split at white space.
QStringList NativeImportBase::split(const QString& lin) {
    QStringList list;
    QString listElement;
    QChar stringIntro = 0;  // buffers the string introducer character
    bool seenSpace = false;
    QString line = lin.stripWhiteSpace();
    for (uint i = 0; i < line.length(); i++) {
        const QChar& c = line[i];
        if (stringIntro) {        // we are in a string
            listElement += c;
            if (c == stringIntro) {
                if (line[i - 1] != '\\') {
                    list.append(listElement);
                    listElement = QString();
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
                listElement = QString();
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

/// The lexer. Tokenizes the given string and fills `m_source'.
/// Stores possible comments in `m_comment'.
void NativeImportBase::scan(QString line) {
    if (preprocess(line))
        return;
    // Check for single line comment.
    int pos = line.find(m_singleLineCommentIntro);
    if (pos != -1) {
        QString cmnt = line.mid(pos);
        m_source.append(cmnt);
        if (pos == 0)
            return;
        line = line.left(pos);
    }
    if (line.contains(QRegExp("^\\s*$")))
        return;
    QStringList words = split(line);
    for (QStringList::Iterator it = words.begin(); it != words.end(); ++it) {
        QString word = *it;
        if (word[0] == '"' || word[0] == '\'')
            m_source.append(word);  // string constants are handled by split()
        else
            fillSource(word);
    }
}

void NativeImportBase::initVars() {
}

void NativeImportBase::parseFile(const QString& filename) {
    QString nameWithoutPath = filename;
    nameWithoutPath.remove(QRegExp("^.*/"));
    if (m_parsedFiles.contains(nameWithoutPath))
        return;
    m_parsedFiles.append(nameWithoutPath);
    QString fname = filename;
    const QString msgPrefix = "NativeImportBase::parseFile(" + filename + "): ";
    if (filename.contains('/')) {
        QString path = filename;
        path.remove( QRegExp("/[^/]+$") );
        kDebug() << msgPrefix << "adding path " << path << endl;
        Import_Utils::addIncludePath(path);
    }
    if (! QFile::exists(filename)) {
        if (filename.startsWith("/")) {
            kError() << msgPrefix << "cannot find file" << endl;
            return;
        }
        bool found = false;
        QStringList includePaths = Import_Utils::includePathList();
        for (QStringList::Iterator pathIt = includePaths.begin();
                                   pathIt != includePaths.end(); ++pathIt) {
            QString path = (*pathIt);
            if (! path.endsWith("/")) {
                path.append("/");
            }
            if (QFile::exists(path + filename)) {
                fname.prepend(path);
                found = true;
                break;
            }
        }
        if (! found) {
            kError() << msgPrefix << "cannot find file" << endl;
            return;
        }
    }
    QFile file(fname);
    if (! file.open(IO_ReadOnly)) {
        kError() << msgPrefix << "cannot open file" << endl;
        return;
    }
    kDebug() << msgPrefix << "parsing." << endl;
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
    const uint srcLength = m_source.count();
    for (m_srcIndex = 0; m_srcIndex < srcLength; m_srcIndex++) {
        const QString& firstToken = m_source[m_srcIndex];
        //kDebug() << '"' << firstToken << '"' << endl;
        if (firstToken.startsWith(m_singleLineCommentIntro)) {
            m_comment = firstToken.mid(m_singleLineCommentIntro.length());
            continue;
        }
        if (! parseStmt())
           skipStmt();
        m_comment = QString();
    }
    kDebug() << msgPrefix << "end of parse." << endl;
}

void NativeImportBase::initialize() {
    m_parsedFiles.clear();
}

