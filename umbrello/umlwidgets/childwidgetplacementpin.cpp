/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "umlwidgets/childwidgetplacementpin.h"

#include "umlwidgets/umlwidget.h"

ChildWidgetPlacementPin::ChildWidgetPlacementPin(PinPortBase* widget)
 : ChildWidgetPlacement(widget)
{
}

ChildWidgetPlacementPin::~ChildWidgetPlacementPin()
{
}

void ChildWidgetPlacementPin::detectConnectedSide()
{

}

void ChildWidgetPlacementPin::setInitialPosition(const QPointF &scenePos)
{
    Q_UNUSED(scenePos);
    m_connectedSide = Top;
    setPos(0, - height() ); // place above parent
}

void ChildWidgetPlacementPin::setNewPositionWhenMoved(qreal diffX, qreal diffY)
{
    const qreal newX = x() + diffX;
    const qreal newY = y() + diffY;

    UMLWidget* owner = ownerWidget();
    if (isAboveParent() || isBelowParent()) {
        if (newX < 0.0) {
            if (- diffX > width()) {
                jumpToLeftOfParent();
            }
            else {
                setX(0);
            }
        }
        else if (newX > owner->width() - width()) {
            if (diffX > width()) {
                jumpToRightOfParent();
            }
            else {
                setX(owner->width() - width());
            }
        }
        else {
            setX(newX);
        }
    }
    else if (isLeftOfParent() || isRightOfParent()) {
        if (newY < 0.0) {
            if (- diffY > height()) {
                jumpToTopOfParent();
            }
            else {
                setY(0);
            }
        }
        else if (newY > owner->height() - height()) {
            if (diffY > height()) {
                jumpToBottomOfParent();
            }
            else {
                setY(owner->height() - height());
            }
        }
        else {
            setY(newY);
        }
    }
    else {
        // error: client is not attached to parent
        jumpToTopOfParent();
    }
}

void ChildWidgetPlacementPin::setNewPositionOnParentResize()
{
    UMLWidget* owner = ownerWidget();
    if (isRightOfParent()) {
        setPos(owner->width(), qMin(y(), owner->height() - height()));
    }
    else if (isBelowParent()) {
        setPos(qMin(x(), owner->width() - width()), owner->height());
    }
}

bool ChildWidgetPlacementPin::isAboveParent() const
{
    return m_connectedSide == Top;
}

bool ChildWidgetPlacementPin::isBelowParent() const
{
    return m_connectedSide == Bottom;
}

bool ChildWidgetPlacementPin::isLeftOfParent() const
{
    return m_connectedSide == Left;
}

bool ChildWidgetPlacementPin::isRightOfParent() const
{
    return m_connectedSide == Right;
}

qreal ChildWidgetPlacementPin::getNewXOnJumpToTopOrBottom() const
{
    return isLeftOfParent() ? 0 : ownerWidget()->width() - width();
}

void ChildWidgetPlacementPin::jumpToTopOfParent()
{
    setPos(QPointF(getNewXOnJumpToTopOrBottom(), - height()));
    m_connectedSide = Top;
}

void ChildWidgetPlacementPin::jumpToBottomOfParent()
{
    setPos(QPointF(getNewXOnJumpToTopOrBottom(), ownerWidget()->height()));
    m_connectedSide = Bottom;
}

qreal ChildWidgetPlacementPin::getNewYOnJumpToSide() const
{
    return isAboveParent() ? 0 : ownerWidget()->height() - height();
}

void ChildWidgetPlacementPin::jumpToLeftOfParent()
{
    setPos(QPointF(-width(), getNewYOnJumpToSide()));
    m_connectedSide = Left;
}

void ChildWidgetPlacementPin::jumpToRightOfParent()
{
    setPos(QPointF(ownerWidget()->width(), getNewYOnJumpToSide()));
    m_connectedSide = Right;
}

