/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef ADAIMPORT_H
#define ADAIMPORT_H

#include "nativeimportbase.h"

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

    bool m_inGenericFormalPart; ///< auxiliary variable
};

#endif

