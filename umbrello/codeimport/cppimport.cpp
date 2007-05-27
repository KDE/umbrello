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
#include "cppimport.h"
// qt/kde includes
#include <qmap.h>
#include <qregexp.h>
#include <kmessagebox.h>
#include <kdebug.h>
// app includes
#include "import_utils.h"
#include "../umlobject.h"
#include "../docwindow.h"
#include "../package.h"
#include "../enum.h"
#include "../classifier.h"
#include "../operation.h"
#include "../attribute.h"
#include "../template.h"
#include "../association.h"
#include "kdevcppparser/lexer.h"
#include "kdevcppparser/driver.h"
#include "kdevcppparser/cpptree2uml.h"

// static members
CppDriver * CppImport::ms_driver;
QStringList CppImport::ms_seenFiles;

class CppDriver : public Driver {
public:
    void setupLexer(Lexer* lexer) {
        Driver::setupLexer(lexer);
        lexer->setRecordComments(true);
    }
};

CppImport::CppImport() {
    ms_driver = new CppDriver();
}

CppImport::~CppImport() {}

void CppImport::feedTheModel(const QString& fileName) {
    if (ms_seenFiles.find(fileName) != ms_seenFiles.end())
        return;
    ms_seenFiles.append(fileName);
    QMap<QString, Dependence> deps = ms_driver->dependences(fileName);
    if (! deps.empty()) {
        QMap<QString, Dependence>::Iterator it;
        for (it = deps.begin(); it != deps.end(); ++it) {
            if (it.data().second == Dep_Global)  // don't want these
                continue;
            QString includeFile = it.key();
            if (includeFile.isEmpty()) {
                kError() << fileName << ": " << it.data().first
                << " not found" << endl;
                continue;
            }
            kDebug() << fileName << ": " << includeFile << " => " << it.data().first << endl;
            if (ms_seenFiles.find(includeFile) == ms_seenFiles.end())
                feedTheModel(includeFile);
        }
    }
    TranslationUnitAST *ast = ms_driver->translationUnit( fileName );
    if (ast == NULL) {
        kError() << "CppImport::feedTheModel: " << fileName << " not found" << endl;
        return;
    }
    CppTree2Uml modelFeeder( fileName );
    modelFeeder.parseTranslationUnit( ast );
}

void CppImport::initialize() {
    // Reset the driver
    ms_driver->reset();
    // The driver shall attempt to parse included files.
    ms_driver->setResolveDependencesEnabled( true );
    // Add some standard include paths
    ms_driver->addIncludePath( "/usr/include" );
    ms_driver->addIncludePath( "/usr/include/c++" );
    ms_driver->addIncludePath( "/usr/include/g++" );
    ms_driver->addIncludePath( "/usr/local/include" );
    QStringList incPathList = Import_Utils::includePathList();
    if (incPathList.count()) {
        QStringList::Iterator end(incPathList.end());
        for (QStringList::Iterator i(incPathList.begin()); i != end; ++i) {
            ms_driver->addIncludePath( *i );
        }

    }
    ms_seenFiles.clear();
}

void CppImport::parseFile(const QString& fileName) {
    if (ms_seenFiles.find(fileName) != ms_seenFiles.end())
        return;
    ms_driver->parseFile( fileName );
    feedTheModel(fileName);
}

