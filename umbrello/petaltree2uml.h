/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

