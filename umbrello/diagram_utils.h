/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2017-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DIAGRAM_UTILS_H
#define DIAGRAM_UTILS_H

#include "basictypes.h"

#include <QString>

class UMLScene;
class QMimeData;
class QStringList;

/**
 * Utilities for diagrams
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
namespace Diagram_Utils {
    typedef enum { Invalid, GDB, QtCreatorGDB, Simple} SequenceLineFormat;
    SequenceLineFormat detectSequenceLineFormat(const QStringList &lines);
    bool importSequences(const QStringList &lines, UMLScene *scene, const QString &fileName=QString());
    bool importGraph(const QStringList &lines, UMLScene *scene, const QString &fileName=QString());
    bool importGraph(const QString &fileName, UMLScene *scene);
    bool importGraph(const QMimeData *mimeData, UMLScene *scene);
    bool isUniqueDiagramName(Uml::DiagramType::Enum type, QString &name);
} // end namespace Diagram_Utils

#endif
