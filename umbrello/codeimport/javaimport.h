/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006-2009                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef JAVAIMPORT_H
#define JAVAIMPORT_H

#include "nativeimportbase.h"

class UMLObject;

/**
 * Java code import
 * @author Oliver Kellogg
 * @author JP Fournier
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class JavaImport : public NativeImportBase
{
public:
    JavaImport(CodeImpThread* thread = 0);
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
    Uml::Visibility m_defaultCurrentAccess;  ///< current visibility for when the visibility is absent

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

