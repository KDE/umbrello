  /***************************************************************************
                               pathtool.cpp
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

#include "pathtool.h"
#include "diagramview.h"
#include "diagram.h"
#include "diagramelement.h"
#include "diagramwidget.h"
#include "path.h"


#include "../umldoc.h"

#include <qcursor.h>
#include <qpopupmenu.h>
#include <kdebug.h>
#include <qcolor.h>
#include <qpen.h>
#include <klocale.h>


namespace Umbrello
{

PathTool::PathTool( DiagramView  *view): Tool(view,"pathtool")
{
	setIcon("path");
	setToolTip(i18n("Creates paths"));
	m_linePath.setAutoDelete(true);
}

PathTool::~PathTool()
{ }

void PathTool::activate()
{
	setCursor( );
	view()->viewport()->setMouseTracking(true);
	kdDebug()<<"PathTool tool is now active"<<endl;
}

void PathTool::deactivate()
{
 	kdDebug()<<"Deactivating PathTool tool"<<endl;
	view()->viewport()->setMouseTracking(false);


}

void PathTool::setCursor( )
{
	view()->setCursor( QCursor(Qt::ArrowCursor ));
}

bool PathTool::mousePressEvent( )
{
	if( buttonPressed() == Qt::RightButton )
	{
		if( m_linePath.count() > 0 )
		{
			cancelPathCreation( );
			return true;
		}
	}
	else
	{
		return createNextPathPoint( );
	}
	return false;
}

void PathTool::cancelPathCreation( )
{
	m_linePath.clear();
	diagram()->update();
}

bool PathTool::createNextPathPoint( )
{
	QCanvasLine *line;
	QPoint point;
	if( m_linePath.count() > 0 )
	{
		point = m_linePath.last()->endPoint();
	}
	else
	{
		point = currentPos();
	}
	line = new QCanvasLine(diagram());
	line->setPoints(point.x(),point.y(),point.x(),point.y());
	line->setPen(QPen( QColor(Qt::red), 0, QPen::DashLine ));
	line->show();
	m_linePath.append(line);
	diagram()->update();
	return true;
}

bool PathTool::mouseReleaseEvent( )
{
	return false;
}

bool PathTool::mouseMoveEvent( )
{
	QCanvasLine *line;
	if( m_linePath.count() > 0 )
	{
		line = m_linePath.last();
		QPoint start = line->startPoint( );
		QPoint end = findNextPathPoint( );
		//line->invalidate();
		//line->hide();
		line->setPoints(start.x(),start.y(),end.x(),end.y());
		//line->show();
		diagram()->update();
		return true;
	}
	return false;
}

QPoint PathTool::findNextPathPoint( )
{
	QCanvasLine *line = m_linePath.last( );
	if( !line || !ctrlPressed( ) )
		return currentPos( );
	// control key is pressed, so adjust the next positon
	QPoint prev, next;
	prev = line->startPoint( );
	next = currentPos( );

	float dx = next.x() - prev.x();
	float dy = next.y() - prev.y();
	if( dx < 0 )
		dx = dx * -1;
	if( dy < 0 )
		dy = dy * -1;

	if( dx > (dy*2.5) )
	{	//adjust to horizontal
		next.setY(prev.y());
	}
	else if( dy > (dx*2.5) )
	{
		//adjust to vertical
		next.setX(prev.x());
	}
	else
	{	kdDebug()<<"next(x,y) = "<<next.x()<<","<<next.y()<<endl;
		next.setY(next.x());
		kdDebug()<<"next(x,y) = "<<next.x()<<","<<next.y()<<endl;
	}
	return next;
}


bool PathTool::mouseDragEvent( )
{
	return false;
}

bool PathTool::mouseDragReleaseEvent( )
{
	return false;
}

bool PathTool::mouseDblClickEvent( )
{
	if( m_linePath.count() > 0 )
	{	createPath();
		return true;
	}
	return false;
}

void PathTool::createPath( )
{
	Path *path = new Path(diagram(),diagram()->document()->getUniqueID());
	QPointArray points( m_linePath.count() + 1 );
	points[0] = m_linePath.at(0)->startPoint();
	for( uint i = 0; i < m_linePath.count(); i++ )
	{
		points[i+1] = m_linePath.at(i)->endPoint();
	}

	path->setPathPoints(points);
	path->show();
	m_linePath.clear();
	diagram()->update();
}


bool PathTool::keyPressed( int key )
{
	switch(key)
	{
		case Qt::Key_Shift:
		case Qt::Key_Alt:
		case Qt::Key_Control:
		 //we need to recalculate the path, as if the mouse had been moved
			mouseMoveEvent();
			return true;
	}
	return false;
}
bool PathTool::keyReleased( int key )
{
	switch(key)
	{
		case Qt::Key_Shift:
		case Qt::Key_Alt:
		case Qt::Key_Control:
		 //we need to recalculate the path, as if the mouse had been moved
			mouseMoveEvent();
			return true;
	}
	return false;
}

}

