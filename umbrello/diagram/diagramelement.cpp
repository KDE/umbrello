/***************************************************************************
                               diagramelement.cpp
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

#include "diagramelement.h"
#include "diagram.h"
#include <kiconloader.h>
#include <klocale.h>
#include <qpoint.h>
#include <qpopupmenu.h>

//anonymous namespace for file-only data
float min( float a, float b ) { return a < b ? a: b;}
float max( float a, float b ) { return a > b ? a: b;}


namespace Umbrello{

DiagramElement::DiagramElement( Diagram *diagram, int id):
	QCanvasPolygonalItem(diagram),m_id(id), m_useOwnPen(false), m_useOwnBrush(false)
{
	setZ(100);  // default depth
	setPen(this->diagram()->pen());
	setBrush(this->diagram()->brush());
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
	update();
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
	update();
	canvas()->update();
	emit moved();
}

void DiagramElement::fillContextMenu(QPopupMenu &menu)
{
	QPopupMenu *submenu = new QPopupMenu( &menu, "diagramelement submenu");
	submenu->insertItem(SmallIcon("foreground"),i18n("To forground"),this,SLOT(moveToForeground()));
	submenu->insertItem(SmallIcon("raise"),i18n("Raise"),this,SLOT(raise()));
	submenu->insertItem(SmallIcon("lower"),i18n("Lower"),this,SLOT(lower()));
	submenu->insertItem(SmallIcon("backround"),i18n("To background"),this,SLOT(moveToBackground()));
	menu.insertItem(i18n("Depth..."),submenu);

	menu.insertItem(SmallIcon("info"),i18n("Properties"),this,SLOT(editProperties()));
	menu.insertSeparator( );
	menu.insertItem(SmallIcon("editdelete"),i18n("Delete"),this,SLOT(deleteLater()));
}


void DiagramElement::moveToForeground()
{
	QCanvasItemList l = collisions(false);
	if(l.empty()) return; // nothing to do

	double maxZ = z();
	QCanvasItemList::iterator it;
	for( it = l.begin() ; it != l.end() ; ++it )
	{
		maxZ = max(maxZ, (*it)->z());
	}
	kdDebug()<<"max z is "<<maxZ<<endl;
	if( z() < maxZ )
	{
		setZ( maxZ + 1 );
	}
	else // we are already higher or equal than all others,
	{   //  move all others one down to make sure we are drawn on top
		for( it = l.begin() ; it != l.end() ; ++it )
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
	for( it = l.begin() ; it != l.end() ; ++it )
	{
		minZ = min(minZ, (*it)->z());
	}
	kdDebug()<<"min z is "<<minZ<<endl;
	if( z() > minZ )
	{
		setZ( minZ - 1 );
	}
	else // we are already lower or equal than all others,
	{   //  move all others one up to make sure we are drawn in the backround
		for( it = l.begin() ; it != l.end() ; ++it )
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
	for( it = l.begin() ; it != l.end() ; ++it )
	{
		if((*it)->z() > z() )
		{
			next = min(next, (*it)->z());
		}
	}
	kdDebug()<<"next z is "<<next<<endl;
	if( z() < next )
	{
		setZ( next + 1 );
	}
	else // we are already higher or equal than all others,
	{   //  move all others one down to make sure we are drawn on the top
		for( it = l.begin() ; it != l.end() ; ++it )
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
	for( it = l.begin() ; it != l.end() ; ++it )
	{
		if((*it)->z() < z() )
		{
			prev = max(prev, (*it)->z());
		}
	}
	kdDebug()<<"prev z is "<<prev<<endl;
	if( z() < prev )
	{
		setZ( prev - 1 );
	}
	else // we are already lower or equal than all others,
	{   //  move all others one up to make sure we are drawn on the bottom
		for( it = l.begin() ; it != l.end() ; ++it )
		{
			(*it)->setZ((*it)->z() + 1 );
			kdDebug()<<"raising other"<<endl;
		}
	}
}

void DiagramElement::setSelected(bool sel)
{
	QCanvasItem::setSelected(sel);
	update();
	canvas()->update();
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

}


#include "diagramelement.moc"
