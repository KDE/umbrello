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

BoxWidget::BoxWidget(Uml::IDType id)
    : UMLWidget(0)
{
    m_baseType = WidgetBase::wt_Box;
    setID(id);
    setMinimumSize(QSizeF(20, 20));
    setZValue(-10);
}

BoxWidget::~BoxWidget()
{
}

void BoxWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *)
{
    Q_UNUSED(o);
    painter->setPen(QPen(lineColor(), lineWidth()));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect());
}

void BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement boxElement = qDoc.createElement("boxwidget");
    UMLWidget::saveToXMI(qDoc, boxElement);
    qElement.appendChild(boxElement);
}
