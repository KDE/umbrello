/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                  *
 ***************************************************************************/

// own header
#include "boxwidget.h"

/**
 * Constructs a BoxWidget.
 *
 * @param scene   The parent to this widget.
 * @param id      The ID to assign (-1 will prompt a new ID.)
 */
BoxWidget::BoxWidget(UMLScene * scene, Uml::ID::Type id, WidgetType type)
  : UMLWidget(scene, type, id)
{
    setSize(100,80);
    m_usesDiagramLineColor = false;  // boxes be black
    m_lineColor = QColor("black");
    setZValue(-10);
}

/**
 * Destructor.
 */
BoxWidget::~BoxWidget()
{
}

/**
 * Draws a rectangle.
 */
void BoxWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    setPenFromSettings(painter);
    painter->drawRect(0, 0, width(), height());

    if (m_selected) {
        paintSelected(painter);
    }
}

/**
 * Saves the widget to the "boxwidget" XMI element.
 * Note: For loading from XMI, the inherited parent method is used.
 */
void BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement boxElement = qDoc.createElement("boxwidget");
    UMLWidget::saveToXMI(qDoc, boxElement);
    qElement.appendChild(boxElement);
}
