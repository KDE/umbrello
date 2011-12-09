/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef ADAIMPORT_H
#define ADAIMPORT_H

#include <QtCore/QMap>
#include <QtCore/QStringList>

#include "nativeimportbase.h"
#include "umlobjectlist.h"

/**
 * Ada code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AdaImport : public NativeImportBase
{
public:
    AdaImport(CodeImpThread* thread = 0);
    virtual ~AdaImport();

protected:

    void initVars();

    bool parseStmt();

    QStringList split(const QString& line);

    void fillSource(const QString& word);

    QString expand(const QString& name);

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

