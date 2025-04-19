/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CSVALAIMPORTBASE_H
#define CSVALAIMPORTBASE_H

#include "javacsvalaimportbase.h"

class UMLObject;

/**
 * Base class for C# and Vala code import.
 * @author Oliver Kellogg
 * @author Andi Fischer (copied from JavaImport)
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class CsValaImportBase : public JavaCsValaImportBase
{
public:
    explicit CsValaImportBase(CodeImpThread *thread = nullptr) : JavaCsValaImportBase(thread) {}
    virtual ~CsValaImportBase() {}
    // The pure virtual functions fileExtension() and spawnImport() are
    // implemented in the deriving classes.

protected:
    bool parseStmt();

    bool preprocess(QString& line);

private:
    bool parseUsingDirectives();
    bool parseGlobalAttributes();
    bool parseNamespaceDeclaration();
    bool parseAnnotation();

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
