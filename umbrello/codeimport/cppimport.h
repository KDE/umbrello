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

#ifndef CPPIMPORT_H
#define CPPIMPORT_H

#include <qstring.h>
#include "classimport.h"

class CppDriver;

/**
 * C++ code import
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class CppImport : public ClassImport {
public:
    CppImport();
    virtual ~CppImport();

protected:
    /**
     * Implement abstract operation from ClassImport for C++.
     */
    void initialize();

    /**
     * Import a single file.
     *
     * @param filename  The file to import.
     */
    void parseFile(const QString& filename);

private:
    /**
    * Auxiliary method for recursively traversing the #include dependencies
    * in order to feed innermost includes to the model before dependent
    * includes.  It is important that includefiles are fed to the model
    * in proper order so that references between UML objects are created
    * properly.
    */
    void feedTheModel(const QString& fileName);

    static CppDriver * ms_driver;
    static QStringList ms_seenFiles;  ///< auxiliary buffer for feedTheModel()

};

#endif
