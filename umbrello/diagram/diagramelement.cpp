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
#include <qpoint.h>
#include <qpopupmenu.h>


namespace Umbrello{

DiagramElement::DiagramElement( Diagram *diagram, int id): 
	QCanvasPolygonalItem(diagram),m_id(id)
{
}
	
DiagramElement::~DiagramElement()
{
	hide();
}
	
int DiagramElement::getID() const
{
	return m_id;
}	
	
void DiagramElement::moveAbs( int x, int y )
{
	QCanvasItem::move(x,y);
//do we need to do something else? should this be virtual?	
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
//do we need to do something else? should this be virtual?

update();
canvas()->update();
	emit moved();	
}

void DiagramElement::fillContextMenu(QPopupMenu &menu)
{
	menu.insertItem("properties",this,SLOT(editProperties()));
	menu.insertItem("to Forground",this,SLOT(moveToForeground()));
	menu.insertItem("to background",this,SLOT(moveToBackground()));
	menu.insertItem("delete",this,SLOT(deleteLater()));
}


void DiagramElement::moveToForeground()
{
	kdDebug()<<"foreground.."<<endl;
	QCanvasItemList l = collisions(false);
	kdDebug()<<"list is empty? "<<l.empty();
//	QCanvasItemList::iterator it;
//	for(it = collisions(false).begin(); it != collisions(false).end(); ++it)
//	kdDebug()<<"z = "<<(*it)->z()<<endl;
	//setZ(1000);
}
void DiagramElement::moveToBackground()
{
kdDebug()<<"background..."<<endl;
setZ(0);
}

void DiagramElement::setSelected(bool sel)
{
	QCanvasItem::setSelected(sel);
	update();
	canvas()->update();
}

void DiagramElement::doubleClick()
{
kdDebug()<<"execute default action."<<endl;
}

void DiagramElement::editProperties()
{
	kdDebug()<<"show Properties dialog"<<endl;
}
Diagram* DiagramElement::diagram() const
{
	return dynamic_cast<Diagram*>(canvas());
}

}


#include "diagramelement.moc"
