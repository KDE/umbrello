/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef PETALTREE2UML_H
#define PETALTREE2UML_H

#include <QString>

// fwd decl
class PetalNode;
class UMLPackage;

/**
 * Traverse the PetalNode tree and create corresponding Umbrello objects
 * for the PetalNodes encountered.
 *
 * @author Oliver Kellogg
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Import_Rose {

    bool importView(PetalNode *root,
                    UMLPackage *parent,
                    const QString& rootName,
                    const QString& modelsName,
                    const QString& firstNodeName,
                    const QString& presentationsName = QString());

    UMLPackage* petalTree2Uml(PetalNode *root, UMLPackage *parentPkg);

}

#endif

