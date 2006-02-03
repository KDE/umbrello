/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "classimport.h"
// qt/kde includes
#include <qregexp.h>
// app includes
#include "idlimport.h"
#include "pythonimport.h"
#include "javaimport.h"
#include "adaimport.h"
#include "cppimport.h"

ClassImport *ClassImport::createImporterByFileExt(QString filename) {
    ClassImport *classImporter;
    if (filename.endsWith(".idl"))
        classImporter = new IDLImport();
    else if (filename.endsWith(".py"))
        classImporter = new PythonImport();
    else if (filename.endsWith(".java"))
        classImporter = new JavaImport();
    else if (filename.contains( QRegExp("\\.ad[sba]$") ))
        classImporter = new AdaImport();
    else
        classImporter = new CppImport();  // the default.
    return classImporter;
}

