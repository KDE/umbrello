 /***************************************************************************
                               path.cpp
                             -------------------
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


#include "path.h"

#include <qpainter.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qpopupmenu.h>
#include <klocale.h>
#include <kdebug.h>

namespace Umbrello{


Path::Path( Diagram *diagram, uint id) : DiagramElement(diagram,id)
{
	m_segments.setAutoDelete(true);
	m_style = Direct;
}

Path::~Path()
{
	hide();
	m_segments.clear();
	canvas()->update();
}

void Path::setPathPoints( const QPointArray &a )
{
	int count = a.size();
	if( count < 2 )
		return;
	m_segments.clear();

	PathSegment *segment;
	QPoint start( a[0] ), end;
	for( uint i = 1; i < a.size(); i++ )
	{
		end = a[i];
		segment = new PathSegment( this );
		segment->setPoints( start.x(), start.y(), end.x(), end.y() );
		m_segments.append( segment );
		start = end;
	}
	createHotSpots( );
}

void Path::setVisible(bool v)
{
	for( PathSegment *segment = m_segments.first(); segment; segment = m_segments.next() )
	{
		segment->setVisible(v);
	}
	QCanvasPolygonalItem::setVisible(v);
}

void Path::moveAbs( int x, int y )
{
	PathSegment *segment = m_segments.first();
	if(!segment)
		return;
	int dx = segment->startPoint().x() - x;
	int dy = segment->startPoint().y() - y;
	moveBy( dx, dy );
}

void Path::moveAbs( const QPoint &p )
{
	moveAbs( p.x(), p.y() );
}

void Path::moveBy( int dx, int dy)
{
	for( PathSegment *segment = m_segments.first(); segment; segment = m_segments.next() )
	{
		segment->moveBy(dx,dy);
	}
	createHotSpots( );
	canvas()->update();
	emit moved();
}

void Path::moveHotSpotBy( int h, int dx, int dy )
{
	if( h < 0 || h >= (int)m_hotSpots.count() )
		return;
	PathSegment *before,*after;

	if( h == 0)
	{
		after = m_segments.first();
		after->setPoints( after->startPoint().x() + dx,
		                  after->startPoint().y() + dy,
		                  after->endPoint().x(),
		                  after->endPoint().y());
	}
	else if ( h == (int)(m_hotSpots.count() - 1) )
	{
		before = m_segments.last();
		before->setPoints( before->startPoint().x(),
		                   before->startPoint().y(),
		                   before->endPoint().x() + dx,
		                   before->endPoint().y() + dy );
	}
	else
	{
		before = m_segments.at(h-1);
		before->setPoints( before->startPoint().x(),
		                   before->startPoint().y(),
		                   before->endPoint().x() + dx,
		                   before->endPoint().y() + dy );
		after = m_segments.at(h);
		after->setPoints( after->startPoint().x() + dx,
		                  after->startPoint().y() + dy,
		                  after->endPoint().x(),
		                  after->endPoint().y());
	}
	QPoint *p = m_hotSpots.at(h);
	p->setX(p->x() + dx);
	p->setY(p->y() + dy);
	update();
	diagram()->update();
}

void Path::fillContextMenu(QPopupMenu &menu)
{
	QPopupMenu *pathMenu = new QPopupMenu(&menu, "path popup");
	pathMenu->setCheckable(true);
	pathMenu->insertItem(i18n("Direct Lines"),this,SLOT(setDirectStyle()),0,Direct);
	pathMenu->insertItem(i18n("Orthogonal Lines"),this,SLOT(setOrthogonalStyle()),0,Orthogonal);
	pathMenu->setItemChecked(m_style,true);
	menu.insertItem(i18n("Path Style"),pathMenu);

	DiagramElement::fillContextMenu(menu);
}

void Path::execDefaultAction()
{
	//QPoint = QCursor::pos();
	//map from global !!

	//insert / delete  a point inthe path

}

void Path::toggleHotSpot( const QPoint &p )
{kdDebug()<<"Path::toggleHotSpot( const QPoint &p )"<<endl;
	int hs = isHotSpot( p );
	if( hs > 0 ) //remove hotspot
	{kdDebug()<<"remove hs "<<hs<<endl;
		if( hs == 0 || hs == (int)m_segments.count() )
		{
			kdDebug()<<"Request to move HS "<<hs<<" ignored - cannot move first/last hs"<<endl;
			return;
		}
		QPointArray a(m_segments.count());
		a[0] = m_segments.first()->startPoint();
		int i;
		PathSegment *segment;
		for( segment = m_segments.first(), i = 1 ; segment; segment = m_segments.next(), ++i )
		{
			if( i == hs )
			{
				--i;
				continue;
			}
			a[i] = segment->endPoint();
		}
		setPathPoints(a);
	}
	else //insert HotSpot
	{
		PathSegment *tobreak;
		QCanvasRectangle *test = new QCanvasRectangle( p.x()-3, p.y()-3, 3, 3, diagram() );
		for( tobreak = m_segments.first(); tobreak; tobreak = m_segments.next() )
		{
			if( tobreak->collidesWith( test ) )
				break;
		}
		delete test;
		if( !tobreak )
		{
			kdDebug()<<"Request to create hotspot at ( "<<p.x()<<","<<p.y()<<") ignored - not on path"<<endl;
			return;
		}
		QPointArray a(m_segments.count() + 1 + 1);
		kdDebug()<<"a( "<<m_segments.count() + 1 + 1<<" )"<<endl;
		a[0] = m_segments.first()->startPoint();
		int i;
		PathSegment *segment;
		for( segment = m_segments.first(), i = 1 ; segment; segment = m_segments.next(), ++i )
		{
			if( segment == tobreak )
			{
				a[i] = p;
				kdDebug()<<"a["<<i<<"] = "<<p.x()<<","<<p.y()<<endl;
				++i;
			}
			kdDebug()<<"a["<<i<<"] = "<<segment->endPoint().x()<<","<<segment->endPoint().y()<<endl;
			a[i] = segment->endPoint();
		}
		setPathPoints(a);
	}
	setSelected(true);
	update();
	canvas()->update();

}

void Path::setDirectStyle()
{
	m_style = Direct;
	kdWarning()<<"style set to direct, but not implemented yet"<<endl;
}

void Path::setOrthogonalStyle()
{
	m_style = Orthogonal;
	kdWarning()<<"style set to orthogonal, but not implemented yet"<<endl;
}

// a path has a hotspot in each end of its segments
void Path::createHotSpots(  )
{
	m_hotSpots.clear();
	QPoint *p;
	for( PathSegment *segment = m_segments.first(); segment; segment = m_segments.next() )
	{
		p = new QPoint(segment->startPoint());
		m_hotSpots.append(p);
	}
	p = new QPoint(m_segments.last()->endPoint());
	m_hotSpots.append(p);
}

void Path::drawHotSpots(QPainter &p)
{
	DiagramElement::drawHotSpots(p);
}



QPointArray Path::areaPoints() const
{//FIXME - is this ok? optimize?
	QPtrList<PathSegment> &list = const_cast<QPtrList<PathSegment>& >(m_segments);
	QRect r;
	for( PathSegment *segment = list.first(); segment; segment = list.next() )
	{
		r |= segment->areaPoints().boundingRect();
	}
	return QPointArray(r);
}

void Path::drawShape(QPainter &p)
{
	for( PathSegment *segment = m_segments.first(); segment; segment = m_segments.next() )
	{
		segment->drawShape(p);
	}
	QPen  pen(p.pen());
	QBrush brush(p.brush());
 	if( isSelected() || isShowHotSpots( ) )
	{
		drawHotSpots( p );
	}
	p.setPen(pen);
	p.setBrush(brush);

}


} // end of namespace Umbrello

#include "path.moc"
