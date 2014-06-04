/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef IMPORT_ROSE__H
#define IMPORT_ROSE__H

#include <QFile>

class UMLPackage;

/**
 * Rose model import
 *
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
namespace Import_Rose {

    bool loadFromMDL(QFile & file, UMLPackage *parentPkg = 0);

    QString mdlPath();

}

#endif
