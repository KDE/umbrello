  /***************************************************************************
                               selecttool.cpp
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

#include "selecttool.h"
#include "diagramview.h"
#include "diagram.h"
#include "diagramelement.h"
#include "associationwidget.h"
#include "diagramwidget.h"

#include <qcursor.h>
#include <qpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>

int m_hotSpot;

namespace Umbrello
{

SelectTool::SelectTool( DiagramView  *view): Tool(view,"selecttool") , m_selectionRect(0)
{
	setIcon("arrow");
	setToolTip(i18n("Select and move diagram elements"));
}

SelectTool::~SelectTool()
{ }

void SelectTool::activate()
{ 
	setCursor( );
	view()->setMouseTracking(false);
	kdDebug()<<"Select tool is now active"<<endl;
}

void SelectTool::deactivate()
{ kdDebug()<<"Deactivating Select tool"<<endl;
	diagram()->deselectAll();
}

void SelectTool::setCursor( )
{
	view()->setCursor( QCursor(Qt::ArrowCursor ));
}

bool SelectTool::mousePressEvent( )
{
	if( buttonPressed() == Qt::RightButton )
	{
		return false;
	}
	
	DiagramElement *element = diagram()->firstDiagramElement(currentPos());
	if( !element )
	{
		diagram()->deselectAll();
		diagram()->update();
		return true;
	}
	bool state = element->isSelected( );
	if(shiftPressed())
	{
		element->setSelected(!state);
	}
	else
	{
		diagram()->deselectAll();
		element->setSelected( true );
	}
	diagram()->update();
	return true;
}

bool SelectTool::mouseReleaseEvent( ) 
{kdDebug()<<"SelectTool::mouseReleaseEvent"<<endl;
	if(m_selectionRect)
	{
		delete m_selectionRect;
		m_selectionRect = 0L;
		diagram()->update();
	}
	m_hotSpot = -1;
	return true;
}

bool SelectTool::mouseDragEvent( )
{
	DiagramElement *element = diagram()->firstDiagramElement(savedPos());
	if( !m_selectionRect && !element ) //start selection
	{
		diagram()->deselectAll();
		m_selectFrom = savedPos();
		m_selectionRect = new QCanvasRectangle(m_selectFrom.x(),m_selectFrom.y(),1,1,diagram());
		m_selectionRect->setPen(QPen(QColor("grey"), 1, Qt::DotLine));
		m_selectionRect->show();
		diagram()->update();
		return true;
	}
		
	if(m_selectionRect)
	{
		QPoint diff(currentPos( ) - m_selectFrom);
		m_selectionRect->setSize( diff.x(), diff.y() );
		diagram()->setItemsSelected(m_selectionRect->rect(),true);
		diagram()->update();
	}
	else
	{
		//DiagramElement *element = diagram()->firstDiagramElement(currentPos( ));
		if( !element )
			return false;
		QPoint diff(currentPos( ) - savedPos( ));
		int spot = element->isHotSpot( savedPos( ) );
		kdDebug()<<"hotspot number = "<<spot<<endl;
		if( spot >= 0 )
		{ 
			element->moveHotSpotBy( spot, diff.x(), diff.y() );
		}
		else
		{
			diagram()->moveSelectedBy(diff.x(),diff.y());
			diagram()->update();
		}
	}
	return true;
}

bool SelectTool::mouseDragReleaseEvent( )
{
	DiagramElement *element = diagram()->firstDiagramElement(savedPos());
	if(m_selectionRect)
	{
		delete m_selectionRect;
		m_selectionRect = 0L;
		diagram()->update();
	}
	AssociationWidget* assoc = dynamic_cast<AssociationWidget*>(element);
	if(assoc)
	{
		assoc->startWidget()->hideHotSpots();
		assoc->endWidget()->hideHotSpots();
		//fake a widgetMoved call - if the first or last hot spot was moved, this makes sure
		//the path is really connected to the widget's spot
		assoc->widgetMoved();
	}
	return true;
}

bool SelectTool::mouseDblClickEvent( ) 
{
	DiagramElement *element = diagram()->firstDiagramElement( currentPos( ));
	if( element )
		element->execDefaultAction();
	return true;
}

// void SelectTool::contextMenu( )
// {
// 	
// }

//all interesting movments are handled by mouseDragEvent -
// since this tool does not use mouse tracking, we should neve receive
// this event anyways
bool SelectTool::mouseMoveEvent( ) { return false;}

bool SelectTool::keyPressed( Qt::Key ) { return false;}
bool SelectTool::keyReleased( Qt::Key ){ return false;}

}

