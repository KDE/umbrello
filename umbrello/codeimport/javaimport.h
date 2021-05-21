/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVAIMPORT_H
#define JAVAIMPORT_H

#include "nativeimportbase.h"

class UMLObject;

/**
 * Java code import
 * @author Oliver Kellogg
 * @author JP Fournier
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class JavaImport : public NativeImportBase
{
public:
    explicit JavaImport(CodeImpThread* thread = 0);
    virtual ~JavaImport();

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

};

#endif

