/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ADAIMPORT_H
#define ADAIMPORT_H

#include <qmap.h>
#include <qstringlist.h>

#include "nativeimportbase.h"
#include "../umlobjectlist.h"

/**
 * Ada code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AdaImport : public NativeImportBase {
public:
    AdaImport();
    virtual ~AdaImport();

protected:
    /**
     * Reimplement operation from NativeImportBase.
     */
    void initVars();

    /**
     * Implement abstract operation from NativeImportBase.
     */
    bool parseStmt();

    /**
     * Split the line so that a string is returned as a single element of the list.
     * When not in a string then split at white space.
     * Reimplementation of method from NativeImportBase is required because of
     * Ada's tic which is liable to be confused with the beginning of a character
     * constant.
     */
    QStringList split(const QString& line);

    /**
     * Implement abstract operation from NativeImportBase.
     */
    void fillSource(const QString& word);

    /**
     * Apply package renamings to the given name.
     *
     * @return  expanded name
     */
    QString expand(const QString& name);

    /**
     * Parse all files that can be formed by concatenation of the given stems.
     */
    void parseStems(const QStringList& stems);

    bool m_inGenericFormalPart; ///< auxiliary variable

    /**
     * List for keeping track of tagged objects declared in the current scope.
     * This is required for distinguishing primitive from non primitive
     * methods.
     */
    UMLObjectList m_classesDefinedInThisScope;

    typedef QMap<QString, QString> StringMap;

    /**
     * Map of package renamings.
     * Keyed by the renaming. Value returns the expanded name.
     */
    StringMap m_renaming;

};

#endif

