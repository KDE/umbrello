/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CSHARPIMPORT_H
#define CSHARPIMPORT_H

#include "nativeimportbase.h"

class UMLObject;

/**
 * C# code import.
 * Grammar of C# can be found at http://msdn.microsoft.com/en-us/library/Aa664812 .
 * @author Andi Fischer (copied from JavaImport)
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class CSharpImport : public NativeImportBase
{
public:
    explicit CSharpImport(CodeImpThread* thread = 0);
    virtual ~CSharpImport();

protected:
    void initVars();

    bool parseStmt();

    void fillSource(const QString& word);

    bool parseFile(const QString& filename);

    UMLObject* resolveClass (const QString& className);

    void spawnImport(const QString& file);

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

private:
    static UMLObject* findObject(const QString& name, UMLPackage *parentPkg);

    bool parseUsingDirectives();
    bool parseGlobalAttributes();
    bool parseNamespaceMemberDeclarations();
    bool parseAttributes();

    // type-declaration
    bool parseClassDeclaration(const QString& keyword);
    bool parseStructDeclaration();
    // bool parseInterfaceDeclaration();
    bool parseEnumDeclaration();
    bool parseDelegateDeclaration();

    bool isTypeDeclaration(const QString& keyword);
    bool isClassModifier(const QString& keyword);
    bool isCommonModifier(const QString& keyword);

};

#endif
