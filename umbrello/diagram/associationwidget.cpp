
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 

#include "associationwidget.h"
#include "pathsegment.h"
#include "diagramwidget.h"

#include <qpoint.h>
#include <qpointarray.h>
#include <qpopupmenu.h>
#include <klocale.h>

#include <kdebug.h>

namespace Umbrello{


AssociationWidget::AssociationWidget( Diagram *diagram, uint id, DiagramWidget *start, DiagramWidget *end ):
                   Path( diagram, id ), m_startWidget(start), m_endWidget(end), m_autoAdjust(true)
{
	//we start with a reasonable default
	m_startSpot = m_startWidget->closestHotSpot(QPoint((m_endWidget->x() + m_endWidget->width())/2 ,
	                                                  (m_endWidget->y() + m_endWidget->height())/2 ));
	QPoint startPoint = m_startWidget->hotSpotPosition( m_startSpot );

	m_endSpot = m_endWidget->closestHotSpot(QPoint((m_startWidget->x() + m_startWidget->width())/2 ,
	                                                  (m_startWidget->y() + m_startWidget->height())/2 ));
	QPoint endPoint = m_endWidget->hotSpotPosition( m_endSpot );
	QPointArray a(2);
	a[0] = startPoint;
	a[1] = endPoint;
	setPathPoints(a);
	
	createHotSpots( );
	
	connect(m_startWidget,SIGNAL(moved()),this,SLOT(widgetMoved()));
	connect(m_endWidget,SIGNAL(moved()),this,SLOT(widgetMoved()));
	
	connect(m_startWidget,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	connect(m_endWidget,SIGNAL(destroyed()),this,SLOT(deleteLater()));
	
}

AssociationWidget::~AssociationWidget()
{
	hide();
	canvas()->update();
}

void AssociationWidget::moveBy( int dx, int dy)
{
	PathSegment *segment(0);
	for( segment = m_segments.first(); segment; segment = m_segments.next() )
	{
		segment->moveBy(dx,dy);
	}
	segment = m_segments.first();
	if(segment)
	{
		segment->moveBy(-dx,-dy);
		segment->setPoints(segment->startPoint().x(),segment->startPoint().y(),
	                           segment->endPoint().x() + dx, segment->endPoint().y() + dy );
	}
	segment = m_segments.last();
	if(segment)
	{
		segment->moveBy(-dx,-dy);
		segment->setPoints(segment->startPoint().x() + dx ,segment->startPoint().y() + dy,
	                   segment->endPoint().x(),segment->endPoint().y());
	}
	createHotSpots( );
	canvas()->update();
	emit moved();
}

//reimplemented to make sure the end points are hotspots
void AssociationWidget::setPathPoints( const QPointArray &a )
{
	Path::setPathPoints(a);
	QPoint start, end;
	m_startSpot = m_startWidget->closestHotSpot(a[0]);
	//make sure we've got the right spot
	start = m_startWidget->hotSpotPosition(m_startSpot);
	end = m_segments.first()->endPoint();
	m_segments.first()->setPoints(start.x(),start.y(),end.x(),end.y());
	                           
	m_endSpot = m_endWidget->closestHotSpot(a[a.size()-1]);
	//make sure we've got the right spot
	end = m_endWidget->hotSpotPosition(m_endSpot);
	start = m_segments.last()->startPoint();
	m_segments.last()->setPoints(start.x(),start.y(),end.x(),end.y());
	
	createHotSpots( );
}


void AssociationWidget::fillContextMenu(QPopupMenu &menu)
{
	QPopupMenu *subMenu = new QPopupMenu(&menu, "association popup");
	subMenu->setCheckable(true);
	subMenu->insertItem(i18n("AutoAdjust"),this,SLOT(setAutoAdjust()),0,1);
	subMenu->insertItem(i18n("Fixed to Widget"),this,SLOT(setFixedSpots()),0,0);
	subMenu->setItemChecked((int)m_autoAdjust,true);
	menu.insertItem(i18n("Association style"),subMenu);
	Path::fillContextMenu(menu);
}

void AssociationWidget::setAutoAdjust( )
{
	m_autoAdjust = true;
	//force a path recalculation
	widgetMoved();
}

void AssociationWidget::setFixedSpots( )
{
	m_autoAdjust = false;
	//force a path recalculation
	widgetMoved();
}

void AssociationWidget::moveHotSpotBy( int h, int dx, int dy )
{
	PathSegment *before,*after;
	int spot;
	QPoint point;
	if( h != 0 && h != m_hotSpots.count() -1 )
		return Path::moveHotSpotBy(h,dx,dy);
	kdDebug()<<"AssociationWidget::moveHotSpotBy()"<<endl;
	if( h == 0)
	{
		point.setX( m_segments.first()->startPoint().x() + dx );
		point.setY( m_segments.first()->startPoint().y() + dy );
		spot = m_startWidget->closestHotSpot(point);
		kdDebug()<<"moving hs 0 to widget spot "<<spot<<endl;
		m_startSpot = spot;
		m_startWidget->showHotSpots(spot);
		
		//point = m_startWidget->hotSpotPosition(spot);
		after = m_segments.first();
		after->setPoints( point.x(),
		                  point.y(),
		                  after->endPoint().x(),
		                  after->endPoint().y());
	}
	else if ( h == m_hotSpots.count() -1 )
	{
		point.setX( m_segments.last()->endPoint().x() + dx );
		point.setY( m_segments.last()->endPoint().y() + dy );
		spot = m_endWidget->closestHotSpot(point);
		kdDebug()<<"moving hs "<<h<<" to widget spot "<<spot<<endl;
		m_endWidget->showHotSpots(spot);
		m_endSpot = spot;
		//point = m_endWidget->hotSpotPosition(spot);
		before = m_segments.last();
		before->setPoints( before->startPoint().x(),
		                   before->startPoint().y(),
		                   point.x(),
		                   point.y() );
	}
	QPoint *p = m_hotSpots.at(h);
	p->setX(p->x() + dx);
	p->setY(p->y() + dy);
	update();
	diagram()->update();
	m_autoAdjust = false;
//	setFixedSpots();
}

void AssociationWidget::widgetMoved( )
{
	//const DiagramWidget *moved = dynamic_cast<const DiagramWidget*>(sender());
	
	PathSegment *segment;
	QPoint p;
	//if( moved == m_startWidget || m_segments.count() == 1 )
	{
		if(m_autoAdjust)
		{
			m_startSpot = m_startWidget->closestHotSpot(m_segments.at(0)->endPoint());
		}
		p = m_startWidget->hotSpotPosition(m_startSpot);
		segment = m_segments.first();
		segment->setPoints(p.x(),p.y(),segment->endPoint().x(),segment->endPoint().y());
	}
	//if( moved == m_endWidget || m_segments.count() == 1 )
	{
		if(m_autoAdjust)
		{
			m_endSpot = m_endWidget->closestHotSpot(m_segments.at(m_segments.count()-1)->startPoint());
		}
		p = m_endWidget->hotSpotPosition(m_endSpot);
		segment = m_segments.last();
		segment->setPoints(segment->startPoint().x(),segment->startPoint().y(),p.x(),p.y());
	}
	createHotSpots( );
	update();
	diagram()->update();
}



} // end of namespace Umbrello

#include "associationwidget.moc"
