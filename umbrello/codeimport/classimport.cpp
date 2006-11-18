/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "classimport.h"
// qt/kde includes
#include <qregexp.h>
#include <klocale.h>
// app includes
#include "../umldoc.h"
#include "../uml.h"
#include "idlimport.h"
#include "pythonimport.h"
#include "javaimport.h"
#include "adaimport.h"
#include "pascalimport.h"
#include "cppimport.h"

void ClassImport::importFiles(const QStringList &fileList) {
    initialize();
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    uint processedFilesCount = 0;
    for (QStringList::const_iterator fileIT = fileList.begin();
            fileIT != fileList.end(); ++fileIT) {
        QString fileName = (*fileIT);
        umldoc->writeToStatusBar(i18n("Importing file: %1 Progress: %2/%3").
                                 arg(fileName).arg(processedFilesCount).arg(fileList.size()));
        parseFile(fileName);
        processedFilesCount++;
    }
    umldoc->writeToStatusBar(i18n("Ready."));
}

ClassImport *ClassImport::createImporterByFileExt(const QString &filename) {
    ClassImport *classImporter;
    if (filename.endsWith(".idl"))
        classImporter = new IDLImport();
    else if (filename.endsWith(".py"))
        classImporter = new PythonImport();
    else if (filename.endsWith(".java"))
        classImporter = new JavaImport();
    else if (filename.contains( QRegExp("\\.ad[sba]$") ))
        classImporter = new AdaImport();
    else if (filename.endsWith(".pas"))
        classImporter = new PascalImport();
    else
        classImporter = new CppImport();  // the default.
    return classImporter;
}

