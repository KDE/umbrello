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

// own header
#include "seqlinewidget.h"

//kde includes
#include <kcursor.h>
#include <kdebug.h>

//qt includes
#include <QPainter>
#include <QGraphicsLineItem>
//app includes
#include "umlview.h"
#include "objectwidget.h"
#include "messagewidget.h"
#include "umlscene.h"

// class members
qreal const SeqLineWidget::m_nMouseDownEpsilonX = 20;

SeqLineWidget::SeqLineWidget( UMLScene * pScene, ObjectWidget * pObject ) : QGraphicsLineItem()
{
    m_pScene = pScene;
    m_pScene->addItem(this);
    m_pObject = pObject;
    setPen( QPen( m_pObject->getLineColor(), 0, Qt::DashLine ) );
    setZValue( 0 );
    setVisible( true );
    m_DestructionBox.line1 = 0;
    m_nLengthY = 250;
    setupDestructionBox();
}

SeqLineWidget::~SeqLineWidget() {}

qreal SeqLineWidget::onWidget( const QPointF & p )
{
    qreal nOnWidget = 0;
    QPointF sp = QGraphicsLineItem::line().p1();
    QPointF ep = QGraphicsLineItem::line().p2();
    //see if on widget ( for message creation )
    if( sp.x() - m_nMouseDownEpsilonX < p.x()
            && ep.x() + m_nMouseDownEpsilonX > p.x()
            && sp.y() < p.y() && ep.y() + 3 > p.y() )
    {
        nOnWidget = 1;
    }
    return nOnWidget;
}

qreal SeqLineWidget::onDestructionBox ( const QPointF & p ) {
    qreal nOnDestructionBox = 0;
    qreal x = m_pObject->getX() + m_pObject->getWidth() / 2;
    qreal y = m_pObject->getY() + m_pObject->getHeight() + m_nLengthY;

    //see if on destruction box
    if( !m_pObject->getShowDestruction() ) {
        return 0;
    }
    if( x - 10 < p.x() && x + 10 > p.x()
            && y - 10 < p.y() && y + 10 > p.y() )
    {
        nOnDestructionBox = 1;
    }
    return nOnDestructionBox;
}


void SeqLineWidget::cleanup() {
    cleanupDestructionBox();
}

void SeqLineWidget::setStartPoint( qreal startX, qreal startY ) {
    qreal endX = startX;
    qreal endY = startY + m_nLengthY;
    QGraphicsLineItem::setLine( startX, startY, endX, endY );
    moveDestructionBox();
}

void SeqLineWidget::cleanupDestructionBox()
{
    if ( m_DestructionBox.line1 ) {
        delete m_DestructionBox.line1;
        m_DestructionBox.line1 = 0;
        delete m_DestructionBox.line2;
        m_DestructionBox.line2 = 0;
    }
}

void SeqLineWidget::setupDestructionBox()
{
    cleanupDestructionBox();
    if( !m_pObject->getShowDestruction() ) {
        return;
    }
    QRectF rect;
    rect.setX( m_pObject->getX() + m_pObject->getWidth() / 2 - 10 );
    rect.setY( m_pObject->getY() + m_pObject->getHeight() + m_nLengthY );
    rect.setWidth( 14 );
    rect.setHeight( 14 );

    m_DestructionBox.line1 = new QGraphicsLineItem();
    m_pScene->addItem(m_DestructionBox.line1);
    m_DestructionBox.setLine1Points(rect);
    m_DestructionBox.line1->setVisible( true );
    m_DestructionBox.line1->setPen( QPen(m_pObject->getLineColor(), 2) );
    m_DestructionBox.line1->setZValue( 3 );

    m_DestructionBox.line2 = new QGraphicsLineItem();
    m_pScene->addItem(m_DestructionBox.line2);
    m_DestructionBox.setLine2Points(rect);
    m_DestructionBox.line2->setVisible( true );
    m_DestructionBox.line2->setPen( QPen(m_pObject->getLineColor(), 2) );
    m_DestructionBox.line2->setZValue( 3 );
}

void SeqLineWidget::moveDestructionBox()
{
    if( !m_DestructionBox.line1 ) {
        return;
    }
    QRectF rect;
    rect.setX( m_pObject->getX() + m_pObject->getWidth() / 2 - 7 );
    rect.setY( m_pObject->getY() + m_pObject->getHeight() + m_nLengthY - 7 );
    rect.setWidth( 14 );
    rect.setHeight( 14 );
    m_DestructionBox.setLine1Points(rect);
    m_DestructionBox.setLine2Points(rect);
}

void SeqLineWidget::setEndOfLine(qreal yPosition) {
    QPointF sp = QGraphicsLineItem::line().p1();
    qreal newY = yPosition;
    m_nLengthY = yPosition - m_pObject->getY() - m_pObject->getHeight();
    // normally the managing Objectwidget is responsible for the call of this function
    // but to be sure - make a double check _against current position_
    if ( m_nLengthY < 0 ) {
        m_nLengthY = 0;
        newY = m_pObject->getY() + m_pObject->getHeight();
    }
    setLine( sp.x(), sp.y(), sp.x(), newY );
    moveDestructionBox();
    m_pScene->resizeCanvasToItems();
}
