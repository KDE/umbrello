/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "pinwidget.h"

// qt includes
#include <qpainter.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>

// app includes
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "umlview.h"
#include "listpopupmenu.h"
#include "dialogs/activitydialog.h"

//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>

PinWidget::PinWidget(UMLView * view, ActivityWidget* a, Uml::IDType id ): UMLWidget(view, id){
    
    init();
    m_pOw[Uml::A] = a;
    int y = getY();
    m_nY = y;
    y = y < getMinY() ? getMinY() : y;
    m_nY = y;
    this->activate();
} 

PinWidget::~PinWidget() {} 
 
void PinWidget::init() {
    UMLWidget::setBaseType(Uml::wt_Pin);
    m_bIgnoreSnapToGrid = true;
    m_bIgnoreSnapComponentSizeToGrid = true;
    m_bResizable =  false ;
    m_pOw[Uml::A] = NULL;
    m_nY = 0;
    setVisible(true);
}

void PinWidget::draw(QPainter & p, int offsetX, int offsetY) { 
    int w = 10;
    int h = 10;
    int width_Activity = m_pOw[Uml::A]->getWidth();
    int height_Activity = m_pOw[Uml::A]->getHeight();
    int y;

    int x = m_pOw[Uml::A]->getX() + (width_Activity/2);
    setX(x);

    if ( (offsetY + height_Activity/2) <= m_pOw[Uml::A]->getY() + height_Activity){
       y = m_pOw[Uml::A]->getY()-5;
    } else if((offsetY + height_Activity/2) > m_pOw[Uml::A]->getY() + height_Activity){
       y = (m_pOw[Uml::A]->getY() + height_Activity)-5;
    }  
    setY(y);


//test if y isn't above the object
//     if ( y <= m_pOw[Uml::A]->getY() + height_Activity-5 && x == m_pOw[Uml::A]->getX() + (width_Activity/2) ) {
// 	y = m_pOw[Uml::A]->getY() + height_Activity + 15;
//     }
//     if (y + h >= m_pOw[Uml::A]->getEndLineY()) {
//         y = m_pOw[Uml::A]->getEndLineY() - h;
//     }
    
    
    UMLWidget::setPen(p); 
        if ( UMLWidget::getUseFillColour() ) { 
            p.setBrush( UMLWidget::getFillColour() ); 
        }
        
        p.drawRect(x,y,w, h); 
        UMLWidget::setPen(p);
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}
 
int PinWidget::getMinY() {
    if (!m_pOw[Uml::A]) {
        return 0;
    }
    int heightA = m_pOw[Uml::A]->getY() + m_pOw[Uml::A]->getHeight();
    int height = heightA;
    return height;
}

// int PinWidget::getMaxY() {
//     if( !m_pOw[Uml::A]) {
//         return 0;
//     }
// 
//     int heightA = (int)((ActivityWidget*)m_pOw[Uml::A])->getEndLineY();
//     int height = heightA;
//     return (height - this->height());
// }

void PinWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) { 
    QDomElement PinElement = qDoc.createElement( "pinwidget" );
    UMLWidget::saveToXMI( qDoc, PinElement ); 
    //PinElement.setAttribute( "pinname", m_Text ); 
   // PinElement.setAttribute( "documentation", m_Doc ); 
    qElement.appendChild( PinElement ); 
}

bool PinWidget::loadFromXMI( QDomElement & qElement ) { 
    if( !UMLWidget::loadFromXMI( qElement ) ) 
        return false; 
    //m_Text = qElement.attribute( "pinname", "" ); 
//kDebug() << "load pinwidget from xmi !!!! " << m_Text << endl;
   // m_Doc = qElement.attribute( "documentation", "" );
    //QString type = qElement.attribute( "objectflowtype", "1" );
  //  setObjectFlowType( (ObjectFlowType)type.toInt() );
    return true; 
}


#include "pinwidget.moc"

