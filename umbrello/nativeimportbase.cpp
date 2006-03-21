/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
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

NativeImportBase::NativeImportBase(QString singleLineCommentIntro) {
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

void NativeImportBase::skipStmt(QString until /* = ";" */) {
    const uint srcLength = m_source.count();
    while (m_srcIndex < srcLength && m_source[m_srcIndex] != until)
        m_srcIndex++;
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
        return QString::null;
    }
    return m_source[m_srcIndex];
}

bool NativeImportBase::preprocess(QString& line) {
    // Check for end of multi line comment.
    if (m_inComment) {
        int pos = line.find("*/");
        if (pos == -1) {
            m_comment += line + "\n";
            return true;  // done
        }
        if (pos > 0) {
            QString text = line.mid(0, pos - 1);
            m_comment += text.stripWhiteSpace();
        }
        m_source.append(m_singleLineCommentIntro + m_comment);  // denotes comments in `m_source'
        m_comment = "";
        m_inComment = false;
        pos++;  // pos now points at the slash in the "*/"
        if (pos == (int)line.length() - 1)
            return true;  // done
        line = line.mid(pos + 1);
    }
    // If we get here then m_inComment is false.
    // Check for start of multi line comment.
    int pos = line.find("/*");
    if (pos != -1) {
        int endpos = line.find("*/");
        if (endpos == -1) {
            m_inComment = true;
            if (pos + 1 < (int)line.length() - 1) {
                QString cmnt = line.mid(pos + 2);
                m_comment += cmnt.stripWhiteSpace() + "\n";
            }
            if (pos == 0)
                return true;  // done
            line = line.left(pos);
        } else {   // It's a multiline comment on a single line.
            if (endpos > pos + 2)  {
                QString cmnt = line.mid(pos + 2, endpos - pos - 2);
                cmnt = cmnt.stripWhiteSpace();
                if (!cmnt.isEmpty())
                    m_source.append(m_singleLineCommentIntro + cmnt);
            }
            endpos++;  // endpos now points at the slash of "*/"
            QString pre;
            if (pos > 0)
                pre = line.left(pos);
            QString post;
            if (endpos < (int)line.length() - 1)
                post = line.mid(endpos + 1);
            line = pre + post;
        }
    }
    return false;  // The input was not completely consumed by preprocessing.
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
    line = line.simplifyWhiteSpace();
    if (line.isEmpty())
        return;
    QStringList words = QStringList::split( QRegExp("\\s+"), line );
    for (QStringList::Iterator it = words.begin(); it != words.end(); ++it) {
        QString word = (*it).stripWhiteSpace();
        if (word.isEmpty())
            continue;
        fillSource(word);
    }
}

void NativeImportBase::initVars() {
}

void NativeImportBase::parseFile(QString filename) {
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
                filename.prepend(path);
                found = true;
                break;
            }
        }
        if (! found) {
            kError() << msgPrefix << "cannot find file" << endl;
            return;
        }
    }
    QFile file(filename);
    if (! file.open(IO_ReadOnly)) {
        kError() << msgPrefix << "cannot open file" << endl;
        return;
    }
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
        m_comment = QString::null;
    }
}

void NativeImportBase::initialize() {
    m_parsedFiles.clear();
}

