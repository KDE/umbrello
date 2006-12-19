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
     *
     * @param files  List of files to import.
     */
    void importFiles(const QStringList &files);

    /**
     * Factory method.
     */
    static ClassImport *createImporterByFileExt(const QString &filename);

protected:
    /**
     * Initialize the importer.
     * This is called by importFiles() once, before entering
     * the loop for importing one or more files.
     * To be implemented by inheriting classes.
     */
    virtual void initialize() = 0;

    /**
     * Import a single file.
     * To be implemented by inheriting classes.
     *
     * @param filename  The file to import.
     */
    virtual void parseFile(const QString& filename) = 0;

};

#endif
