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
typedef QLine UMLSceneLine;
typedef int UMLSceneValue;

// event types
typedef QKeyEvent UMLSceneKeyEvent;
typedef QHoverEvent UMLSceneHoverEvent;
typedef QContextMenuEvent UMLSceneContextMenuEvent;
//typedef QDragDropEvent UMLSceneDragDropEvent;

typedef Q3CanvasItem UMLSceneItem;
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

class UMLSceneLineItem : public Q3CanvasLine
{
public:
    UMLSceneLineItem()
      : Q3CanvasLine(0)
    {
    }
};

class UMLSceneRectItem : public Q3CanvasRectangle
{
public:
    UMLSceneRectItem()
      : Q3CanvasRectangle(0)
    {
    }

    UMLSceneRectItem(int x, int y, int w, int h)
      : Q3CanvasRectangle(x, y, w, h, 0)
    {
    }
};

class UMLScenePolygonItem : public Q3CanvasPolygon
{
public:
    UMLScenePolygonItem()
      : Q3CanvasPolygon(0)
    {
    }
};

class UMLSceneEllipseItem : public Q3CanvasEllipse
{
public:
    UMLSceneEllipseItem()
    : Q3CanvasEllipse(0)
    {
    }

    UMLSceneEllipseItem(int width, int height)
    : Q3CanvasEllipse(width, height, 0)
    {
    }
};

#endif
