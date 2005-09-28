#ifndef ADAIMPORT_H
#define ADAIMPORT_H

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
 * Ada code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */
class AdaImport : public NativeImportBase {
public:
    AdaImport();
    virtual ~AdaImport();

    /**
     * Implement abstract operation from NativeImportBase.
     */
    void parseFile(QString file);

    /**
     * Implement abstract operation from NativeImportBase.
     */
    void fillSource(QString word);

    /**
     * Reimplement operation from NativeImportBase to be a no-op.
     * Ada does not require preprocessing.
     */
    bool preprocess(QString& line);

};

#endif

