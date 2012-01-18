/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/
#ifndef UMLSCENE_H
#define UMLSCENE_H

#include "umlview.h"

#include <QMouseEvent>
#include <QKeyEvent>

/**
 * UMLScene instances represent diagrams.
 * The UMLScene class inherits from UMLView yet and 
 * in the future from QGraphicsScene.
 */
class UMLScene : public UMLView
{
public:
    UMLScene(UMLFolder *parentFolder) : UMLView(parentFolder) {}
    friend QDebug operator<<(QDebug out, const UMLScene *item);
};

QDebug operator<<(QDebug debug, UMLScene *item);

/// uml related types - makes it easier to switch to QGraphicsScene types
// base types
typedef QPoint UMLScenePoint;
typedef QRect UMLSceneRect;
typedef QSize UMLSceneSize;
typedef int UMLSceneValue;

// event types
typedef QKeyEvent UMLSceneKeyEvent;
typedef QHoverEvent UMLSceneHoverEvent;
typedef QContextMenuEvent UMLSceneContextMenuEvent;
//typedef QDragDropEvent UMLSceneDragDropEvent;

typedef Q3CanvasLine UMLSceneLine;
typedef Q3CanvasRectangle UMLSceneRectangle;
typedef Q3CanvasItem UMLSceneItem;
typedef Q3CanvasPolygon UMLScenePolygon;
typedef Q3CanvasEllipse UMLSceneEllipse;
typedef Q3CanvasItemList UMLSceneItemList;

/**
  qt3 migration wrapper for QMouseEvent
*/ 
class  UMLSceneMouseEvent : public QMouseEvent 
{
public:
    UMLSceneMouseEvent(Type type, const QPoint & position, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers) 
     : QMouseEvent(type, position, button, buttons, modifiers)
    {
    }

    UMLScenePoint scenePos() { return pos(); }
};

#endif
