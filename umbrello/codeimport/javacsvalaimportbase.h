/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2025 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACSVALAIMPORTBASE_H
#define JAVACSVALAIMPORTBASE_H

#include "nativeimportbase.h"

class UMLObject;

/**
 * Base class for Java, C# and Vala code import.
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class JavaCsValaImportBase : public NativeImportBase
{
public:
    explicit JavaCsValaImportBase(CodeImpThread *thread = nullptr);
    virtual ~JavaCsValaImportBase() {}
    virtual QString fileExtension() = 0;

protected:
    void initVars();

    // bool parseStmt();  done in the specific importer realizations

    void fillSource(const QString& word);

    bool parseFile(const QString& filename);

    virtual void spawnImport(const QString& file) = 0;

    UMLObject* resolveClass (const QString& className);

    QString joinTypename(const QString& typeName);

    bool        m_isStatic;         ///< static flag for the member var or method
    QString     m_currentFileName;  ///< current filename being parsed
    QString     m_currentPackage;   ///< current package of the file being parsed
    QStringList m_imports;          ///< imports included in the current file
    Uml::Visibility::Enum m_defaultCurrentAccess;  ///< current visibility for when the visibility is absent

    /**
     * Keep track of the files we have already parsed so we don't
     * reparse the same ones over and over again.
     */
    static QStringList s_filesAlreadyParsed;

    /**
     * Keep track of the parses so that the filesAlreadyParsed
     * can be reset when we're done.
     */
    static int s_parseDepth;

    /**
     * required for language specific suport
     */
    Uml::ProgrammingLanguage::Enum m_language;

    static UMLObject* findObject(const QString& name, UMLPackage *parentPkg);

    // type-declaration
    bool parseClassDeclaration(const QString& keyword);
    bool parseEnumDeclaration();

};

#endif
