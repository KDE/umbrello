/***************************************************************************
    copyright            : (C) 2003 Luis De la Parra
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diagramwidget.h"
#include "diagram.h"
// #include "associationwidget.h"
#include <qrect.h>
#include <qcolor.h>
#include <qpainter.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kdebug.h>



namespace Umbrello{

DiagramWidget::DiagramWidget( Diagram *diagram, int id): DiagramElement(diagram,id)
{
}

DiagramWidget::~DiagramWidget()
{
	hide();
	canvas()->update();
}

void DiagramWidget::setSize( uint w, uint h )
{
	if( m_width == w && m_height == h )
		return;
	m_width = w;
	m_height = h;
	createHotSpots( );
	invalidate( );
}



QPointArray DiagramWidget::areaPoints() const
{
	QPointArray pa(4);
	int pw = (pen().width()+1)/2;
	if ( pw < 1 ) pw = 1;
	if ( pen() == NoPen ) pw = 0;
	if( isShowHotSpots() )
		pw += ( hotSpotSize( ) + 2) / 2;
	pa[0] = QPoint((int)x()-pw,(int)y()-pw);
	pa[1] = pa[0] + QPoint(m_width + pw*2, 0);
	pa[2] = pa[1] + QPoint(0, m_height + pw*2);
	pa[3] = pa[0] + QPoint(0, m_height + pw*2);
	return pa;

}


void DiagramWidget::createHotSpots( )
{
	m_hotSpots.clear();
//create points - one on each corner, plus i points equally distributed on each side
	QPoint *p = new QPoint(0,0);
	m_hotSpots.append(p);
	p = new QPoint(m_width,0);
	m_hotSpots.append(p);
	p = new QPoint(0,m_height);
	m_hotSpots.append(p);
	p = new QPoint(m_width,m_height);
	m_hotSpots.append(p);

	int i = 3;
	for( int j = 1; j <= i ; j++ )
	{
		p = new QPoint( (m_width / (i+1))*j, 0 );
		m_hotSpots.append(p);
		p = new QPoint( (m_width / (i+1))*j, m_height );
		m_hotSpots.append(p);

		p = new QPoint( 0 , (m_height / (i+1))*j );
		m_hotSpots.append(p);
		p = new QPoint( m_width, (m_height / (i+1))*j );
		m_hotSpots.append(p);
	}
}

void DiagramWidget::drawHotSpots( QPainter &p )
{
	DiagramElement::drawHotSpots(p);
}




}


#include "diagramwidget.moc"
