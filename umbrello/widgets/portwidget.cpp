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
#include "debug_utils.h"
#include "umldoc.h"
#include "umlscene.h"
#include "umlview.h"

// qt includes
#include <QPainter>

const QSizeF PortWidget::FixedSize(30, 20);

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

