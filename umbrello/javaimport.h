#ifndef JAVAIMPORT_H
#define JAVAIMPORT_H

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

#include "nativeimportbase.h"

/**
 * Java code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class JavaImport : public NativeImportBase {
public:
    JavaImport();
    virtual ~JavaImport();

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
     * Implement abstract operation from NativeImportBase.
     */
    void fillSource(QString word);

    QString joinTypename();
    bool m_isStatic;
};

#endif

