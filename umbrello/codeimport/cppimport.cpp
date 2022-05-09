/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cppimport.h"

#include "lexer.h"
#include "driver.h"
#include "kdevcppparser/cpptree2uml.h"

// app includes
#define DBG_SRC QLatin1String("CppImport")
#include "debug_utils.h"
#include "import_utils.h"
#include "uml.h"
#include "umlobject.h"
#include "package.h"
#include "enum.h"
#include "classifier.h"
#include "operation.h"
#include "attribute.h"
#include "template.h"
#include "association.h"
#include "optionstate.h"

// qt includes
#include <QListWidget>
#include <QMap>

// static members
CppDriver * CppImport::ms_driver;
QStringList CppImport::ms_seenFiles;

class CppDriver : public Driver
{
public:
    void setupLexer(Lexer* lexer)
    {
        Driver::setupLexer(lexer);
        lexer->setRecordComments(true);
    }
};

DEBUG_REGISTER(CppImport)

/**
 * Constructor.
 */
CppImport::CppImport(CodeImpThread* thread)
  : ClassImport(thread)
{
    ms_driver = new CppDriver();
}

/**
 * Destructor.
 */
CppImport::~CppImport()
{
}

/**
 * Auxiliary method for recursively traversing the \#include dependencies
 * in order to feed innermost includes to the model before dependent
 * includes.  It is important that includefiles are fed to the model
 * in proper order so that references between UML objects are created
 * properly.
 * @param fileName   the file to import
 */
void CppImport::feedTheModel(const QString& fileName)
{
    if (ms_seenFiles.indexOf(fileName) != -1)
        return;
    QMap<QString, Dependence> deps = ms_driver->dependences(fileName);
    if (! deps.empty()) {
        QMap<QString, Dependence>::Iterator it;
        for (it = deps.begin(); it != deps.end(); ++it) {
            if (it.value().second == Dep_Global)  // don't want these
                continue;
            QString includeFile = it.key();
            if (includeFile.isEmpty()) {
                logError2("CppImport::feedTheModel(%1) : %2 not found", fileName, it.value().first);
                continue;
            }
            logDebug3("CppImport::feedTheModel(%1): %2 => %3", fileName, includeFile, it.value().first);
            if (ms_seenFiles.indexOf(includeFile) == -1)
                ms_seenFiles.append(includeFile);
            feedTheModel(includeFile);
        }
    }
    ParsedFilePointer ast = ms_driver->translationUnit(fileName);
    if (!ast) {
        logError1("CppImport::feedTheModel: %1 not found in list of parsed files", fileName);
        return;
    }
    ms_seenFiles.append(fileName);
    CppTree2Uml modelFeeder(fileName, m_thread);
    modelFeeder.setRootPath(m_rootPath);
    modelFeeder.parseTranslationUnit(*ast);
}

/**
 * Implement abstract operation from ClassImport for C++.
 */
void CppImport::initialize()
{
    // Reset the driver
    ms_driver->reset();
    ms_driver->setResolveDependencesEnabled(Settings::optionState().codeImportState.resolveDependencies);
    // FIXME: port to win32
    // Add some standard include paths
    ms_driver->addIncludePath(QLatin1String("/usr/include"));
    ms_driver->addIncludePath(QLatin1String("/usr/include/c++"));
    ms_driver->addIncludePath(QLatin1String("/usr/include/g++"));
    ms_driver->addIncludePath(QLatin1String("/usr/local/include"));
    const QStringList incPathList = Import_Utils::includePathList();
    if (incPathList.count()) {
        QStringList::ConstIterator end(incPathList.end());
        for (QStringList::ConstIterator i(incPathList.begin()); i != end; ++i) {
            ms_driver->addIncludePath(*i);
        }
    }
}

/**
 * Reimplement method from ClassImport
 */
void CppImport::initPerFile()
{
    ms_seenFiles.clear();
}

/**
 * Import a single file.
 * @param fileName  The file to import.
 */
bool CppImport::parseFile(const QString& fileName) 
{
    if (ms_seenFiles.indexOf(fileName) != -1)
        return true;
    bool result = ms_driver->parseFile(fileName);
    foreach(const Problem &problem, ms_driver->problems(fileName)) {
        QString level;
        if (problem.level() == Problem::Level_Error)
            level = QLatin1String("error");
        else if (problem.level() == Problem::Level_Warning)
            level = QLatin1String("warning");
        else if (problem.level() == Problem::Level_Todo)
            level = QLatin1String("todo");
        else if (problem.level() == Problem::Level_Fixme)
            level = QLatin1String("fixme");

        QString item = QString::fromLatin1("%1:%2:%3: %4: %5")
                .arg(problem.fileName()).arg(problem.line()+1)
                .arg(problem.column()).arg(level).arg(problem.text());
        UMLApp::app()->log(item);
    }
    if (!result)
        return false;
    feedTheModel(fileName);
    return true;
}
