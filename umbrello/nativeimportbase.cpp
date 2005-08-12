/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
 *                                                                         *
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "nativeimportbase.h"

// qt/kde includes
#include <qregexp.h>
#include <klocale.h>
#include <kdebug.h>
// app includes
#include "import_utils.h"
#include "uml.h"
#include "umldoc.h"

NativeImportBase::NativeImportBase(QString singleLineCommentIntro) {
    m_singleLineCommentIntro = singleLineCommentIntro;
    m_srcIndex = 0;
    m_scopeIndex = 0;  // index 0 is reserved for global scope
    m_klass = NULL;
    m_currentAccess = Uml::Public;
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
    }
    if (m_source[m_srcIndex].startsWith(m_singleLineCommentIntro)) {
        // last item in m_source is a comment
        return QString::null;
    }
    return m_source[m_srcIndex];
}

bool NativeImportBase::preprocess(QString&) {
    // The default is that no preprocessing is needed.
    return false;  // The return value indicates that we are not done yet.
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

void NativeImportBase::importFiles(QStringList fileList) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    for (QStringList::Iterator fileIT = fileList.begin();
            fileIT != fileList.end(); ++fileIT) {
        QString fileName = (*fileIT);
        umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
        m_source.clear();
        m_srcIndex = 0;
        parseFile(fileName);
    }
}

