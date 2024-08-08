/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef IMPORT_ROSE__H
#define IMPORT_ROSE__H

#include <QFile>

class UMLPackage;

/**
 * Rose model import
 *
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Import_Rose {

    UMLPackage *loadFromMDL(QFile & file, UMLPackage *parentPkg = nullptr);

    QString mdlPath();

}

#endif
