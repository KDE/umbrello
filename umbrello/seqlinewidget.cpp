/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//kde includes
#include <kcursor.h>
#include <kdebug.h>
//app includes
#include "umlview.h"
#include "objectwidget.h"
#include "messagewidget.h"
#include "seqlinewidget.h"

// class members
int const SeqLineWidget::m_nMouseDownEpsilonX = 20;

SeqLineWidget::SeqLineWidget( UMLView * pView, ObjectWidget * pObject ) : QCanvasLine( pView -> canvas() ) {
	m_pView = pView;
	m_pObject = pObject;
	setPen( QPen( m_pObject -> getLineColor(), 0, DashLine ) );
	setZ( 0 );
	setVisible( true );
	m_pDestructionBox.line1 = 0;
	m_nLengthY = 250;
	setupDestructionBox();
	m_nOffsetY = m_nOldY = 0;
	m_pView -> addSeqLine( this );
	m_nMinY = m_nLengthY + m_pObject->height() + (int)m_pObject->y();
}

SeqLineWidget::~SeqLineWidget() {}

bool SeqLineWidget::onWidget( const QPoint & p ) {
	bool bOnWidget = false;
	QPoint sp = startPoint();
	QPoint ep = endPoint();
	//see if on widget ( for message creation )
	if( sp.x() - m_nMouseDownEpsilonX < p.x()
	    && ep.x() + m_nMouseDownEpsilonX > p.x()
	    && sp.y() < p.y() && ep.y() + 3 > p.y() )
	{
		bOnWidget = true;
	}
	return bOnWidget;
}

void SeqLineWidget::cleanup() {
	cleanupDestructionBox();
	m_pView->removeSeqLine( this );
}

void SeqLineWidget::setStartPoint( int startX, int startY ) {
	int endX = startX;
	int endY = startY + m_nLengthY;
	QCanvasLine::setPoints( startX, startY, endX, endY );
	moveDestructionBox();
}

void SeqLineWidget::cleanupDestructionBox() {
	if ( m_pDestructionBox.line1 ) {
		delete m_pDestructionBox.line1;
		m_pDestructionBox.line1 = 0;
		delete m_pDestructionBox.line2;
		m_pDestructionBox.line2 = 0;
	}
}

void SeqLineWidget::setupDestructionBox() {
	cleanupDestructionBox();
	if( !m_pObject->getShowDestruction() ) {
		return;
	}
	QRect rect;
	rect.setX( (int)m_pObject->x() + m_pObject->width() / 2 - 10 );
	rect.setY( (int)m_pObject->y() + m_pObject->height() + m_nLengthY );
	rect.setWidth( 14 );
	rect.setHeight( 14 );

	m_pDestructionBox.line1 = new QCanvasLine( m_pView->canvas() );
	m_pDestructionBox.line1->setPoints( rect.x(), rect.y(),
					      rect.x() + rect.width(), rect.y() + rect.height() );
	m_pDestructionBox.line1->setVisible( true );
	m_pDestructionBox.line1->setPen( QPen(m_pObject->getLineColor(), 2) );
	m_pDestructionBox.line1->setZ( 2 );

	m_pDestructionBox.line2 = new QCanvasLine( m_pView -> canvas() );
	m_pDestructionBox.line2->setPoints( rect.x(), rect.y() + rect.height(), rect.x() + rect.width(), rect.y() );
	m_pDestructionBox.line2->setVisible( true );
	m_pDestructionBox.line2->setPen( QPen(m_pObject->getLineColor(), 2) );
	m_pDestructionBox.line2->setZ( 2 );
}

void SeqLineWidget::moveDestructionBox() {
	if( !m_pDestructionBox.line1 ) {
		return;
	}
	QRect rect;
	rect.setX( (int)m_pObject->x() + m_pObject->width() / 2 - 7 );
	rect.setY( (int)m_pObject->y() + m_pObject->height() + m_nLengthY - 7 );
	rect.setWidth( 14 );
	rect.setHeight( 14 );
	m_pDestructionBox.line1->setPoints( rect.x(), rect.y(),
					    rect.x() + rect.width(), rect.y() + rect.height() );
	m_pDestructionBox.line2->setPoints( rect.x(), rect.y() + rect.height(),
					    rect.x() + rect.width(), rect.y() );
}

void SeqLineWidget::setLineLength( int nLength ) {
	m_nLengthY = nLength;
	QPoint sp = startPoint();
	setPoints( sp.x(), sp.y(), sp.x(),
		   m_nLengthY + (int)m_pObject->y() + m_pObject->height() + m_nLengthY );
	moveDestructionBox();
}

void SeqLineWidget::setEndOfLine(int height) {
	int dy = height - m_nOffsetY - m_nOldY;
	QPoint endpoint = endPoint();
	int newY = (int)endpoint.y() + dy;
	m_nOldY = newY;
	QPoint sp = startPoint();

	if (newY < m_nMinY) {
		newY = m_nMinY;
	}

	setPoints( sp.x(), sp.y(), sp.x(), newY );
	m_nLengthY = newY - (int)m_pObject->y() - m_pObject->height();
	moveDestructionBox();
	m_pView->resizeCanvasToItems();
}
