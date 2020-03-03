/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CHILDWIDGETPLACEMENT_H
#define CHILDWIDGETPLACEMENT_H

class PinPortBase;
class UMLWidget;

#include <QPointF>
#include <QtGlobal>

/**
 * Base class for placing child widgets relative to their parent. Child widget positions
 * shall somehow be constrained, e.g. always in contact to the parent widgets border.
 *
 * @short Base class for child widget placement.
 */
class ChildWidgetPlacement
{
public:
    ChildWidgetPlacement(PinPortBase* widget);
    virtual ~ChildWidgetPlacement() {}

    virtual void detectConnectedSide() = 0;
    virtual void setInitialPosition(const QPointF &scenePos = QPointF()) = 0;
    virtual void setNewPositionWhenMoved(qreal diffX, qreal diffY) = 0;
    virtual void setNewPositionOnParentResize() = 0;

protected:
    enum ConnectedSide {
        Undefined,
        Top,
        Right,
        Bottom,
        Left,
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft
    };

    void setPos(const QPointF& pos);
    void setPos(qreal x, qreal y);
    void setX(qreal x);
    void setY(qreal y);
    qreal x() const;
    qreal y() const;
    qreal width() const;
    qreal height() const;
    UMLWidget* ownerWidget() const;

    PinPortBase* const m_widget;
    ConnectedSide m_connectedSide;
};

#endif /* ! CHILDWIDGETPLACEMENT_H */

