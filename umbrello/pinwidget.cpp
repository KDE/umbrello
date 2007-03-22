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
#include "floatingtextwidget.h"

//Added by qt3to4:
#include <QMouseEvent>
#include <QPolygon>

PinWidget::PinWidget(UMLView * view, UMLWidget* a, Uml::IDType id ): UMLWidget(view, id){
    
    init();
    m_pOw = a;
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
    m_pOw = NULL;
    m_nY = 0;
    setVisible(true);
}

void PinWidget::draw(QPainter & p, int offsetX, int offsetY) { 
    int w = 10;
    int h = 10;
    int width_Activity = m_pOw->getWidth();
    int height_Activity = m_pOw->getHeight();
    int y;
    int x = m_pOw->getX() + (width_Activity/2);

    if ( (offsetY + height_Activity/2) <= m_pOw->getY() + height_Activity){
        y = m_pOw->getY()-5;
    } else if((offsetY + height_Activity/2) > m_pOw->getY() + height_Activity){
       y = (m_pOw->getY() + height_Activity)-5;
    }

    if (offsetX + width_Activity/4 <= m_pOw->getX() + width_Activity/2 
         && (offsetY > m_pOw->getY() +5 && offsetY < m_pOw->getY() + height_Activity - 5) ){
        x = m_pOw->getX() -5;
        y = m_pOw->getY() + (height_Activity/2) -5;
    } else if (offsetX + width_Activity/4 > m_pOw->getX() + width_Activity/2
         && (offsetY > m_pOw->getY() +5 && offsetY < m_pOw->getY() + height_Activity - 5) ){
        x = m_pOw->getX() + width_Activity -5;
        y = m_pOw->getY() + (height_Activity/2) -5;
    }

    setX(x);
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
        //make sure it's always above the other
        setZ(20);
        UMLWidget::setPen(p);
        if(m_bSelected)
             drawSelected(&p, offsetX, offsetY);
}

QSize PinWidget::calculateSize() {
    setSize(10,10);
    return QSize(10,10);
}

void PinWidget::setName(const QString &strName) {
    m_Text = strName;
    updateComponentSize();
    m_pName->setText(m_Text);
}

int PinWidget::getMinY() {
    if (!m_pOw) {
        return 0;
    }
    int heightA = m_pOw->getY() + m_pOw->getHeight();
    return heightA;
}


void PinWidget::slotMenuSelection(int sel) {
    bool done = false;

    bool ok = false;
    QString name = m_Text;

    switch( sel ) {
    case ListPopupMenu::mt_Rename:
        name = KInputDialog::getText( i18n("Enter Pin Name"), i18n("Enter the pin name :"), m_Text, &ok );
        if( ok )
            setName(name);
        done = true;
        break;
    }
    if( !done )
        UMLWidget::slotMenuSelection( sel );
}


void PinWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) { 
    QDomElement PinElement = qDoc.createElement( "pinwidget" );
    PinElement.setAttribute( "widgetaid", ID2STR(m_pOw->getID()) );
    UMLWidget::saveToXMI( qDoc, PinElement ); 
    qElement.appendChild( PinElement ); 
}


bool PinWidget::loadFromXMI( QDomElement & qElement ) { 
    if( !UMLWidget::loadFromXMI( qElement ) )
        return false;
    QString widgetaid = qElement.attribute( "widgetaid", "-1" );

    Uml::IDType aId = STR2ID(widgetaid);
    
    UMLWidget *pWA = m_pView -> findWidget( aId );
    if (pWA == NULL) {
        kDebug() << "PinWidget::loadFromXMI: role A object "
        << ID2STR(aId) << " not found" << endl;
        return false;
    }

    m_pOw = pWA;
  
    return true; 
}


#include "pinwidget.moc"

