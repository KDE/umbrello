/***************************************************************************
                                  pathsegment.cpp
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

#include "pathsegment.h"
#include "path.h"

#include <qpainter.h>
#include <qpoint.h>
#include <qrect.h>

namespace Umbrello{

PathSegment::PathSegment( Path *path ) : QCanvasLine( path->diagram() ), m_path(path)
{
}

PathSegment::~PathSegment()
{
	hide();
}

void PathSegment::moveBy( double dx, double dy)
{
	setPoints( startPoint().x() + (int)dx, startPoint().y() + (int)dy,
	             endPoint().x() + (int)dx,   endPoint().y() + (int)dy );
}


void PathSegment::drawShape(QPainter &p )
{
	QCanvasLine::drawShape(p);
/*	QPen  pen(p.pen());
	QBrush brush(p.brush());
 	if(m_path->isSelected())
	{
		p.setPen(Qt::blue);
		p.setBrush(Qt::blue);
		QRect selectionRect(0,0,2,2);
			selectionRect.moveCenter(QPoint((int)startPoint().x(),(int)startPoint().y()));
		p.drawRect(selectionRect);
			selectionRect.moveCenter(QPoint((int)endPoint().x(),(int)endPoint().y()));
		p.drawRect(selectionRect);
	}
	p.setPen(pen);
	p.setBrush(brush);*/
}

QPointArray PathSegment::areaPoints() const
{
//code from QCanvasLine::areaPoints() - (c) Trolltech
    QPointArray p(4);
    int xi = int(x());
    int yi = int(y());
    ////////// make the area thicker - the size of the hotspots
    int pw = pen().width() + m_path->hotSpotSize( );
    int x1 = startPoint().x();
    int x2 = endPoint().x();
    int y1 = startPoint().y();
    int y2 = endPoint().y();
    ////////////////////
    int dx = QABS(x1-x2);
    int dy = QABS(y1-y2);
    pw = pw*4/3+2; // approx pw*sqrt(2)
    int px = x1<x2 ? -pw : pw ;
    int py = y1<y2 ? -pw : pw ;
    if ( dx && dy && (dx > dy ? (dx*2/dy <= 2) : (dy*2/dx <= 2)) ) {
	// steep
	if ( px == py ) {
	    p[0] = QPoint(x1+xi   ,y1+yi+py);
	    p[1] = QPoint(x2+xi-px,y2+yi   );
	    p[2] = QPoint(x2+xi   ,y2+yi-py);
	    p[3] = QPoint(x1+xi+px,y1+yi   );
	} else {
	    p[0] = QPoint(x1+xi+px,y1+yi   );
	    p[1] = QPoint(x2+xi   ,y2+yi-py);
	    p[2] = QPoint(x2+xi-px,y2+yi   );
	    p[3] = QPoint(x1+xi   ,y1+yi+py);
	}
    } else if ( dx > dy ) {
	// horizontal
	p[0] = QPoint(x1+xi+px,y1+yi+py);
	p[1] = QPoint(x2+xi-px,y2+yi+py);
	p[2] = QPoint(x2+xi-px,y2+yi-py);
	p[3] = QPoint(x1+xi+px,y1+yi-py);
    } else {
	// vertical
	p[0] = QPoint(x1+xi+px,y1+yi+py);
	p[1] = QPoint(x2+xi+px,y2+yi-py);
	p[2] = QPoint(x2+xi-px,y2+yi-py);
	p[3] = QPoint(x1+xi-px,y1+yi+py);
    }
    return p;

}



} //end of namespace Umbrello

