/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//qt includes
#include <qpointarray.h>
//kde includes
#include <kcursor.h>
#include <kcolordialog.h>
#include <kdebug.h>
//app includes
#include "umlview.h"
#include "boxwidget.h"

BoxWidget::BoxWidget(UMLView * view, int id) : UMLWidget( view, id ) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BoxWidget::BoxWidget(UMLView * view) : UMLWidget( view ) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void BoxWidget::init() {
	setSize(100,80);
	UMLWidget::setBaseType( wt_Box );
	setZ(0);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
BoxWidget::~BoxWidget() {

}
////////////////////////////////////////////////////////////////////////////////////////////////////
void BoxWidget::draw(QPainter& p, int offsetX, int offsetY) {
	p.drawRect( offsetX, offsetY, width(), height() );

	if (m_bSelected) {
		drawSelected(&p, offsetX, offsetY, true);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void BoxWidget::mouseMoveEvent(QMouseEvent *me) {
	if(!m_bResizing) {
		UMLWidget::mouseMoveEvent(me);
		return;
	}
	if( !m_bMouseDown )
		return;
	int newW = m_nOldW + me->x()- m_nOldX - m_nPressOffsetX;
	int newH = m_nOldH + me->y()- m_nOldY - m_nPressOffsetY;
	newW = newW < 20?20:newW;
	newH = newH < 20?20:newH;
	setSize( newW, newH );
	adjustAssocs( getX(), getY() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void BoxWidget::mousePressEvent(QMouseEvent *me) {
	UMLWidget::mousePressEvent(me);
	int w = width();
	int h = height();
	m_nOldW = w;
	m_nOldH = h;
	int m = 10;
	//bottomRight
	if( (m_nOldX + m_nPressOffsetX) >= (getX() + width() - m) &&
	    (m_nOldY + m_nPressOffsetY) >= (getY() + height() - m) && me->button() == LeftButton) {
		m_bResizing = true;
		m_pView->setCursor(KCursor::sizeFDiagCursor());
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void BoxWidget::mouseReleaseEvent(QMouseEvent* me) {
	UMLWidget::mouseReleaseEvent(me);
	m_bResizing = false;
	m_pView->setCursor( KCursor::arrowCursor() );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool BoxWidget::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement boxElement = qDoc.createElement("boxwidget");
	bool status = UMLWidget::saveToXMI(qDoc, boxElement);
	qElement.appendChild(boxElement);
	return status;
}

