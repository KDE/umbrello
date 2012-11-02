/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATEARROW_H
#define TOOLBARSTATEARROW_H

#include "toolbarstate.h"

class UMLScene;

/**
 * Arrow tool for select, move and resize widgets and associations.
 * Arrow tool delegates the event handling in the widgets and associations. When
 * no widget nor association is being used,the arrow tool acts as a selecting
 * tool that selects all the elements in the rectangle created when dragging the
 * mouse.
 *
 * This is the default tool.
 */
class ToolBarStateArrow : public ToolBarState
{
    Q_OBJECT
public:
    ToolBarStateArrow(UMLScene *umlScene);

    // We just forward events back to UMLScene, which handles the
    // conversion of scene events to widget events as well as selection.
    void mousePress(QGraphicsSceneMouseEvent *ome);
    void mouseMove(QGraphicsSceneMouseEvent *ome);
    void mouseRelease(QGraphicsSceneMouseEvent *ome);
    void mouseDoubleClick(QGraphicsSceneMouseEvent *ome);
};

#endif //TOOLBARSTATEARROW_H
