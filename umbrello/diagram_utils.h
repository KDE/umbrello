/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2017                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef DIAGRAM_UTILS_H
#define DIAGRAM_UTILS_H

#include <QString>

class UMLScene;
class QMimeData;
class QStringList;

/**
 * Utilities for diagrams
 * @author Ralf Habacker
 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
namespace Diagram_Utils {
    typedef enum { Invalid, GDB, QtCreatorGDB, Simple} SequenceLineFormat;
    SequenceLineFormat detectSequenceLineFormat(const QStringList &lines);
    bool importSequences(const QStringList &lines, UMLScene *scene, const QString &fileName=QString());
    bool importGraph(const QStringList &lines, UMLScene *scene, const QString &fileName=QString());
    bool importGraph(const QString &fileName, UMLScene *scene);
    bool importGraph(const QMimeData *mimeData, UMLScene *scene);
} // end namespace Diagram_Utils

#endif
