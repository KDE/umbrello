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
    //setObjectFlowType( objectflowType );
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
            //int middleX = w / 2; 
            int textStartY = (h / 2) - (fontHeight / 2);
            p.drawRect(offsetX, offsetY, w, h /*,(h * 60) / w, 60*/); 
            p.drawLine(offsetX + 10, offsetY + (h/2), (offsetX + w)-10,  offsetY + (h/2)  );
            p.setPen(Qt::black);
            p.setFont( UMLWidget::getFont() ); 
            p.drawText(offsetX + OBJECTFLOW_MARGIN, offsetY + textStartY,
                       w - OBJECTFLOW_MARGIN * 2, fontHeight, Qt::AlignCenter, getName());
        }
        UMLWidget::setPen(p);
    if(m_bSelected)
        drawSelected(&p, offsetX, offsetY);
}
 
QSize ObjectFlowWidget::calculateSize() {
    int width = 10, height = 10;
//     if ( m_ObjectFlowType == Normal ) {
        const QFontMetrics &fm = getFontMetrics(FT_NORMAL);
        const int fontHeight  = fm.lineSpacing(); 
        const int textWidth = fm.width(getName());
        height = fontHeight; 
        width = textWidth > OBJECTFLOW_WIDTH ? textWidth : OBJECTFLOW_WIDTH; 
        height = height > OBJECTFLOW_HEIGHT ? height : OBJECTFLOW_HEIGHT; 
        width += OBJECTFLOW_MARGIN * 2;
        height += OBJECTFLOW_MARGIN * 2; 
//     } else if ( m_ObjectFlowType == Branch ) {
//         width = height = 20;
//     }
    return QSize(width, height); 
}

// ObjectFlowWidget::ObjectFlowType ObjectFlowWidget::getObjectFlowType() const {
//     return m_ObjectFlowType;
// }

// void ObjectFlowWidget::setObjectFlowType( ObjectFlowType objectflowType ) {
//     m_ObjectFlowType = objectflowType;
//     UMLWidget::m_bResizable = (m_ObjectFlowType == Normal);
// }

void ObjectFlowWidget::slotMenuSelection(int sel) { 
    bool done = false;  

    bool ok = false; 
    QString name = m_Text; 

    switch( sel ) { 
    case ListPopupMenu::mt_Rename: 
        name = KInputDialog::getText( i18n("Enter Object Flow Name"), i18n("Enter the name of the new Object Flow:"), m_Text, &ok );
        if( ok && name.length() > 0 ) 
            m_Text = name; 
        done = true; 
        break; 
 
    case ListPopupMenu::mt_Properties:
        showProperties(); 
        done = true;
        break;
    }

    if( !done ) 
        UMLWidget::slotMenuSelection( sel ); 
}

 bool ObjectFlowWidget::showProperties() { 
//     DocWindow *docwindow = UMLApp::app()->getDocWindow();
//     docwindow->updateDocumentation(false);
// 
//     ObjectFlowDialog dialog(m_pView, this);
//     bool modified = false;
//     if (dialog.exec() && dialog.getChangesMade()) {
//         docwindow->showDocumentation(this, true);
//         UMLApp::app()->getDocument()->setModified(true);
//         modified = true;
//     }
// 
     return true; 
 }
// 
// bool ObjectFlowWidget::isObjectFlow(WorkToolBar::ToolBar_Buttons tbb,
//                                 ObjectFlowType& resultType)
// {
//     bool status = true;
//     switch (tbb) {
//     case WorkToolBar::tbb_Initial_ObjectFlow:
//         resultType = Initial;
//         break;
//     case WorkToolBar::tbb_ObjectFlow:
//         resultType = Normal;
//         break;
//     case WorkToolBar::tbb_End_ObjectFlow:
//         resultType = End;
//         break;
//     case WorkToolBar::tbb_Branch:
//         resultType = Branch;
//         break;
//     case WorkToolBar::tbb_Fork:
//         kError() << "ObjectFlowWidget::isObjectFlow returns Fork_DEPRECATED" << endl;
//         resultType = Fork_DEPRECATED;
//         break;
//     default:
//         status = false;
//         break;
//     }
//     return status;
// }

void ObjectFlowWidget::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) { 
    QDomElement ObjectFlowElement = qDoc.createElement( "objectflowwidget" );
    UMLWidget::saveToXMI( qDoc, ObjectFlowElement ); 
   // ObjectFlowElement.setAttribute( "objectflowname", m_Text ); 
    ObjectFlowElement.setAttribute( "documentation", m_Doc ); 
    qElement.appendChild( ObjectFlowElement ); 
}

bool ObjectFlowWidget::loadFromXMI( QDomElement & qElement ) { 
    if( !UMLWidget::loadFromXMI( qElement ) ) 
        return false; 
    //m_Text = qElement.attribute( "objectflowname", "" ); 
    m_Doc = qElement.attribute( "documentation", "" );
   /* QString type = qElement.attribute( "objectflowtype", "1" );
    setObjectFlowType( (ObjectFlowType)type.toInt() );
    */return true; 
}


#include "objectflowwidget.moc"

