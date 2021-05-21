/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "classimport.h"

// app includes
#include "debug_utils.h"
#include "folder.h"
#include "uml.h"
#include "umldoc.h"
#include "idlimport.h"
#include "pythonimport.h"
#include "javaimport.h"
#include "adaimport.h"
#include "pascalimport.h"
#include "sqlimport.h"
#include "cppimport.h"
#include "csharpimport.h"
#include "codeimpthread.h"
#ifdef ENABLE_PHP_IMPORT
#include "phpimport.h"
#endif

// kde includes
#include <KLocalizedString>

// qt includes
#include <QRegExp>

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
    else if (fileName.contains(QRegExp(QLatin1String("\\.pyw?$"))))
        classImporter = new PythonImport(thread);
    else if (fileName.endsWith(QLatin1String(".java")))
        classImporter = new JavaImport(thread);
    else if (fileName.contains(QRegExp(QLatin1String("\\.ad[sba]$"))))
        classImporter = new AdaImport(thread);
    else if (fileName.endsWith(QLatin1String(".pas")))
        classImporter = new PascalImport(thread);
    else if (fileName.endsWith(QLatin1String(".cs")))
        classImporter = new CSharpImport(thread);
    else if (fileName.endsWith(QLatin1String(".sql")))
        classImporter = new SQLImport(thread);
#ifdef ENABLE_PHP_IMPORT
    else if (fileName.endsWith(QLatin1String(".php")))
        classImporter = new PHPImport(thread);
#endif
    else
        classImporter = new CppImport(thread);  // the default.
    return classImporter;
}

ClassImport::ClassImport(CodeImpThread* thread)
  : m_thread(thread),
    m_enabled(true)
{
}

ClassImport::~ClassImport()
{
}

/**
 * Do initializations before importing a single file.
 * This is called by importFile() before calling parseFile().
 * @todo check if the default implementation should do anything
 */
void ClassImport::initPerFile()
{
}

/**
 * Import files.
 * @param fileNames  List of files to import.
 */
bool ClassImport::importFiles(const QStringList& fileNames)
{
    initialize();
    UMLDoc *umldoc = UMLApp::app()->document();
    uint processedFilesCount = 0;
    bool result = true;
    umldoc->setLoading(true);
    umldoc->setImporting(true);
    foreach (const QString& fileName, fileNames) {
        umldoc->writeToStatusBar(i18n("Importing file: %1 Progress: %2/%3",
                                 fileName, processedFilesCount, fileNames.size()));
        if (!importFile(fileName))
            result = false;
        processedFilesCount++;
    }
    umldoc->setLoading(false);
    umldoc->setImporting(false);
    umldoc->writeToStatusBar(result ? i18nc("ready to status bar", "Ready.") : i18nc("failed to status bar", "Failed."));
    return result;
}

/**
 * Import a single file.
 * @param fileName  The file to import.
 */
bool ClassImport::importFile(const QString& fileName)
{
    initPerFile();
    return parseFile(fileName);
}

void ClassImport::setRootPath(const QString &path)
{
    m_rootPath = path;
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
    log(QString(), text);
}
