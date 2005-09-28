#ifndef IDLIMPORT_H
#define IDLIMPORT_H

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

#include "nativeimportbase.h"

/**
 * CORBA IDL code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class IDLImport : public NativeImportBase {
public:
    IDLImport();
    virtual ~IDLImport();

    /**
     * Implement abstract operation from NativeImportBase.
     */
    void parseFile(QString file);

    /**
     * Override operation from NativeImportBase.
     */
    bool preprocess(QString& line);

    /**
     * Implement abstract operation from NativeImportBase.
     */
    void fillSource(QString word);

protected:
    QString joinTypename();
    bool m_isOneway, m_isReadonly, m_isAttribute;
};

#endif

