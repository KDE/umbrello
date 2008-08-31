/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef JAVAIMPORT_H
#define JAVAIMPORT_H

#include "nativeimportbase.h"
#include "../umlobject.h"

/**
 * Java code import
 * @author Oliver Kellogg
 * @author JP Fournier
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class JavaImport : public NativeImportBase {
public:
    JavaImport();
    virtual ~JavaImport();

protected:

    void initVars();

    bool parseStmt();

    void fillSource(const QString& word);

    void parseFile(const QString& filename);

    UMLObject* resolveClass (QString className);

    void spawnImport(QString file);

    QString joinTypename(QString typeName);

    /**
     * true if the member var or method is declared static
     */
    bool m_isStatic;

    /**
     * The current filename being parsed
     */
    QString m_currentFileName;

    /**
     * the current package of the file being parsed
     */
    QString m_currentPackage;

    /**
     * the imports included in the current file
     */
    QStringList m_imports;

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
     * The current visibility for when the visibility is absent
     */
    Uml::Visibility m_defaultCurrentAccess;


};

#endif

