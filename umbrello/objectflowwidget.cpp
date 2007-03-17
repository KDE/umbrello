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
#include "objectflowwidget.h"

// qt includes
#include <qpainter.h>

// kde includes
#include <klocale.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <kdialog.h>
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

ObjectFlowWidget::ObjectFlowWidget(UMLView * view, Uml::IDType id ): UMLWidget(view, id)
{
    UMLWidget::setBaseType( Uml::wt_Object_Flow ); 
    updateComponentSize(); 
} 

ObjectFlowWidget::~ObjectFlowWidget() {} 
 
void ObjectFlowWidget::draw(QPainter & p, int offsetX, int offsetY) { 
    int w = width(); 
    int h = height();
   
    UMLWidget::setPen(p); 
        if ( UMLWidget::getUseFillColour() ) { 
            p.setBrush( UMLWidget::getFillColour() ); 
        }
        { 
            const QFontMetrics &fm = getFontMetrics(FT_NORMAL); 
            const int fontHeight  = fm.lineSpacing(); 
            QString objectflow_value;
            if(getState() == "-")
            {
            	objectflow_value = " ";
            }
            else
            {
            	objectflow_value = "[" + getState() + "]";
            }
            
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRect(offsetX, offsetY, w, h); 
            p.drawLine(offsetX + 10, offsetY + (h/2) +3 , (offsetX + w)-10, offsetY + (h/2) +3 );
            p.setPen(Qt::black);
            p.setFont( UMLWidget::getFont() ); 
            p.drawText(offsetX + OBJECTFLOW_MARGIN, (offsetY + textStartY/2)- 4, w - OBJECTFLOW_MARGIN * 2, fontHeight, Qt::AlignHCenter, getName());
            p.drawText(offsetX + OBJECTFLOW_MARGIN, (offsetY + textStartY/2) + (h/2) - 3, w - OBJECTFLOW_MARGIN * 2, fontHeight, Qt::AlignHCenter, objectflow_value);
        }
        UMLWidget::setPen(p);
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}
 
QSize ObjectFlowWidget::calculateSize() {
    int width = 10, height = 10;
    const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
    const int fontHeight  = fm.lineSpacing();
    int textWidth = fm.width(getName());
    if(fm.width(getState()) > textWidth)
    	textWidth = fm.width(getState());
    height = fontHeight;
    width = textWidth > OBJECTFLOW_WIDTH ? textWidth  : OBJECTFLOW_WIDTH;
    height = height + 5  > OBJECTFLOW_HEIGHT ? height + 5 : OBJECTFLOW_HEIGHT;
    width += OBJECTFLOW_MARGIN * 2;
    height += (OBJECTFLOW_MARGIN * 2);

    return QSize(width, height+5); 
}

void ObjectFlowWidget::setState(const QString &strState){
	/*if (m_pObject)
		m_pObject->setState(strState);
	else*/
		m_State = strState;
	    updateComponentSize();
    	adjustAssocs( getX(), getY() );	
}

QString ObjectFlowWidget::getState() const{
	/*if (m_pObject)
		return m_pObject->getState();*/
	return m_State;
}



void ObjectFlowWidget::askStateForWidget(){
							
	bool pressedOK = false;
	QString state = KInputDialog::getText(i18n("Enter Object Flow State"),
        	i18n("Enter State (keep '-' if there's no state for the object) "),i18n("-"), &pressedOK, UMLApp::app());
	if (pressedOK) {
		setState(state);
    } else {
        cleanup();
    }					
}

// void ObjectFlowWidget::slotMenuSelection(int sel) { 
//     bool done = false;  
// 
//     bool ok = false; 
//     QString name = m_Text; 
// 
//     switch( sel ) { 
//     case ListPopupMenu::mt_Rename: 
//         name = KInputDialog::getText( i18n("Enter Object Flow Name"), i18n("Enter the name of the new Object Flow:"), m_Text, &ok );
//         if( ok && name.length() > 0 ) 
//             m_Text = name; 
//         done = true; 
//         break; 
//  
//     case ListPopupMenu::mt_Properties:
//         showProperties(); 
//         done = true;
//         break;
//     }
// 
//     if( !done ) 
//         UMLWidget::slotMenuSelection( sel ); 
// }

//  bool ObjectFlowWidget::showProperties() { 
// //     DocWindow *docwindow = UMLApp::app()->getDocWindow();
// //     docwindow->updateDocumentation(false);
// // 
// //     ObjectFlowDialog dialog(m_pView, this);
// //     bool modified = false;
// //     if (dialog.exec() && dialog.getChangesMade()) {
// //         docwindow->showDocumentation(this, true);
// //         UMLApp::app()->getDocument()->setModified(true);
// //         modified = true;
// //     }
// // 
//      return true; 
 //}
// 


void ObjectFlowWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) { 
    QDomElement ObjectFlowElement = qDoc.createElement( "objectflowwidget" );
    UMLWidget::saveToXMI( qDoc, ObjectFlowElement ); 
    ObjectFlowElement.setAttribute( "objectflowname", m_Text ); 
    ObjectFlowElement.setAttribute( "documentation", m_Doc ); 
    ObjectFlowElement.setAttribute( "objectflowstate", m_State );
    qElement.appendChild( ObjectFlowElement ); 
}

bool ObjectFlowWidget::loadFromXMI( QDomElement & qElement ) { 
    if( !UMLWidget::loadFromXMI( qElement ) ) 
        return false; 
    m_Text = qElement.attribute( "objectflowname", "" ); 
    m_Doc = qElement.attribute( "documentation", "" );
    m_State = qElement.attribute( "objectflowstate", "" );
    //QString type = qElement.attribute( "objectflowtype", "1" );
  //  setObjectFlowType( (ObjectFlowType)type.toInt() );
    return true; 
}


#include "objectflowwidget.moc"

