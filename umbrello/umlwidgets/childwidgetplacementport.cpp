/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "umlwidgets/childwidgetplacementport.h"

#include "umlwidgets/umlwidget.h"

ChildWidgetPlacementPort::ChildWidgetPlacementPort(PinPortBase* widget)
 : ChildWidgetPlacement(widget)
{
}

ChildWidgetPlacementPort::~ChildWidgetPlacementPort()
{
}

void ChildWidgetPlacementPort::setInitialPosition()
{
    m_connectedSide = Top;
    setPos( - width() / 2, - height() / 2); // place on top left corner
}

void ChildWidgetPlacementPort::setNewPositionWhenMoved(qreal diffX, qreal diffY)
{
    qreal newX = x() + diffX;
    qreal newY = y() + diffY;

    UMLWidget* owner = ownerWidget();
    const qreal minX = - width() / 2;
    const qreal maxX = owner->width() - width() / 2;
    const qreal minY = - height() / 2;
    const qreal maxY = owner->height() - height() / 2;

    bool setXToMin = false, setXToMax = false;
    trimToRange(newX, minX, maxX, setXToMin, setXToMax);

    bool setYToMin = false, setYToMax = false;
    trimToRange(newY, minY, maxY, setYToMin, setYToMax);
    switch (m_connectedSide)
    {
        case Top:
        {
            if (setXToMin) {
                m_connectedSide = TopLeft;
            }
            else if (setXToMax) {
                m_connectedSide = TopRight;
            }
            else {
                newY = minY;
            }
        }
        break;

        case Bottom:
        {
            if (setXToMin) {
                m_connectedSide = BottomLeft;
            }
            else if (setXToMax) {
                m_connectedSide = BottomRight;
            }
            else {
                newY = maxY;
            }
        }
        break;

        case Left:
        {
            if (setYToMin) {
                m_connectedSide = TopLeft;
            }
            else if (setYToMax) {
                m_connectedSide = BottomLeft;
            }
            else {
                newX = minX;
            }
        }
        break;

        case Right:
        {
            if (setYToMin) {
                m_connectedSide = TopRight;
            }
            else if (setYToMax) {
                m_connectedSide = BottomRight;
            }
            else {
                newX = maxX;
            }
        }
        break;

        case TopLeft:
        {
            if (newX > minX) {
                m_connectedSide = Top;
                newY = minY;
            }
            else if (newY > minY) {
                m_connectedSide = Left;
                newX = minX;
            }
        }
        break;

        case TopRight:
        {
            if (newX < maxX) {
                m_connectedSide = Top;
                newY = minY;
            }
            else if (newY > minY) {
                m_connectedSide = Right;
                newX = maxX;
            }
        }
        break;

        case BottomRight:
        {
            if (newX < maxX) {
                m_connectedSide = Bottom;
                newY = maxY;
            }
            else if (newY < maxY) {
                m_connectedSide = Right;
                newX = maxX;
            }
        }
        break;

        case BottomLeft:
        {
            if (newX > minX) {
                m_connectedSide = Bottom;
                newY = maxY;
            }
            else if (newY < maxY) {
                m_connectedSide = Left;
                newX = minX;
            }
        }
        break;
    }
    setPos(newX, newY);
}

void ChildWidgetPlacementPort::setNewPositionOnParentResize()
{
    UMLWidget* owner = ownerWidget();
    const qreal maxX = owner->width() - width() / 2;
    const qreal maxY = owner->height() - height() / 2;

    if (m_connectedSide == Right) {
        setPos(maxX, qMin(y(), maxY));
    }
    else if (m_connectedSide == Bottom) {
        setPos(qMin(x(), maxX), maxY);
    }
    else if (m_connectedSide == BottomRight) {
        setPos(maxX, maxY);
    }
}

/**
 * Returns value bound between min and max, and flags whether value has been set.
 */
void ChildWidgetPlacementPort::trimToRange(qreal& value, qreal min, qreal max, bool& setToMin, bool& setToMax) const
{
    if (value < min) {
        setToMin = true;
        value = min;
    }
    else if (value > max) {
        setToMax = true;
        value = max;
    }
}

