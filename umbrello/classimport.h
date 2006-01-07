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

#ifndef CLASSIMPORT_H
#define CLASSIMPORT_H

#include <qstringlist.h>

/**
 * Interfaces classparser library to uml models
 * Abstract base for programming language specific import classes
 * @author Mikko Pasanen
 * @author Oliver Kellogg
 * Bugs and comments to uml-devel@lists.sf.net or http://bugs.kde.org
 */

class ClassImport {
public:
    ClassImport() {}
    virtual ~ClassImport() {}

    /**
     * Import files.
     * To be provided by the programming language specific code import
     * implementation class.
     *
     * @param files  List of files to import.
     */
    virtual void importFiles(QStringList files) = 0;

    /**
     * Factory method.
     */
    static ClassImport *createImporterByFileExt(QString filename);

};

#endif
