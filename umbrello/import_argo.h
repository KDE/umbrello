/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef IMPORT_ARGO_H
#define IMPORT_ARGO_H

#include <qiodevice.h>

class UMLPackage;
class KZip;

/**
 * Argo model import
 *
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class Import_Argo
{
public:
    static bool loadFromZArgoFile(QIODevice & file, UMLPackage *parentPkg = 0);

protected:
    static bool loadFromArgoFile(const KZip &zipFile, const QString &fileName);
    static bool loadFromPGMLFile(const KZip &zipFile, const QString &fileName);
    static bool loadFromTodoFile(const KZip &zipFile, const QString &fileName);
    static bool loadFromXMIFile(const KZip &zipFile, const QString &fileName);
};

#endif
