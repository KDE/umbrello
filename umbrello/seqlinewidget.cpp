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

SeqLineWidget::SeqLineWidget( UMLView * pView, ObjectWidget * pObject ) : QCanvasLine( pView -> canvas() ) {
	m_pView = pView;
	m_pObject = pObject;
	setPen( QPen( m_pObject -> getLineColor(), 0, DashLine ) );
	setZ( 0 );
	setVisible( true );
	m_pDeconBox.rect = 0;
	m_nLengthY = (int)m_pObject -> y() + 250;
	setupDeconBox();
	m_bMouseDown = false;
	m_nOffsetY = m_nOldY = 0;
	m_pView -> addSeqLine( this );
}

SeqLineWidget::~SeqLineWidget() {}

bool SeqLineWidget::onWidget( const QPoint & p ) {
	bool bOnWidget = false;
	m_bMouseDown = false;
	QPoint sp = startPoint();
	QPoint ep = endPoint();
	//see if on widget ( for message creation )
	if( sp.x() - 2 < p.x() && ep.x() + 2 > p.x() &&
	    sp.y() < p.y() && ep.y() + 3 > p.y() ) {
		bOnWidget = true;
	}
	if( m_pDeconBox.rect ) {
		QRect rect = m_pDeconBox.rect->rect();
		if( rect.x() <= p.x() && rect.x() + rect.width() >= p.x() &&
		    rect.y() <= p.y() && rect.y() + rect.height() >= p.y() ) {
			bOnWidget = true;
		}
	}
	//see if going to do a move
	if( ep.x() - 3 < p.x() && ep.x() + 3 > p.x() &&
	    ep.y() - 3 < p.y() && ep.y() + 3 > p.y() ) {
		m_bMouseDown = true;
	}

	if( m_pDeconBox.rect ) {
		QRect rect = m_pDeconBox.rect -> rect();
		if( rect.x() <= p.x() && rect.x() + rect.width() >= p.x() &&
		    rect.y() <= p.y() && rect.y() + rect.height() >= p.y() ) {
			m_bMouseDown =  true;
		}
	}
	return bOnWidget;
}

void SeqLineWidget::mouseMoveEvent( QMouseEvent *me ) {
	if( !m_bMouseDown )
		return;
	int dy = (int)me -> y() - m_nOffsetY - m_nOldY;
	QPoint ep = endPoint();
	int newY = (int)ep.y() + dy;
	m_nOldY = newY;
	QPoint sp = startPoint();
	newY = newY < m_nMinY?m_nMinY:newY;
	setPoints( sp.x(), sp.y(), sp.x(), newY );
	m_nLengthY = newY - (int)m_pObject -> y() - m_pObject -> height();
	moveDeconBox();
}

void SeqLineWidget::mouseReleaseEvent( QMouseEvent * /*me*/ ) {
	m_bMouseDown = false;
	m_pView -> setCursor(KCursor::arrowCursor());
}

void SeqLineWidget::mousePressEvent( QMouseEvent * me ) {
	QPoint ep = endPoint();
	if( me -> button() != LeftButton )
		return;
	m_nOffsetY = me -> y() - ep.y();
	m_nOldY = ep.y();
	m_pView -> setCursor(KCursor::sizeVerCursor());
	m_nMinY = (int)m_pObject -> y() + m_pObject -> height() + 50;
	QPtrList<MessageWidget> m_List = m_pView -> getMessageWidgetList();
	MessageWidget * pMessage = 0;
	ObjectWidget * wA, * wB;
	for( pMessage = m_List.first(); pMessage; pMessage = m_List.next() ) {
		int y = 0;
		wA = (ObjectWidget*)pMessage -> getWidgetA();
		wB = (ObjectWidget*)pMessage -> getWidgetB();
		if( wA == m_pObject ) {
			y = (int)pMessage -> y() + pMessage -> height();
		} else if( wB == m_pObject ) {
			y = (int)pMessage -> y() + pMessage -> height();
		}
		m_nMinY = m_nMinY < y?y:m_nMinY;
	}
}

void SeqLineWidget::cleanup() {
	cleanupDeconBox();
	m_pView -> removeSeqLine( this );
}

void SeqLineWidget::setStartPoint( int startX, int startY ) {
	int endX = startX;
	int endY = startY + m_nLengthY;
	QCanvasLine::setPoints( startX, startY, endX, endY );
	moveDeconBox();
}

void SeqLineWidget::cleanupDeconBox() {
	if( m_pDeconBox.rect ) {
		delete m_pDeconBox.rect;
		m_pDeconBox.rect = 0;
		delete m_pDeconBox.line1;//don't need to check as will be same as rect.
		delete m_pDeconBox.line2;
	}
}

void SeqLineWidget::setupDeconBox() {
	cleanupDeconBox();
	if( !m_pObject -> getShowDeconstruction() )
		return;
	QRect rect;
	rect.setX( (int)m_pObject -> x() + m_pObject -> width() / 2 - 5 );
	rect.setY( (int)m_pObject -> y() + m_pObject -> height() + m_nLengthY );
	rect.setWidth( 10 );
	rect.setHeight( 10 );
	m_pDeconBox.rect = new QCanvasRectangle( rect, m_pView -> canvas() );
	m_pDeconBox.rect -> setPen( QPen( black ) );
	m_pDeconBox.rect -> setBrush( QBrush( red ) );
	m_pDeconBox.rect -> setVisible( true );
	m_pDeconBox.rect -> setZ( 1 );

	rect.setWidth( 9 );
	rect.setHeight( 9 );
	m_pDeconBox.line1 = new QCanvasLine( m_pView -> canvas() );
	m_pDeconBox.line1 -> setPoints( rect.x(), rect.y(), rect.x() + rect.width(), rect.y() + rect.height() );
	m_pDeconBox.line1 -> setVisible( true );
	m_pDeconBox.line1 -> setPen( QPen( black ) );
	m_pDeconBox.line1 -> setZ( 2 );

	m_pDeconBox.line2 = new QCanvasLine( m_pView -> canvas() );
	m_pDeconBox.line2 -> setPoints( rect.x(), rect.y() + rect.height(), rect.x() + rect.width(), rect.y() );
	m_pDeconBox.line2 -> setVisible( true );
	m_pDeconBox.line2 -> setPen( QPen( black ) );
	m_pDeconBox.line2 -> setZ( 2 );
}

void SeqLineWidget::moveDeconBox() {
	if( !m_pDeconBox.rect )
		return;
	QRect rect;
	rect.setX( (int)m_pObject -> x() + m_pObject -> width() / 2 - 5 );
	rect.setY( (int)m_pObject -> y() + m_pObject -> height() + m_nLengthY );
	rect.setWidth( 9 );
	rect.setHeight( 9 );
	m_pDeconBox.rect -> setX( rect.x() );
	m_pDeconBox.rect -> setY( rect.y() );
	m_pDeconBox.line1 -> setPoints( rect.x(), rect.y(), rect.x() + rect.width(), rect.y() + rect.height() );
	m_pDeconBox.line2 -> setPoints( rect.x(), rect.y() + rect.height(), rect.x() + rect.width(), rect.y() );
}

void SeqLineWidget::setLineLength( int nLength ) {
	m_nLengthY = nLength;
	QPoint sp = startPoint();
	setPoints( sp.x(), sp.y(), sp.x(), m_nLengthY + (int)m_pObject -> y() + m_pObject -> height() + m_nLengthY );
	moveDeconBox();
}


