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

#include "diagramelement.h"
#include "diagram.h"

#include <kiconloader.h>
#include <klocale.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qpainter.h>
#include <qrect.h>

#include <kdebug.h>

#include <math.h>


namespace Umbrello{

DiagramElement::DiagramElement( Diagram *diagram, int id):
        QCanvasPolygonalItem(diagram),m_id(id), m_useOwnPen(false), m_useOwnBrush(false)
{
	diagram->registerElement( this );
	setZ(100);  // default depth
	setPen(this->diagram()->pen());
	setBrush(this->diagram()->brush());
	m_showHotSpots = false;
	m_highlightHotSpot = -1;
	m_hotSpotSize = 5;
	m_hotSpots.setAutoDelete(true);
}

DiagramElement::~DiagramElement()
{
	hide();
	canvas()->update();
}

int DiagramElement::getID() const
{
	return m_id;
}

void DiagramElement::moveAbs( int x, int y )
{
	QCanvasItem::move(x,y);
	canvas()->update();
	emit moved();
}

void DiagramElement::moveAbs( const QPoint &point )
{
	moveAbs(point.x(), point.y());
}

void DiagramElement::moveBy( int dx, int dy)
{
	QCanvasPolygonalItem::moveBy(dx,dy);
	canvas()->update();
	emit moved();
}

void DiagramElement::setHotSpotSize( int s )
{
	m_hotSpotSize = s;
	if(m_showHotSpots)
	{// size of bounding box might have changed
		invalidate();
		update();
		diagram()->update();
	}
}

int DiagramElement::hotSpotSize( ) const
{
	return m_hotSpotSize;
}

void DiagramElement::showHotSpots( int highlight )
{
	m_showHotSpots = true;
	m_highlightHotSpot = highlight;
	//we need to call invalidate because the size needs to be recalculated
	invalidate();
	update();
	canvas()->update();
}

void DiagramElement::hideHotSpots( )
{
	//we need to call invalidate because the size needs to be recalculated
	invalidate();
	m_showHotSpots = false;
	m_highlightHotSpot = -1;
	update();
	canvas()->update();
}

int DiagramElement::closestHotSpot( const QPoint &point) const
{
	QPoint *offset;
	int index;
	double distance;
	int best_index = -1 ;
	double best_distance = 999999999;
	int dx, dy;
	for( offset = m_hotSpots.first(), index = 0; offset ; offset = m_hotSpots.next(), index++ )
	{
		QPoint hot(QPoint((int)x(),(int)y()) + (*offset) );
		dx = point.x() - hot.x();
		dy = point.y() - hot.y();
		distance = sqrt( (dx*dx) + (dy*dy) );
		if( distance < best_distance )
		{
			best_distance = distance;
			best_index = index;
		}
	}
	return best_index;
}

int DiagramElement::isHotSpot( const QPoint &p ) const
{
	QRect hotspot(0,0,m_hotSpotSize,m_hotSpotSize);
	QPoint *offset;
	int index;
	for( offset = m_hotSpots.first(), index = 0; offset ; offset = m_hotSpots.next(), index++ )
	{
		hotspot.moveCenter(QPoint((int)x(),(int)y()) + (*offset) );
		if( hotspot.contains( p ) )
		{
			return index;
		}
	}
	return -1;
}

QPoint DiagramElement::hotSpotPosition( int i ) const
{
	if( i < 0 || i > (int)m_hotSpots.count() )
		return QPoint();
	return QPoint( QPoint((int)x(),(int)y()) + *(m_hotSpots.at(i)) );
}

void DiagramElement::drawHotSpots(QPainter& p)
{
	// save, normal and highlight pen and brushes...
	QPen spen = p.pen();
	QBrush sbrush = p.brush();
	QPen npen(Qt::red);
	QBrush nbrush(Qt::magenta);
	QPen hpen(Qt::green);
	QBrush hbrush(Qt::red);


	p.setPen(npen);
	p.setBrush(nbrush);
	QRect hotspot(0,0,hotSpotSize(),hotSpotSize());
	QPoint *offset;
	int index;
	for( offset = m_hotSpots.first(), index = 0; offset ; offset = m_hotSpots.next(), index++ )
	{
		hotspot.moveCenter(QPoint((int)x(),(int)y()) + (*offset) );
		if( index == m_highlightHotSpot )
		{
			p.setPen(hpen);
			p.setBrush(hbrush);
			p.drawRect(hotspot);
			p.setPen(npen);
			p.setBrush(nbrush);
		}
		else
		{
			p.drawRect(hotspot);
		}
	}
	p.setBrush(sbrush);
	p.setPen(spen);
}


void DiagramElement::createHotSpots( )
{
	m_hotSpots.clear();
}

void DiagramElement::moveHotSpotBy( int /* h */, int dx, int dy )
{//for a simple DiagramElement, move the complete element
	moveBy(dx,dy);
}


void DiagramElement::fillContextMenu(QPopupMenu &menu)
{
	QPopupMenu *submenu = new QPopupMenu( &menu, "diagramelement submenu");
	submenu->insertItem(SmallIcon("foreground"),i18n("To Foreground"),this,SLOT(moveToForeground()));
	submenu->insertItem(SmallIcon("raise"),i18n("Raise"),this,SLOT(raise()));
	submenu->insertItem(SmallIcon("lower"),i18n("Lower"),this,SLOT(lower()));
	submenu->insertItem(SmallIcon("backround"),i18n("To Background"),this,SLOT(moveToBackground()));
	menu.insertItem(i18n("Depth..."),submenu);

	menu.insertItem(SmallIcon("info"),i18n("Properties"),this,SLOT(editProperties()));
	menu.insertSeparator( );
	menu.insertItem(SmallIcon("editdelete"),i18n("Delete"),this,SLOT(deleteLater()));
}

void DiagramElement::moveToForeground()
{
	QCanvasItemList l = collisions(false);
	if(l.empty())
		return; // nothing to do

	double maxZ = z();

	QCanvasItemList::iterator it;
        QCanvasItemList::iterator end(l.end());

        for(it = l.begin(); it != end; ++it )
	{
		maxZ = kMax(maxZ, (*it)->z());
	}

	kdDebug()<<"max z is "<<maxZ<<endl;
	if( z() < maxZ )
	{
		setZ( maxZ + 1 );
	}
	else // we are already higher or equal than all others,
	{   //  move all others one down to make sure we are drawn on top

		for( it = l.begin() ; it != end ; ++it )
		{
			(*it)->setZ((*it)->z() - 1 );
			kdDebug()<<"lowering other"<<maxZ<<endl;
		}
	}
}

void DiagramElement::moveToBackground()
{
	QCanvasItemList l = collisions(false);
	if(l.empty()) return; // nothing to do

	double minZ = z();

	QCanvasItemList::iterator it;
        QCanvasItemList::iterator end(l.end());

        for(it = l.begin(); it != end; ++it )
	{
		minZ = kMin(minZ, (*it)->z());
	}

	kdDebug()<<"min z is "<<minZ<<endl;
	if( z() > minZ )
	{
		setZ( minZ - 1 );
	}
	else // we are already lower or equal than all others,
	{   //  move all others one up to make sure we are drawn in the backround
		for( it = l.begin() ; it != end ; ++it )
		{
			(*it)->setZ((*it)->z() + 1 );
			kdDebug()<<"raising other"<<minZ<<endl;
		}
	}
}

void DiagramElement::raise()
{
	QCanvasItemList l = collisions(false);
	if(l.empty()) return; // nothing to do

	double next = z();


        QCanvasItemList::iterator it;
        QCanvasItemList::iterator end(l.end());

	for( it = l.begin() ; it != end ; ++it )
	{
		if((*it)->z() > z() )
		{
			next = kMin(next, (*it)->z());
		}
	}
	kdDebug()<<"next z is "<<next<<endl;
	if( z() < next )
	{
		setZ( next + 1 );
	}
	else // we are already higher or equal than all others,
	{   //  move all others one down to make sure we are drawn on the top
		for( it = l.begin() ; it != end ; ++it )
		{
			(*it)->setZ((*it)->z() - 1 );
			kdDebug()<<"lowering other"<<endl;
		}
	}
}

void DiagramElement::lower()
{
	QCanvasItemList l = collisions(false);
	if(l.empty()) return; // nothing to do

	double prev = z();

        QCanvasItemList::iterator it;
        QCanvasItemList::iterator end(l.end());

	for( it = l.begin() ; it != end ; ++it )
	{
		if((*it)->z() < z() )
		{
			prev = kMax(prev, (*it)->z());
		}
	}
	kdDebug()<<"prev z is "<<prev<<endl;
	if( z() < prev )
	{
		setZ( prev - 1 );
	}
	else // we are already lower or equal than all others,
	{   //  move all others one up to make sure we are drawn on the bottom
		for( it = l.begin() ; it != end ; ++it )
		{
			(*it)->setZ((*it)->z() + 1 );
			kdDebug()<<"raising other"<<endl;
		}
	}
}


void DiagramElement::setSelected(bool sel)
{
	if( isSelected() != sel )
	{
		QCanvasPolygonalItem::setSelected(sel);
		update();
		canvas()->update();
		emit selected(sel);
	}
}

void DiagramElement::execDefaultAction()
{
	kdWarning()<<"DiagramElement::execDefaultAction() called."
		<<"This should reimplemented by the derived classes"<<endl;
}

void DiagramElement::editProperties()
{
	kdWarning()<<"DiagramElement::editProperties() called."
		<<"This should reimplemented by the derived classes"<<endl;
}

Diagram* DiagramElement::diagram() const
{
	return dynamic_cast<Diagram*>(canvas());
}

void DiagramElement::setUseOwnPen( bool b )
{
	m_useOwnPen = b;
	update();
	canvas()->update();
}

void DiagramElement::setUseOwnBrush( bool b )
{
	m_useOwnBrush = b;
	update();
	canvas()->update();
}

}


#include "diagramelement.moc"
