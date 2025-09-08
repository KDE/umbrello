/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "javacsvalaimportbase.h"

// app includes
#include "umlattribute.h"
#include "umlclassifier.h"
#include "codeimpthread.h"
#define DBG_SRC QStringLiteral("JavaCsValaImportBase")
#include "debug_utils.h"
#include "umlenum.h"
#include "import_utils.h"
#include "umloperation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QFile>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

DEBUG_REGISTER(JavaCsValaImportBase)

QStringList JavaCsValaImportBase::s_filesAlreadyParsed;
int JavaCsValaImportBase::s_parseDepth = 0;

/**
 * Constructor.
 */
JavaCsValaImportBase::JavaCsValaImportBase(CodeImpThread* thread)
  : NativeImportBase(QStringLiteral("//"), thread),
    m_defaultCurrentAccess(Uml::Visibility::Public)
{
    setMultiLineComment(QStringLiteral("/*"), QStringLiteral("*/"));
    initVars();
}

/**
 * Reimplement operation from NativeImportBase.
 */
void JavaCsValaImportBase::initVars()
{
    m_isStatic = false;
}

/**
 * Figure out if the type is really an array or template of the given typeName.
 * Catenate possible template arguments/array dimensions to the end of the type name.
 * @param typeName   the type name
 * @return the type name with the additional information
 */
QString JavaCsValaImportBase::joinTypename(const QString& typeName)
{
    QString typeNameRet(typeName);
    QString next = lookAhead();
    if (!next.isEmpty()) {
        if (next == QStringLiteral("<") || next == QStringLiteral("[")) {
            int start = ++m_srcIndex;
            if (! skipToClosing(m_source[start][0]))
                return typeNameRet;
            for (int i = start; i <= m_srcIndex; ++i) {
                typeNameRet += m_source[i];
            }
        }
    }
    // to handle multidimensional arrays, call recursively
    if (lookAhead() == QStringLiteral("[")) {
        typeNameRet = joinTypename(typeNameRet);
    }
    return typeNameRet;
}

/**
 * Implement abstract operation from NativeImportBase.
 * @param word   whitespace delimited item
 */
void JavaCsValaImportBase::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        const QChar& c = word[i];
        if (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.')) {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme.clear();
            }
            m_source.append(QString(c));
        }
    }
    if (!lexeme.isEmpty())
        m_source.append(lexeme);

    // Condense single dimension array into the type name as done for the
    // predefined types in the CSharpWriter and ValaWriter code generators.
    const int last = m_source.size() - 1;
    if (last > 1 && m_source.at(last-1) == QStringLiteral("[")
                   && m_source.at(last) == QStringLiteral("]")) {
        m_source.removeLast();
        m_source.removeLast();
        m_source.last() += QStringLiteral("[]");
    }
}

/**
 * Returns the UML Object if found, or null otherwise.
 * @param name        name of the uml object
 * @param parentPkg   parent package
 * @return null or the uml objecct
 */
UMLObject* JavaCsValaImportBase::findObject(const QString& name, UMLPackage *parentPkg)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLObject * o = umldoc->findUMLObject(name, UMLObject::ot_UMLObject, parentPkg);
    return o;
}

/**
 * Try to resolve the specified class the current class depends on.
 * @param className  the name of the class
 */
UMLObject* JavaCsValaImportBase::resolveClass(const QString& className)
{
    UMLObject *existing = findObject(className, currentScope());
    if (existing)
        return existing;
    logDebug1("JavaCsValaImportBase::resolveClass trying to resolve %1", className);
    // keep track if we are dealing with an array
    bool isArray = className.contains(QLatin1Char('['));
    // remove any [] so that the class itself can be resolved
    QString baseClassName = className;
    baseClassName.remove(QLatin1Char('['));
    baseClassName.remove(QLatin1Char(']'));

    // remove template class name so that the class itself can be resolved
    int index = baseClassName.indexOf(QLatin1Char('<'));
    if (index != -1) {
        baseClassName = baseClassName.remove(index, baseClassName.size()-index);
    }

    // java has a few implicit imports.  Most relevant for this is the
    // current package, which is in the same directory as the current file
    // being parsed
    QStringList file = m_currentFileName.split(QLatin1Char('/'));
    // remove the filename.  This leaves the full path to the containing
    // dir which should also include the package hierarchy
    file.pop_back();

    // the file we're looking for might be in the same directory as the
    // current class
    QString myDir = file.join(QStringLiteral("/"));
    QString myFile = myDir + QLatin1Char('/') + baseClassName + fileExtension();
    if (QFile::exists(myFile)) {
        spawnImport(myFile);
        if (isArray) {
            // we have imported the type. For arrays we want to return
            // the array type
            return Import_Utils::createUMLObject(UMLObject::ot_Class, className, currentScope());
        }
        return findObject(baseClassName, currentScope());
    }

    // the class we want is not in the same package as the one being imported.
    // use the imports to find the one we want.
    QStringList package = m_currentPackage.split(QLatin1Char('.'));
    int dirsInPackageCount = package.size();

    // in case the path does not fit into the package hierarchy
    // we cannot check the imports
    if (dirsInPackageCount >= file.size())
        return nullptr;

    for (int count = 0; count < dirsInPackageCount; ++count) {
        // pop off one by one the directories, until only the source root remains
        file.pop_back();
    }
    // this is now the root of any further source imports
    QString sourceRoot = file.join(QStringLiteral("/")) + QLatin1Char('/');

    for (QStringList::Iterator pathIt = m_imports.begin();
            pathIt != m_imports.end(); ++pathIt) {
        QString import = (*pathIt);
        QStringList split = import.split(QLatin1Char('.'));
        split.pop_back(); // remove the * or the classname
        if (import.endsWith(QLatin1Char('*')) || import.endsWith(baseClassName)) {
            // check if the file we want is in this imported package
            // convert the org.test type package into a filename
            QString aFile = sourceRoot + split.join(QStringLiteral("/")) + QLatin1Char('/') + baseClassName + fileExtension();
            if (QFile::exists(aFile)) {
                spawnImport(aFile);
                // we need to set the package for the class that will be resolved
                // start at the root package
                UMLPackage  *parent = nullptr;
                UMLPackage  *current = nullptr;

                for (QStringList::Iterator it = split.begin(); it != split.end(); ++it) {
                    QString name = (*it);
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                                                  name, parent,
                                                                  QString(), QString(),
                                                                  true, false);
                    current = ns->asUMLPackage();
                    parent = current;
                } // for
                if (isArray) {
                    // we have imported the type. For arrays we want to return
                    // the array type
                    return Import_Utils::createUMLObject(UMLObject::ot_Class, className, current,
                                                         QString(), QString(), true, false);
                }
                // now that we have the right package, the class should be findable
                return findObject(baseClassName, current);
            // imported class is specified but seems to be external
            } else if (import.endsWith(baseClassName)) {
                // we need to set the package for the class that will be resolved
                // start at the root package
                UMLPackage  *parent = nullptr;
                UMLPackage  *current = nullptr;

                for (QStringList::Iterator it = split.begin(); it != split.end(); ++it) {
                    QString name = (*it);
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                                                   name, parent);
                    current = ns->asUMLPackage();
                    parent = current;
                } // for
                if (isArray) {
                    // we have imported the type. For arrays we want to return
                    // the array type
                    return Import_Utils::createUMLObject(UMLObject::ot_Class, className, current);
                }
                // now that we have the right package, the class should be findable
                return Import_Utils::createUMLObject(UMLObject::ot_Class,
                                                              baseClassName, current);
            } // if file exists
        } // if import matches
    } //foreach import
    return nullptr; // no match
}

/**
 * Keep track of the current file being parsed and reset the list of imports.
 * @param filename   the name of the file being parsed
 */
bool JavaCsValaImportBase::parseFile(const QString& filename)
{
    logDebug1("JavaCsValaImportBase::parseFile %1", filename);
    m_currentFileName = filename;
    m_imports.clear();
    // default visibility is Impl, unless we are an interface, then it is
    // public for member vars and methods
    m_defaultCurrentAccess = Uml::Visibility::Implementation;
    m_currentAccess = m_defaultCurrentAccess;
    s_parseDepth++;
    // in the case of self referencing types, we can avoid parsing the
    // file twice by adding it to the list
    if (s_filesAlreadyParsed.contains(filename)) {
        s_parseDepth--;
        return true;
    }

    s_filesAlreadyParsed.append(filename);
    NativeImportBase::parseFile(filename);
    s_parseDepth--;
    if (s_parseDepth <= 0) {
        // if the user decides to clear things out and reparse, we need
        // to honor the request, so reset things for next time.
        s_filesAlreadyParsed.clear();
        s_parseDepth = 0;
    }
    return true;
}

