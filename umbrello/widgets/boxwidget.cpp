/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "boxwidget.h"

#include <QtGui/QPainter>

/**
 * Constructs a BoxWidget.
 * @param id The ID to assign (-1 will prompt a new ID.)
 */
BoxWidget::BoxWidget(Uml::IDType id)
    : UMLWidget(0)
{
    m_baseType = Uml::wt_Box;
    setID(id);
    setMinimumSize(QSizeF(20, 20));
}

/// destructor
BoxWidget::~BoxWidget()
{
}

/// Draws a rectangle.
void BoxWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
    Q_UNUSED(o);
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect());
}

/**
 * Saves the widget to the "boxwidget" XMI element.
 * @note For loading from XMI, the inherited parent method is used.
 */
void BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement boxElement = qDoc.createElement("boxwidget");
    UMLWidget::saveToXMI(qDoc, boxElement);
    qElement.appendChild(boxElement);
}
