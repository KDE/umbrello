/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "boxwidget.h"

/**
 * Constructs a BoxWidget.
 *
 * @param scene              The parent to this widget.
 * @param id                The ID to assign (-1 will prompt a new ID.)
 */
BoxWidget::BoxWidget(UMLScene * scene, Uml::IDType id, WidgetType type)
  : UMLWidget(scene, type, id)
{
    setSize(100,80);
    m_usesDiagramLineColor = false;  // boxes be black
    m_LineColor = QColor("black");
    setZ(m_origZ = 0);
}

/**
 * destructor
 */
BoxWidget::~BoxWidget()
{
}

/**
 * Draws a rectangle.
 */
void BoxWidget::paint(QPainter& p, int offsetX, int offsetY)
{
    UMLWidget::setPenFromSettings(p);
    p.drawRect( offsetX, offsetY, width(), height() );

    if (m_selected) {
        drawSelected(&p, offsetX, offsetY);
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
