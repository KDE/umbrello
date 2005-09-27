/*
 *  copyright (C) 2003-2005
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "boxwidget.h"
// qt/kde includes
#include <kcursor.h>
#include <kdebug.h>
// app includes
#include "umlview.h"

BoxWidget::BoxWidget(UMLView * view, Uml::IDType id) : UMLWidget( view, id ) {
    init();
}

void BoxWidget::init() {
    setSize(100,80);
    UMLWidget::setBaseType( Uml::wt_Box );
    WidgetBase::m_bUsesDiagramLineColour = false;  // boxes be black
    WidgetBase::m_LineColour = QColor("black");
    setZ(0);
}

BoxWidget::~BoxWidget() {

}

void BoxWidget::draw(QPainter& p, int offsetX, int offsetY) {
    UMLWidget::setPen(p);
    p.drawRect( offsetX, offsetY, width(), height() );

    if (m_bSelected) {
        drawSelected(&p, offsetX, offsetY, true);
    }
}

void BoxWidget::constrain(int& width, int& height) {
    if (width < 20)
        width = 20;
    if (height < 20)
        height = 20;
}

void BoxWidget::mouseMoveEvent(QMouseEvent *me) {
    if(!m_bResizing) {
        UMLWidget::mouseMoveEvent(me);
        return;
    }
    if( !m_bMouseDown )
        return;
    int newX = me->x();
    int newY = me->y();
    if (! m_bIgnoreSnapToGrid) {
        newX = m_pView->snappedX( newX );
        newY = m_pView->snappedY( newY );
    }
    int newW = m_nOldW + newX - m_nOldX - m_nPressOffsetX;
    int newH = m_nOldH + newY - m_nOldY - m_nPressOffsetY;
    constrain(newW, newH);
    setSize( newW, newH );
    adjustAssocs( getX(), getY() );
}

void BoxWidget::mousePressEvent(QMouseEvent *me) {
    UMLWidget::mousePressEvent(me);
    int w = width();
    int h = height();
    m_nOldW = w;
    m_nOldH = h;
    int m = 10;
    //bottomRight
    if( (m_nOldX + m_nPressOffsetX) >= (getX() + width() - m) &&
        (m_nOldY + m_nPressOffsetY) >= (getY() + height() - m) && me->button() == Qt::LeftButton) {
        m_bResizing = true;
        m_pView->setCursor(KCursor::sizeFDiagCursor());
    }
}

void BoxWidget::mouseReleaseEvent(QMouseEvent* me) {
    UMLWidget::mouseReleaseEvent(me);
    m_bResizing = false;
    m_pView->setCursor( KCursor::arrowCursor() );
}

void BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement boxElement = qDoc.createElement("boxwidget");
    UMLWidget::saveToXMI(qDoc, boxElement);
    qElement.appendChild(boxElement);
}

