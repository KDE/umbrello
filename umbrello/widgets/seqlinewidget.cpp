/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "seqlinewidget.h"

//qt includes
#include <QtGui/QPainter>

//app includes
#include "umlview.h"
#include "objectwidget.h"
#include "messagewidget.h"

// class members
int const SeqLineWidget::m_nMouseDownEpsilonX = 20;

/**
 * Constructor.
 */
SeqLineWidget::SeqLineWidget(UMLView * pView, ObjectWidget * pObject)
  : UMLSceneLine( pView -> canvas() )
{
    m_scene = pView;
    m_pObject = pObject;
    setPen( QPen( m_pObject->lineColor(), 0, Qt::DashLine ) );
    setZ( 0 );
    setVisible( true );
    m_DestructionBox.line1 = 0;
    m_nLengthY = 250;
    setupDestructionBox();
}

/**
 * Destructor.
 */
SeqLineWidget::~SeqLineWidget()
{
}

/**
 * Return whether on seq. line.
 * Takes into account destruction box if shown.
 *
 * @param p The point to investigate.
 * @return  Non-zero if point is on this sequence line.
 */
int SeqLineWidget::onWidget(const QPoint & p)
{
    int nOnWidget = 0;
    QPoint sp = startPoint();
    QPoint ep = endPoint();
    //see if on widget ( for message creation )
    if( sp.x() - m_nMouseDownEpsilonX < p.x()
            && ep.x() + m_nMouseDownEpsilonX > p.x()
            && sp.y() < p.y() && ep.y() + 3 > p.y() )
    {
        nOnWidget = 1;
    }
    return nOnWidget;
}

/**
 * Return whether on the destruction box.
 *
 * @param p The point to investigate.
 * @return  Non-zero if point is on the destruction box of this sequence line.
 */
int SeqLineWidget::onDestructionBox(const QPoint & p)
{
    int nOnDestructionBox = 0;
    int x = m_pObject->getX() + m_pObject->getWidth() / 2;
    int y = m_pObject->getY() + m_pObject->getHeight() + m_nLengthY;

    //see if on destruction box
    if( !m_pObject->showDestruction() ) {
        return 0;
    }
    if( x - 10 < p.x() && x + 10 > p.x()
            && y - 10 < p.y() && y + 10 > p.y() )
    {
        nOnDestructionBox = 1;
    }
    return nOnDestructionBox;
}

/**
 * Clean up anything before deletion.
 */
void SeqLineWidget::cleanup()
{
    cleanupDestructionBox();
}

/**
 * Set the start point of the line.
 *
 * @param startX    X coordinate of the start point.
 * @param startY    Y coordinate of the start point.
 */
void SeqLineWidget::setStartPoint(int startX, int startY)
{
    int endX = startX;
    int endY = startY + m_nLengthY;
    UMLSceneLine::setPoints( startX, startY, endX, endY );
    moveDestructionBox();
}

/**
 * Clean up destruction box.
 */
void SeqLineWidget::cleanupDestructionBox()
{
    if ( m_DestructionBox.line1 ) {
        delete m_DestructionBox.line1;
        m_DestructionBox.line1 = 0;
        delete m_DestructionBox.line2;
        m_DestructionBox.line2 = 0;
    }
}

/**
 * Set up destruction box.
 */
void SeqLineWidget::setupDestructionBox()
{
    cleanupDestructionBox();
    if( !m_pObject->showDestruction() ) {
        return;
    }
    QRect rect;
    rect.setX( m_pObject->getX() + m_pObject->getWidth() / 2 - 10 );
    rect.setY( m_pObject->getY() + m_pObject->getHeight() + m_nLengthY );
    rect.setWidth( 14 );
    rect.setHeight( 14 );

    m_DestructionBox.line1 = new UMLSceneLine( m_scene->canvas() );
    m_DestructionBox.setLine1Points(rect);
    m_DestructionBox.line1->setVisible( true );
    m_DestructionBox.line1->setPen( QPen(m_pObject->lineColor(), 2) );
    m_DestructionBox.line1->setZ( 3 );

    m_DestructionBox.line2 = new UMLSceneLine( m_scene -> canvas() );
    m_DestructionBox.setLine2Points(rect);
    m_DestructionBox.line2->setVisible( true );
    m_DestructionBox.line2->setPen( QPen(m_pObject->lineColor(), 2) );
    m_DestructionBox.line2->setZ( 3 );
}

/**
 * Move destruction box.
 */
void SeqLineWidget::moveDestructionBox()
{
    if( !m_DestructionBox.line1 ) {
        return;
    }
    QRect rect;
    rect.setX( m_pObject->getX() + m_pObject->getWidth() / 2 - 7 );
    rect.setY( m_pObject->getY() + m_pObject->getHeight() + m_nLengthY - 7 );
    rect.setWidth( 14 );
    rect.setHeight( 14 );
    m_DestructionBox.setLine1Points(rect);
    m_DestructionBox.setLine2Points(rect);
}

/**
 * Sets the y position of the bottom of the vertical line.
 *
 * @param yPosition The y coordinate for the bottom of the line.
 */
void SeqLineWidget::setEndOfLine(int yPosition)
{
    QPoint sp = startPoint();
    int newY = yPosition;
    m_nLengthY = yPosition - m_pObject->getY() - m_pObject->getHeight();
    // normally the managing Objectwidget is responsible for the call of this function
    // but to be sure - make a double check _against current position_
    if ( m_nLengthY < 0 ) {
        m_nLengthY = 0;
        newY = m_pObject->getY() + m_pObject->getHeight();
    }
    setPoints( sp.x(), sp.y(), sp.x(), newY );
    moveDestructionBox();
    m_scene->resizeCanvasToItems();
}
