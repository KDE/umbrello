/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>

// qt includes
#include <qcanvas.h>
#include <qdatastream.h>
#include <qdom.h>

// kde includes
#include <kdebug.h>

// application includes
#include "associationwidget.h"
#include "activitywidget.h"
#include "umlview.h"
#include "linepath.h"

LinePath::LinePath() {
	m_RectList.setAutoDelete( true );
	m_LineList.setAutoDelete( true );
	m_HeadList.setAutoDelete( true );
	m_ParallelList.setAutoDelete( true );
	m_bSelected = false;
	m_pClearPoly = 0;
	m_pEllipse = 0;
	m_PointArray.resize( 4 );
	m_ParallelLines.resize( 4 );
	m_pAssociation = 0;
	m_bHeadCreated = false;
	m_bParallelLineCreated = false;
}

LinePath::~LinePath() {}

void LinePath::setAssociation(AssociationWidget * association ) {
	if( !association )
		return;
	cleanup();
	m_pAssociation = association;
	createHeadLines();
	if( getAssocType() == at_Coll_Message )
		setupParallelLine();
	UMLView * view =  (UMLView *)m_pAssociation -> parent();
	connect( view, SIGNAL( sigLineColorChanged( QColor ) ), this, SLOT( setLineColor( QColor ) ) );
}

QPoint LinePath::getPoint( int pointIndex ) {
	int count = m_LineList.count();
	if( count == 0 || pointIndex > count  || pointIndex < 0)
		return QPoint( -1, -1 );

	if( pointIndex == count ) {
		QCanvasLine * line = m_LineList.last();
		return line -> endPoint();
	}
	QCanvasLine * line = m_LineList.at( pointIndex );
	return line -> startPoint();
}

bool LinePath::setPoint( int pointIndex, QPoint point ) {
	int count = m_LineList.count();
	if( count == 0 || pointIndex > count  || pointIndex < 0)
		return false;

	if( pointIndex == count) {
		QCanvasLine * line = m_LineList.last();
		QPoint p = line -> startPoint();
		line -> setPoints( p.x(), p.y(), point.x(), point.y() );
		moveSelected( pointIndex );
		update();
		return true;
	}
	if( pointIndex == 0 ) {
		QCanvasLine * line = m_LineList.first();
		QPoint p = line -> endPoint();
		line -> setPoints( point.x(), point.y(), p.x(), p.y() );
		moveSelected( pointIndex );
		update();
		return true;
	}
	QCanvasLine * line = m_LineList.at( pointIndex  );
	QPoint p = line -> endPoint();
	line -> setPoints( point.x(), point.y(), p.x(), p.y() );
	line = m_LineList.at( pointIndex - 1 );
	p = line -> startPoint();
	line -> setPoints( p.x(), p.y(), point.x(), point.y() );
	moveSelected( pointIndex );
	update();
	return true;
}

bool LinePath::isPoint( int pointIndex, QPoint point, unsigned short delta) {
	/* onLinePath doesn't return the index number we mean */
	pointIndex--;
	int count = m_LineList.count();
	if ( pointIndex >= count )
		return false;

	QCanvasLine * line = m_LineList.at( pointIndex );

	/* check if the given point is the start or end point of the line */
	if ( (
	            abs( line -> endPoint().x() - point.x() ) <= delta
	            &&
	            abs( line -> endPoint().y() - point.y() ) <= delta
	        ) || (
	            abs( line -> startPoint().x() - point.x() ) <= delta
	            &&
	            abs( line -> startPoint().y() - point.y() ) <= delta
	        ) )
		return true;

	/* check if the given point is the start or end point of the line */
	return false;
}

bool LinePath::insertPoint( int pointIndex, QPoint point ) {
	int count = m_LineList.count();
	if( count == 0 )
		return false;

	if( count == 1 || pointIndex == 1) {
		QCanvasLine * first = m_LineList.first();
		QPoint sp = first -> startPoint();
		QPoint ep = first -> endPoint();
		first -> setPoints( sp.x(), sp.y(), point.x(), point.y() );
		QCanvasLine * line = new QCanvasLine( getCanvas() );
		line -> setZ( -2 );
		line -> setPoints( point.x(), point.y(), ep.x(), ep.y() );
		line -> setPen( getPen() );
		line -> setVisible( true );
		m_LineList.insert( 1, line );
		setupSelected();
		return true;
	}
	if( count + 1 == pointIndex ) {
		QCanvasLine * before = m_LineList.last();
		QPoint sp = before -> startPoint();
		QPoint ep = before -> endPoint();
		before -> setPoints( sp.x(), sp.y(), point.x(), point.y() );
		QCanvasLine * line = new QCanvasLine( getCanvas() );
		line -> setPoints( point.x(), point.y(), ep.x(), ep.y() );
		line -> setZ( -2 );
		line -> setPen( getPen() );
		line -> setVisible( true );
		m_LineList.append( line );
		setupSelected();
		return true;
	}
	QCanvasLine * before = m_LineList.at( pointIndex - 1 );
	QPoint sp = before -> startPoint();
	QPoint ep = before -> endPoint();
	before -> setPoints( sp.x(), sp.y(), point.x(), point.y() );
	QCanvasLine * line = new QCanvasLine(getCanvas() );
	line -> setPoints( point.x(), point.y(), ep.x(), ep.y() );
	line -> setZ( -2 );
	line -> setPen( getPen() );
	line -> setVisible( true );
	m_LineList.insert( pointIndex, line );
	setupSelected();
	return true;
}

bool LinePath::removePoint( int pointIndex, QPoint point, unsigned short delta )
{
	/* get the number of line segments */
	int count = m_LineList.count();

	/* we have to reduce the pointIndex, because it is always 1 too high returned
	 * by LinePath::onLinePath */
	if (pointIndex != 0)
		pointIndex--;

	/* we don't know if the user clicked on the start- or endpoint of a
	 * line segment */
	QCanvasLine * current_line = m_LineList.at( pointIndex );
	if (abs( current_line -> endPoint().x() - point.x() ) <= delta
	    &&
	    abs( current_line -> endPoint().y() - point.y() ) <= delta)
	{
		/* the user clicked on the end point of the line;
		 * we have to make sure that this isn't the last line segment */
		if (pointIndex >= count - 1)
			return false;

		/* the next segment will get the starting point from the current one,
		 * which is going to be removed */
		QCanvasLine * next_line = m_LineList.at( pointIndex + 1 );
		QPoint startPoint = current_line -> startPoint();
		QPoint endPoint = next_line -> endPoint();
		next_line -> setPoints(startPoint.x(), startPoint.y(),
									  endPoint.x(), endPoint.y());

	} else
	if (abs( current_line -> startPoint().x() - point.x() ) <= delta
	    &&
	    abs( current_line -> startPoint().y() - point.y() ) <= delta)
	{
		/* the user clicked on the start point of the line;
		 * we have to make sure that this isn't the first line segment */
		 if (pointIndex < 1)
		 	return false;

		/* the previous segment will get the end point from the current one,
		 * which is going to be removed */
		QCanvasLine * previous_line = m_LineList.at( pointIndex - 1 );
		QPoint startPoint = previous_line -> startPoint();
		QPoint endPoint = current_line -> endPoint();
		previous_line -> setPoints(startPoint.x(), startPoint.y(),
											endPoint.x(), endPoint.y());
	} else {
		/* the user clicked neither on the start- nor on the end point of
		 * the line; this really shouldn't happen, but just make sure */
		return false;
	}


	/* remove the segment from the list */
	m_LineList.remove( pointIndex );

	return true;
}

bool LinePath::setStartEndPoints( QPoint start, QPoint end ) {
	int count = m_LineList.count();

	if( count == 0 ) {
		QCanvasLine * line = new QCanvasLine(getCanvas() );
		line -> setPoints( start.x(), start.y(), end.x(), end.y() );
		line -> setZ( -2 );
		line -> setPen( getPen() );
		line -> setVisible( true );
		m_LineList.append( line );
		return true;
	}
	bool status = setPoint( 0, start );
	if( status)
		return setPoint( count , end );
	return false;
}

int LinePath::count() {
	return m_LineList.count() + 1;
}

int LinePath::onLinePath( QPoint position ) {
	typedef QCanvasItemList ItemList;
	ItemList list = getCanvas() -> collisions( position );
	ItemList::iterator item_it;
	int index = 0;
	for( item_it = list.begin(); item_it != list.end(); ++item_it ) {
		if( ( index = m_LineList.findRef( (QCanvasLine*)*item_it ) ) != -1 )
			return index + 1;
	}//end for
	return -1;
}

void LinePath::setSelected( bool select ) {
	if(select)
		setupSelected();
	else if( m_RectList.count() > 0 )
		m_RectList.clear();
}

void LinePath::setAssocType( Uml::Association_Type type ) {
	LineListIt it( m_LineList );
	QCanvasLine * line = 0;
	while( ( line = it.current() ) ) {
		line -> setPen( getPen() );
		++it;
	}
	if( m_pClearPoly ) {
		delete m_pClearPoly;
		m_pClearPoly = 0;
	}
	createHeadLines();
	if( type == at_Coll_Message )
		setupParallelLine();
	update();
}

void LinePath::update() {
	if( m_bHeadCreated ) {
		calculateHead();
		updateHead();
	} else {
		createHeadLines();
	}

	if( getAssocType() == at_Coll_Message ) {
		if( m_bParallelLineCreated ) {
			calculateParallelLine();
			updateParallelLine();
		} else
			setupParallelLine();

	}
}

void LinePath::setLineColor( QColor color ) {
	QCanvasLine * line = 0;
	LineListIt it( m_LineList );
	while( ( line = it.current() ) ) {
		line -> setPen( QPen( color ) );
		++it;
	}
	LineListIt hit( m_HeadList );
	while( ( line = hit.current() ) ) {
		line -> setPen( QPen( color ) );
		++hit;
	}
	LineListIt pit( m_ParallelList );
	while( ( line = pit.current() ) ) {
		line -> setPen( QPen( color ) );
		++pit;
	}

	if( getAssocType() == at_Aggregation )
		if (m_pClearPoly) m_pClearPoly -> setBrush( QBrush( white ) );
	else if( getAssocType() == at_Composition )
		if (m_pClearPoly) m_pClearPoly -> setBrush( QBrush( color ) );

	if( m_pEllipse )
		m_pEllipse -> setPen( color );
}

void LinePath::moveSelected( int pointIndex ) {
	int lineCount = m_LineList.count();
	if( !m_bSelected ) {
		m_RectList.clear();
		return;
	}
	if( (int)m_RectList.count() + 1 != lineCount )
		setupSelected();
	QCanvasRectangle * rect = 0;
	QCanvasLine * line = 0;
	if( pointIndex == lineCount || lineCount == 1) {
		line = m_LineList.last();
		QPoint p = line -> endPoint();
		rect = m_RectList.last();
		rect -> setX( p.x() );
		rect -> setY( p.y() );
		rect -> setZ( 4 );
		return;
	}
	line = m_LineList.at( pointIndex );
	QPoint p = line -> startPoint();
	rect = m_RectList.at( pointIndex );
	rect -> setX( p.x() );
	rect -> setY( p.y() );
	rect -> setZ( 4 );
}

void LinePath::setupSelected() {
	m_RectList.clear();
	const int SIZE = 4;
	QCanvasLine * line = 0;
	LineListIt it( m_LineList );
	while( ( line = it.current() ) ) {
		QPoint sp = line -> startPoint();
		QCanvasRectangle * rect = new QCanvasRectangle( sp.x() - SIZE / 2, sp.y() - SIZE / 2, SIZE, SIZE, getCanvas() );
		rect -> setBrush( QBrush( blue ) );
		rect -> setPen( QPen( blue ) );
		rect -> setVisible( true );
		m_RectList.append( rect );
		++it;
	}
	//special case for last point
	line = m_LineList.last();
	QPoint p = line -> endPoint();
	QCanvasRectangle * rect = new QCanvasRectangle( p.x() - SIZE / 2, p.y() - SIZE / 2, SIZE, SIZE, getCanvas() );
	rect -> setBrush( QBrush( blue ) );
	rect -> setPen( QPen( blue ) );
	rect -> setVisible( true );
	m_RectList.append( rect );
}

QPen LinePath::getPen() {
	Uml::Association_Type type = getAssocType();
	if( type == at_Dependency || type == at_Realization || type == at_Anchor )
		return QPen( getLineColor(), 0, DashLine );
	return QPen( getLineColor() );
}

#ifdef DEBUG_ASSOCLINES
extern int calls_to_calc_head;
#endif

void LinePath::calculateHead() {
#ifdef DEBUG_ASSOCLINES
	calls_to_calc_head++;
#endif
	uint size = m_LineList.count();
	bool diamond;
	int xa, ya, xb, yb;
	uint edgePointIndex;
	if (getAssocType() == at_Aggregation || getAssocType() == at_Composition) {
		diamond = true;
		xa = getPoint(1).x();
		ya = getPoint(1).y();
		xb = getPoint(0).x();
		yb = getPoint(0).y();
		edgePointIndex = 0;
	} else {
		diamond = false;
		xa = getPoint(size - 1).x();
		ya = getPoint(size - 1).y();
		xb = getPoint(size).x();
		yb = getPoint(size).y();
		edgePointIndex = size;
	}
	double deltaX = xb - xa;
	double deltaY = yb - ya;
	double hypotenuse = sqrt(deltaX*deltaX + deltaY*deltaY); // the length
	int halfLength = 10;
	double arrowAngle = 0.5 * atan(sqrt(3.0) / 3.0);
	if (diamond) {
		arrowAngle *= 1.5;	// wider
		halfLength += 1;	// longer
	} else {
		arrowAngle *= 2;	// wider
		halfLength += 3;	// longer
	}
	double slope = atan2(deltaY, deltaX);	//slope of line
	double cosx = hypotenuse==0?1:halfLength * deltaX/hypotenuse;

	double siny = hypotenuse==0?0:halfLength * deltaY/hypotenuse;
	double arrowSlope = slope + arrowAngle;

	m_LastPoint = getPoint(edgePointIndex);
	m_ArrowPointA.setX( (int)rint(m_LastPoint.x() - halfLength * cos(arrowSlope)) );
	m_ArrowPointA.setY( (int)rint(m_LastPoint.y() - halfLength * sin(arrowSlope)) );
	arrowSlope = slope - arrowAngle;
	m_ArrowPointB.setX( (int)rint(m_LastPoint.x() - halfLength * cos(arrowSlope)) );
	m_ArrowPointB.setY( (int)rint(m_LastPoint.y() - halfLength * sin(arrowSlope)) );

	if(xa > xb)
		cosx = cosx > 0 ? cosx : cosx * -1;
	else
		cosx = cosx > 0 ? cosx * -1: cosx;

	if(ya > yb)
		siny = siny > 0 ? siny : siny * -1;
	else
		siny = siny > 0 ? siny * -1 : siny;

	m_MidPoint.setX( (int)rint(m_LastPoint.x() + cosx) );
	m_MidPoint.setY( (int)rint(m_LastPoint.y() + siny) );
	m_FullPoint.setX( (int)rint(m_LastPoint.x() + cosx * 2) );
	m_FullPoint.setY( (int)rint(m_LastPoint.y() + siny * 2) );

	m_PointArray.setPoint(0, m_LastPoint);
	m_PointArray.setPoint(1, m_ArrowPointA);
	m_PointArray.setPoint(2, m_FullPoint);
	m_PointArray.setPoint(3, m_ArrowPointB);
	if( getAssocType() == at_Realization ||
	        getAssocType() == at_Generalization ) {
		m_PointArray.setPoint( 2, m_ArrowPointB );
		m_PointArray.setPoint( 3, m_LastPoint );
	}
}

void LinePath::updateHead() {
	int count = m_HeadList.count();
	QCanvasLine * line = 0;
	switch( getAssocType() ) {
		case at_State:
		case at_Activity:
		case at_UniAssociation:
		case at_Dependency:
			if( count < 2)
				return;
			line = m_HeadList.at( 0 );
			line -> setPoints( m_LastPoint.x(), m_LastPoint.y(), m_ArrowPointA.x(), m_ArrowPointA.y() );

			line = m_HeadList.at( 1 );
			line -> setPoints( m_LastPoint.x(), m_LastPoint.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );
			break;

		case at_Generalization:
		case at_Realization:
			if( count < 3)
				return;
			line = m_HeadList.at( 0 );
			line -> setPoints( m_LastPoint.x(), m_LastPoint.y(), m_ArrowPointA.x(), m_ArrowPointA.y() );

			line = m_HeadList.at( 1 );
			line -> setPoints( m_LastPoint.x(), m_LastPoint.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );

			line = m_HeadList.at( 2 );
			line -> setPoints( m_ArrowPointA.x(), m_ArrowPointA.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );
			m_pClearPoly -> setPoints( m_PointArray );
			break;

		case at_Composition:
		case at_Aggregation:
			if( count < 4)
				return;
			line = m_HeadList.at( 0 );
			line -> setPoints( m_PointArray[ 0 ].x(), m_PointArray[ 0 ].y(), m_PointArray[ 1 ].x(), m_PointArray[ 1 ].y() );

			line = m_HeadList.at( 1 );
			line -> setPoints( m_PointArray[ 1 ].x(), m_PointArray[ 1 ].y(), m_PointArray[ 2 ].x(), m_PointArray[ 2 ].y() );

			line = m_HeadList.at( 2 );
			line -> setPoints( m_PointArray[ 2 ].x(), m_PointArray[ 2 ].y(), m_PointArray[ 3 ].x(), m_PointArray[ 3 ].y() );

			line = m_HeadList.at( 3 );
			line -> setPoints( m_PointArray[ 3 ].x(), m_PointArray[ 3 ].y(), m_PointArray[ 0 ].x(), m_PointArray[ 0 ].y() );
			m_pClearPoly -> setPoints( m_PointArray );
			break;

		case at_Implementation:
			//find which side we are on of icon to calculate circle co-ords
			if( m_MidPoint.y() <= m_LastPoint.y()) {
				m_pEllipse -> setY( m_MidPoint.y() );
			} else {
				m_pEllipse -> setY( m_LastPoint.y() );
			}

			if( m_MidPoint.x() <= m_LastPoint.x()) {
				m_pEllipse -> setX( m_MidPoint.x() );
			} else {
				m_pEllipse -> setX( m_LastPoint.x() );
			}
			break;
		default:
			break;
	}
}

void LinePath::growHeadList(int by) {
	QPen pen( getLineColor() );
	for (int i = 0; i < by; i++) {
		QCanvasLine * line = new QCanvasLine( getCanvas() );
		line -> setZ( 0 );
		line -> setPen( pen );
		line -> setVisible( true );
		m_HeadList.append( line );
	}
}

void LinePath::createHeadLines() {
	m_HeadList.clear();
	QCanvas * canvas = getCanvas();
	switch( getAssocType() ) {
		case at_Activity:
			growHeadList( 2 );
			break;

		case at_State:
		case at_Dependency:
		case at_UniAssociation:
			growHeadList( 2 );
			break;

		case at_Generalization:
		case at_Realization:
			growHeadList( 3 );
			m_pClearPoly = new QCanvasPolygon( canvas );
			m_pClearPoly -> setVisible( true );
			m_pClearPoly -> setBrush( QBrush( white ) );
			m_pClearPoly -> setZ( -1 );
			break;

		case at_Composition:
		case at_Aggregation:
			growHeadList( 4 );
			m_pClearPoly = new QCanvasPolygon( canvas );
			m_pClearPoly -> setVisible( true );
			if( getAssocType() == at_Aggregation )
				m_pClearPoly -> setBrush( QBrush( white ) );
			else
				m_pClearPoly -> setBrush( QBrush( getLineColor() ) );
			m_pClearPoly -> setZ( -1 );
			break;

		case at_Implementation:
			if (!m_pEllipse) {
				m_pEllipse = new QCanvasEllipse( canvas );
				m_pEllipse -> setVisible( true );
				m_pEllipse -> setBrush( QBrush( getLineColor() ) );
				m_pEllipse -> setSize( 10, 10 );
			}
			break;
		default:
			break;
	}
	m_bHeadCreated = true;
}

void LinePath::calculateParallelLine() {
	int midCount = count() / 2;
	double ATAN = atan(1.0);
	int lineDist = 10;
	//get  1/8(M) and 7/8(T) point
	QPoint a = getPoint( midCount - 1 );
	QPoint b = getPoint( midCount );
	int mx = ( a.x() + b.x() ) / 2;
	int my = ( a.y() + b.y() ) / 2;
	int tx = ( mx + b.x() ) / 2;
	int ty = ( my + b.y() ) / 2;
	//find dist between M and T points
	int distX = ( mx - tx );
	distX *= distX;
	int distY = ( my - ty );
	distY *= distY;
	double dist = sqrt( double(distX + distY) );
	double angle = atan2( double(ty - my), double(tx - mx) ) + ( ATAN * 2 );
	//find point from M to start line from.
	double cosx = cos( angle ) * lineDist;
	double siny = sin( angle ) * lineDist;
	QPoint pointM( mx + (int)cosx, my + (int)siny );
	//find dist between P(xb, yb)
	distX = ( tx - b.x() );
	distX *= distX;
	distY = ( ty - b.y() );
	distY *= distY;
	dist = sqrt( double(distX + distY) );
	//find point from T to end line
	cosx = cos( angle ) * lineDist;
	siny = sin( angle ) * lineDist;
	QPoint pointT( tx + (int)cosx, ty + (int)siny );
	m_ParallelLines[ 1 ] = pointM;
	m_ParallelLines[ 0 ] = pointT;

	int arrowDist = 5;
	angle = atan2( double(pointT.y() - pointM.y()),
						double(pointT.x() - pointM.x()) );
	double arrowSlope = angle + ATAN;
	cosx = ( cos( arrowSlope ) ) * arrowDist;
	siny = ( sin( arrowSlope ) ) * arrowDist;
	m_ParallelLines[ 2 ] = QPoint( pointT.x() - (int)cosx, pointT.y() - (int)siny );
	arrowSlope = angle - ATAN;
	cosx = ( cos( arrowSlope )  ) * arrowDist;
	siny = ( sin( arrowSlope ) ) * arrowDist;
	m_ParallelLines[ 3 ] = QPoint( pointT.x() - (int)cosx, pointT.y() - (int)siny );
}

void LinePath::setupParallelLine() {
	m_ParallelList.clear();
	QCanvas * canvas = getCanvas();
	QPen pen( getLineColor() );
	QCanvasLine * line = new QCanvasLine( canvas );
	line -> setZ( 0 );
	line -> setPen( pen );
	line -> setVisible( true );
	m_ParallelList.append( line );

	line = new QCanvasLine( canvas );
	line -> setZ( 0 );
	line -> setPen( pen );
	line -> setVisible( true );
	m_ParallelList.append( line );

	line = new QCanvasLine( canvas );
	line -> setZ( 0 );
	line -> setPen( pen );
	line -> setVisible( true );
	m_ParallelList.append( line );
	m_bParallelLineCreated = true;
}

void LinePath::updateParallelLine() {
	if( !m_bParallelLineCreated )
		return;
	QCanvasLine * line = 0;
	QPoint common = m_ParallelLines.at( 0 );
	QPoint p = m_ParallelLines.at( 1 );
	line = m_ParallelList.at( 0 );
	line -> setPoints( common.x(), common.y(), p.x(), p.y() );

	p = m_ParallelLines.at( 2 );
	line = m_ParallelList.at( 1 );
	line -> setPoints( common.x(), common.y(), p.x(), p.y() );

	p = m_ParallelLines.at( 3 );
	line = m_ParallelList.at( 2 );
	line -> setPoints( common.x(), common.y(), p.x(), p.y() );
}

bool LinePath::operator==( LinePath & rhs ) {
	if( this -> m_LineList.count() != rhs.m_LineList.count() )
		return false;

	//Check to see if all points at the same position
	for( int i = 0; i< rhs.count() ; i++ ) {
		if( this -> getPoint( i ) != rhs.getPoint( i ) )
			return false;
	}
	return true;
}

LinePath & LinePath::operator=( LinePath & rhs ) {
	if( this == &rhs )
		return *this;
	//clear out the old canvas objects
	this -> m_LineList.clear();
	this -> m_ParallelList.clear();
	this -> m_RectList.clear();
	this -> m_HeadList.clear();
	int count = rhs.m_LineList.count();
	//setup start end points
	this -> setStartEndPoints( rhs.getPoint( 0 ), rhs.getPoint( count) );
	//now insert the rest
	for( int i = 1; i < count ; i++ ) {
		this -> insertPoint( i, rhs.getPoint ( i ) );
	}
	this -> setAssocType( rhs.getAssocType() );

	return *this;
}

QCanvas * LinePath::getCanvas() {
	if( !m_pAssociation )
		return 0;
	UMLView * view =  (UMLView *)m_pAssociation -> parent();
	return view -> canvas();
}

Uml::Association_Type LinePath::getAssocType() {
	if( m_pAssociation )
		return m_pAssociation -> getAssocType();
	return Uml::at_Association;
}

QColor LinePath::getLineColor() {
	if( !m_pAssociation )
		return black;
	UMLView * view =  (UMLView *)m_pAssociation -> parent();
	return view -> getLineColor();
}

void LinePath::cleanup() {
	if (m_pAssociation)
		m_LineList.clear();
	m_HeadList.clear();
	m_RectList.clear();
	m_ParallelList.clear();

	if( m_pClearPoly )
		delete m_pClearPoly;
	if( m_pEllipse )
		delete m_pEllipse;
	m_pEllipse = 0;
	m_pClearPoly = 0;
	m_bHeadCreated = m_bParallelLineCreated = false;
	if( m_pAssociation ) {
		UMLView * view =  (UMLView *)m_pAssociation -> parent();
		if(view)
			disconnect( view, SIGNAL( sigLineColorChanged( QColor ) ), this, SLOT( setLineColor( QColor ) ) );
		m_pAssociation = NULL;
	}
}

bool LinePath::hasPoints () {
	int count = m_LineList.count();
	if (count>1)
		return true;
	return false;
}
void LinePath::dumpPoints () {
	int count = m_LineList.count();
	for( int i = 1; i < count; i++ ) {
		QPoint point = getPoint( i );
		kdDebug()<<" * point x:"<<point.x()<<" y:"<<point.y()<<endl;
	}
}

bool LinePath::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	int count = m_LineList.count();
	QPoint point = getPoint( 0 );
	QDomElement lineElement = qDoc.createElement( "linepath" );
	QDomElement startElement = qDoc.createElement( "startpoint" );
	startElement.setAttribute( "startx", point.x() );
	startElement.setAttribute( "starty", point.y() );
	lineElement.appendChild( startElement );
	QDomElement endElement = qDoc.createElement( "endpoint" );
	point = getPoint( count );
	endElement.setAttribute( "endx", point.x() );
	endElement.setAttribute( "endy", point.y() );
	lineElement.appendChild( endElement );
	for( int i = 1; i < count; i++ ) {
		QDomElement pointElement = qDoc.createElement( "point" );
		point = getPoint( i );
		pointElement.setAttribute( "x", point.x() );
		pointElement.setAttribute( "y", point.y() );
		lineElement.appendChild( pointElement );
	}
	qElement.appendChild( lineElement );
	return true;
}

bool LinePath::loadFromXMI( QDomElement & qElement ) {
	QDomNode node = qElement.firstChild();
	QDomElement startElement = node.toElement();
	if( startElement.isNull() || startElement.tagName() != "startpoint" )
		return false;
	QString x = startElement.attribute( "startx", "0" );
	int nX = x.toInt();
	QString y = startElement.attribute( "starty", "0" );
	int nY = y.toInt();
	QPoint startPoint( nX, nY );

	node = startElement.nextSibling();
	QDomElement endElement = node.toElement();
	if( endElement.isNull() || endElement.tagName() != "endpoint" )
		return false;
	x = endElement.attribute( "endx", "0" );
	nX = x.toInt();
	y = endElement.attribute( "endy", "0" );
	nY = y.toInt();
	QPoint endPoint( nX, nY );
	setStartEndPoints( startPoint, endPoint );
	QPoint point;
	node = endElement.nextSibling();
	QDomElement element = node.toElement();
	int i = 1;
	while( !element.isNull() ) {
		if( element.tagName() == "point" ) {
			x = element.attribute( "x", "0" );
			y = element.attribute( "y", "0" );
			point.setX( x.toInt() );
			point.setY( y.toInt() );
			insertPoint( i++, point );
		}
		node = element.nextSibling();
		element = node.toElement();
	}

	return true;
}


void LinePath::activate() {
	int count = m_LineList.count();
	if (count == 0)
		return;
	QCanvas * canvas = getCanvas();
	if (canvas == NULL)
		return;
	for (int i = 0; i < count ; i++) {
		QCanvasLine *line = m_LineList.at(i);
		line -> setCanvas( canvas );
		line -> setPen( getPen() );
	}
}



#include "linepath.moc"
