/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "classimport.h"

// qt/kde includes
#include <QtCore/QRegExp>
#include <klocale.h>

// app includes
#include "debug_utils.h"
#include "umldoc.h"
#include "uml.h"
#include "idlimport.h"
#include "pythonimport.h"
#include "javaimport.h"
#include "adaimport.h"
#include "pascalimport.h"
#ifndef DISABLE_CPP_IMPORT
#include "cppimport.h"
#endif
#include "csharpimport.h"
#include "codeimpthread.h"

/**
 * Factory method.
 * @param fileName  name of imported file
 * @return the class import object
 */
ClassImport *ClassImport::createImporterByFileExt(const QString &fileName, CodeImpThread* thread)
{
    ClassImport *classImporter;
    if (fileName.endsWith(QLatin1String(".idl")))
        classImporter = new IDLImport(thread);
    else if (fileName.contains(QRegExp("\\.pyw?$")))
        classImporter = new PythonImport(thread);
    else if (fileName.endsWith(QLatin1String(".java")))
        classImporter = new JavaImport(thread);
    else if (fileName.contains(QRegExp("\\.ad[sba]$")))
        classImporter = new AdaImport(thread);
    else if (fileName.endsWith(QLatin1String(".pas")))
        classImporter = new PascalImport(thread);
    else if (fileName.endsWith(QLatin1String(".cs")))
        classImporter = new CSharpImport(thread);
#ifndef DISABLE_CPP_IMPORT
    else
        classImporter = new CppImport(thread);  // the default.
#else
    else 
        classImporter = 0;
#endif
    return classImporter;
}

/**
 * Import files.  :TODO: can be deleted
 * @param fileNames  List of files to import.
 */
bool ClassImport::importFiles(const QStringList& fileNames)
{
    initialize();
    UMLDoc *umldoc = UMLApp::app()->document();
    uint processedFilesCount = 0;
    bool result = true;
    foreach (const QString& fileName, fileNames) {
        umldoc->writeToStatusBar(i18n("Importing file: %1 Progress: %2/%3",
                                 fileName, processedFilesCount, fileNames.size()));
        if (!parseFile(fileName))
            result = false;
        processedFilesCount++;
    }
    umldoc->writeToStatusBar(result ? i18nc("ready to status bar", "Ready.") : i18nc("failed to status bar", "Failed."));
    return result;
}

/**
 * Import files.
 * @param files  List of files to import.
 */
bool ClassImport::importFile(const QString& fileName)
{
    initialize();
    return parseFile(fileName);
}

/**
 * Write info to a logger or to the debug output.
 * @param file   the name of the parsed file
 * @param text   the text to write
 */
void ClassImport::log(const QString& file, const QString& text)
{
    if (m_thread) {
        m_thread->emitMessageToLog(file, text);
    }
    else {
        uDebug() << file << " - " << text;
    }
}

/**
 * Write info to a logger or to the debug output.
 * @param text   the text to write
 */
void ClassImport::log(const QString& text)
{
    log("", text);
}
