/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "boxwidget.h"
#include "umlview.h"
#include "umlobject.h"
#include "umldoc.h"
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "uml.h"
#include "diagram.h"

namespace QGV {
  
BoxWidget::BoxWidget(UMLView * view, Uml::IDType id /*Uml::id_None*/)
        : UMLWidget(UMLApp::app()->current_View(), id), m_view(UMLApp::app()->current_View()) {
	
    m_diagram = m_view->diagram();
    setRect(qreal(cursor().pos().x()), qreal(cursor().pos().y()), 100, 80);
    
    setFlag(ItemIsMovable, true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsPanel, true);
    setFlag(ItemIsFocusable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setAcceptDrops(true);
    UMLWidget::setBaseType( WidgetType::Box );
    WidgetBase::m_bUsesDiagramLineColour = false;
    WidgetBase::m_LineColour = QColor("black");
    setZValue(m_origZ = 0);
}

BoxWidget::~BoxWidget()
{

}

void BoxWidget::draw(QPainter& p, int offsetX, int offsetY)
{
    p.setPen(Qt::SolidLine);
    p.drawRect(offsetX, offsetY, 80, 100);

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY);
    }
}

void BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    //QDomElement boxElement = qDoc.createElement("boxwidget");
    //UMLWidget::saveToXMI(qDoc, boxElement);
    //qElement.appendChild(boxElement);
}

 
}
