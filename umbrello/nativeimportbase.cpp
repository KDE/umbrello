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
    const int srcLength = m_source.count();
    while (m_srcIndex < srcLength && m_source[m_srcIndex] != until)
        m_srcIndex++;
}

void NativeImportBase::importFiles(QStringList fileList) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    for (QStringList::Iterator fileIT = fileList.begin();
            fileIT != fileList.end(); ++fileIT) {
        QString fileName = (*fileIT);
        umldoc->writeToStatusBar(i18n("Importing file: %1").arg(fileName));
        parseFile(fileName);
    }
}

