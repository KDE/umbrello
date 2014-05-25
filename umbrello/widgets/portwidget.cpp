/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2014                                                    *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "portwidget.h"

// app includes
#include "port.h"
#include "package.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QPainter>

// sys includes
#include <cmath>

const QSizeF PortWidget::FixedSize(15, 15);

/**
 * Constructs a PortWidget.
 *
 * @param scene   The parent of this PortWidget.
 * @param d       The UMLPort this will be representing.
 */
PortWidget::PortWidget(UMLScene *scene, UMLPort *d) 
  : UMLWidget(scene, WidgetBase::wt_Port, d)
{
    m_ignoreSnapToGrid = true;
    m_ignoreSnapComponentSizeToGrid = true;
    m_resizable = false;
    setMinimumSize(FixedSize);
    setMaximumSize(FixedSize);
    setSize(FixedSize);
    setToolTip(d->name());
}

/**
 * Standard deconstructor.
 */
PortWidget::~PortWidget()
{
}

/**
 * Overrides method from UMLWidget in order to set a tooltip.
 * The tooltip is set to the port name.
 * The reason for using a tooltip for the name is that the size of the port
 * widget is not large enough to accomodate the average name.
 */
void PortWidget::updateWidget()
{
    QString strName = m_umlObject->name();
    uDebug() << " port name is " << strName;
    setToolTip(strName);
}

/**
 * Overridden from UMLWidget.
 * Moves the widget to a new position using the difference between the
 * current position and the new position.
 * Movement is constrained such that the port is always attached to its
 * component.
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void PortWidget::moveWidgetBy(qreal diffX, qreal diffY)
{
    UMLWidget* owner = m_scene->widgetOnDiagram(m_umlObject->umlPackage()->id());
    qreal newX = x() + diffX;
    qreal newY = y() + diffY;
    if (owner == NULL) {
        uError() << "m_scene->widgetOnDiagram returns NULL for owner";
        setX(newX);
        setY(newY);
        return;
    }
    const qreal deltaTop    = fabs(y() + height() - owner->y());
    const qreal deltaBottom = fabs(owner->y() + owner->height() - y());
    const qreal deltaLeft   = fabs(x() + width() - owner->x());
    const qreal deltaRight  = fabs(owner->x() + owner->width() - x());
    bool didAnyMovement = false;
    if (deltaTop < 1.0 || deltaBottom < 1.0) {
        if (newX < owner->x() - width())
            newX = owner->x() - width();
        else if (newX > owner->x() + owner->width())
            newX = owner->x() + owner->width();
        setX(newX);
        didAnyMovement = true;
    }
    if (deltaLeft < 1.0 || deltaRight < 1.0) {
        if (newY < owner->y() - height())
            newY = owner->y() - height();
        else if (newY > owner->y() + owner->height())
            newY = owner->y() + owner->height();
        setY(newY);
        didAnyMovement = true;
    }
    if (!didAnyMovement) {
        uDebug() << "constraint failed for (" << diffX << ", " << diffY << ")";
        setX(newX);
        setY(newY);
    }
}

/**
 * Align this PortWidget's position such that it is attached at one of the
 * sides of its owner's widget.
 */
void PortWidget::attachToOwningComponent() {
    UMLWidget *owner = m_scene->widgetOnDiagram(m_umlObject->umlPackage()->id());
    const QPointF scenePos = m_scene->pos();
    if (owner) {
        if (scenePos.x() < owner->x() - width())
            setX(owner->x() - width());
        else if (scenePos.x() <= owner->x() + owner->width())
            setX(scenePos.x());
        else
            setX(owner->x() + owner->width());
        if (scenePos.y() < owner->y() - height())
            setY(owner->y() - height());
        else if (scenePos.y() <= owner->y() + owner->height())
            setY(scenePos.y());
        else
            setY(owner->y() + owner->height());
    } else {
        uError() << "port : widgetOnDiagram(umlObject) returns NULL";
        setX(scenePos.x());
        setY(scenePos.y());
    }
}

/**
 * Overrides standard method.
 */
void PortWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    setPenFromSettings(painter);
    if (UMLWidget::useFillColor())  {
        painter->setBrush(UMLWidget::fillColor());
    } else {
        painter->setBrush(m_scene->activeView()->viewport()->palette().color(QPalette::Background));
    }

    int w = width();
    int h = height();

    painter->drawRect(0, 0, w, h);

    UMLWidget::paint(painter, option, widget);
}

/**
 * Loads from a "portwidget" XMI element.
 */
bool PortWidget::loadFromXMI(QDomElement & qElement)
{
    return UMLWidget::loadFromXMI(qElement);
}

/**
 * Saves to the "portwidget" XMI element.
 */
void PortWidget::saveToXMI(QDomDocument & qDoc, QDomElement & qElement)
{
    QDomElement conceptElement = qDoc.createElement("portwidget");
    UMLWidget::saveToXMI(qDoc, conceptElement);
    qElement.appendChild(conceptElement);
}

