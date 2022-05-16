/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlwidgets/childwidgetplacementport.h"

#include "umlwidgets/umlwidget.h"
#include "pinportbase.h"

ChildWidgetPlacementPort::ChildWidgetPlacementPort(PinPortBase* widget)
 : ChildWidgetPlacement(widget)
{
}

ChildWidgetPlacementPort::~ChildWidgetPlacementPort()
{
}

void ChildWidgetPlacementPort::setInitialPosition(const QPointF &scenePos)
{
#if 0
    QPointF p = ownerWidget()->mapFromScene(scenePos);
    p -= QPointF(width()/2, height()/2);
    setPos(p);
#else
    if (ownerWidget()) {
        QPointF p = ownerWidget()->mapFromScene(scenePos);
        p -= QPointF(width()/2, height()/2);
        setPos(p);

        detectConnectedSide();
        setNewPositionWhenMoved(0.0, 0.0);
        //setNewPositionOnParentResize();
    } else {
        m_connectedSide = TopLeft;
        setPos(minX(), minY());
    }
#endif
}

void ChildWidgetPlacementPort::setNewPositionWhenMoved(qreal diffX, qreal diffY)
{
    bool setXToMin = false, setXToMax = false;
    qreal newX = trimToRange(x() + diffX, minX(), maxX(), setXToMin, setXToMax);

    bool setYToMin = false, setYToMax = false;
    qreal newY = trimToRange(y() + diffY, minY(), maxY(), setYToMin, setYToMax);

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
                newY = minY();
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
                newY = maxY();
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
                newX = minX();
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
                newX = maxX();
            }
        }
        break;

        case TopLeft:
        {
            if (newX > minX()) {
                m_connectedSide = Top;
                newY = minY();
            }
            else if (newY > minY()) {
                m_connectedSide = Left;
                newX = minX();
            }
        }
        break;

        case TopRight:
        {
            if (newX < maxX()) {
                m_connectedSide = Top;
                newY = minY();
            }
            else if (newY > minY()) {
                m_connectedSide = Right;
                newX = maxX();
            }
        }
        break;

        case BottomRight:
        {
            if (newX < maxX()) {
                m_connectedSide = Bottom;
                newY = maxY();
            }
            else if (newY < maxY()) {
                m_connectedSide = Right;
                newX = maxX();
            }
        }
        break;

        case BottomLeft:
        {
            if (newX > minX()) {
                m_connectedSide = Bottom;
                newY = maxY();
            }
            else if (newY < maxY()) {
                m_connectedSide = Left;
                newX = minX();
            }
        }
        break;

        default:
        break;
    }
    setPos(newX, newY);
}

void ChildWidgetPlacementPort::detectConnectedSide()
{
    if (m_widget->x() < 0) {
        if (m_widget->y() < 0)
            m_connectedSide = TopLeft;
        else if (m_widget->y() < maxY())
            m_connectedSide = Left;
        else
            m_connectedSide = BottomLeft;
    } else if (m_widget->x() < maxX()) {
        if (m_widget->y() < 0)
            m_connectedSide = Top;
        else if (m_widget->y() < maxY())
            m_connectedSide = Undefined;
        else
            m_connectedSide = Bottom;
    } else if (m_widget->x() >= maxX()) {
        if (m_widget->y() < 0)
            m_connectedSide = TopRight;
        else if (m_widget->y() < maxY())
            m_connectedSide = Right;
        else
            m_connectedSide = BottomRight;
    } else {
        m_connectedSide = TopLeft;
    }
}

void ChildWidgetPlacementPort::setNewPositionOnParentResize()
{
    switch (m_connectedSide)
    {
        case Right:
        {
            setPos(maxX(), qMin(y(), maxY()));
        }
        break;

        case Bottom:
        {
            setPos(qMin(x(), maxX()), maxY());
        }
        break;

        case TopRight:
        {
            setPos(maxX(), minY());
        }
        break;

        case BottomRight:
        {
            setPos(maxX(), maxY());
        }
        break;

        case BottomLeft:
        {
            setPos(minX(), maxY());
        }
        break;

        default:
            ; // nothing to do
    }
}

/**
 * Returns value bound between min and max, and flags whether value has been set.
 */
qreal ChildWidgetPlacementPort::trimToRange(qreal value, qreal min, qreal max, bool& setToMin, bool& setToMax) const
{
    if (value < min) {
        setToMin = true;
        return min;
    }
    else if (value > max) {
        setToMax = true;
        return max;
    }
    return value;
}

/**
 * Returns minimum allowed x value.
 */
qreal ChildWidgetPlacementPort::minX() const
{
    return - width() / 2;
}

/**
 * Returns maximum allowed x value.
 */
qreal ChildWidgetPlacementPort::maxX() const
{
    UMLWidget* owner = ownerWidget();
    return owner->width() - width() / 2;
}

/**
 * Returns minimum allowed y value.
 */
qreal ChildWidgetPlacementPort::minY() const
{
    return - height() / 2;
}

/**
 * Returns maximum allowed y value.
 */
qreal ChildWidgetPlacementPort::maxY() const
{
    UMLWidget* owner = ownerWidget();
    return owner->height() - height() / 2;
}

