/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "boxwidget.h"
// qt/kde includes
#include <qevent.h>
#include <kdebug.h>

BoxWidget::BoxWidget(UMLView * view, Uml::IDType id)
        : UMLWidget(view, id) {
    setSize(100,80);
    UMLWidget::setBaseType( Uml::wt_Box );
    WidgetBase::m_bUsesDiagramLineColour = false;  // boxes be black
    WidgetBase::m_LineColour = QColor("black");
    setZ(m_origZ = 0);
}

BoxWidget::~BoxWidget() {
}

void BoxWidget::draw(QPainter& p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    p.drawRect( offsetX, offsetY, width(), height() );

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement boxElement = qDoc.createElement("boxwidget");
    UMLWidget::saveToXMI(qDoc, boxElement);
    qElement.appendChild(boxElement);
}

