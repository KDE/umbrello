
 /***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "associationtool.h"
#include "diagramview.h"
#include "diagram.h"
#include "diagramelement.h"
#include "diagramwidget.h"
#include "associationwidget.h"
#include "../umldoc.h"

#include <qcursor.h>
#include <qpopupmenu.h>
#include <kdebug.h>
#include <klocale.h>


namespace Umbrello
{

AssociationTool::AssociationTool( DiagramView  *view): PathTool(view)
{
	setName("associationtool");
	setIcon("association");
	setToolTip(i18n("Creates associations between diagram widgets"));
}

AssociationTool::~AssociationTool()
{ }

void AssociationTool::activate()
{ 
	setCursor( );
	PathTool::activate();
	kdDebug()<<"AssociationTool tool is now active"<<endl;
}

void AssociationTool::deactivate()
{
 	kdDebug()<<"Deactivating AssociationTool tool"<<endl;
	PathTool::deactivate();
	view()->viewport()->setMouseTracking(false);
 

}

void AssociationTool::setCursor( )
{
	view()->setCursor( QCursor(Qt::ArrowCursor ));
}


bool AssociationTool::createNextPathPoint( )
{
	DiagramWidget *w = diagram()->firstDiagramWidget(currentPos());
	if ( m_linePath.count() != 0 && w != 0 )
	{
		createPath();
		return true;
	}
	else if ( m_linePath.count() != 0 && w == 0 )
	{
		return PathTool::createNextPathPoint( );	 
	}
	else if (m_linePath.count() == 0 && w != 0 )
	{
		m_startWidget = w;
		return PathTool::createNextPathPoint( );
	}
	else 	    
	{
		kdDebug()<<"cannot start an association without a widget"<<endl;
		return false;
	}	
}


QPoint AssociationTool::findNextPathPoint( )
{
	if( m_startWidget )
	{
		m_startWidget->hideHotSpots( );
	}
	DiagramWidget *w = w = diagram()->firstDiagramWidget(currentPos());
	if(w)
	{
		m_underMouse = w;
		if( m_underMouse == m_startWidget )
			return currentPos( );
		int i = m_underMouse->closestHotSpot(currentPos());
		w->showHotSpots(i);
		return m_underMouse->hotSpotPosition(i);
	}
	if( m_underMouse )
	{
		m_underMouse->hideHotSpots( );
		m_underMouse = 0L;
	}
	return PathTool::findNextPathPoint( );
}



void AssociationTool::createPath( )
{
	if( m_underMouse == 0 )
	{
		kdWarning()<<"AssociationTool::createPath( ) called with no widget under the mouse."
		           <<" - ignoring request"<<endl;
		return;
	}	
	AssociationWidget *w = new AssociationWidget(diagram(),diagram()->document()->getUniqueID(),
	                                             m_startWidget,m_underMouse);
	
	QPointArray points( m_linePath.count() + 1 );
	points[0] = m_linePath.at(0)->startPoint();
	for( int i = 0; i < m_linePath.count(); i++ )
	{
		points[i+1] = m_linePath.at(i)->endPoint();
	}
	w->setPathPoints(points);
	
	m_linePath.clear();
	m_startWidget = 0L;
	m_underMouse->hideHotSpots( );
	m_underMouse = 0L;

	w->show();
	diagram()->update();
}


}

