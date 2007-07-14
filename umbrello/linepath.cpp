/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "linepath.h"

// system includes
#include <cstdlib>
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
#include "widget_utils.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"

LinePath::Circle::Circle(QCanvas * canvas, int radius /* = 0 */)
        : QCanvasEllipse(radius * 2, radius * 2, canvas) {
}

void LinePath::Circle::setX(int x) {
    QCanvasItem::setX( (double) x );
}

void LinePath::Circle::setY(int y) {
    QCanvasItem::setY( (double) y );
}

void LinePath::Circle::setRadius(int radius) {
    QCanvasEllipse::setSize(radius * 2, radius * 2);
}

int LinePath::Circle::getRadius() const {
    return (QCanvasEllipse::height() / 2);
}

void LinePath::Circle::drawShape(QPainter& p) {
    int diameter = height();
    int radius = diameter / 2;
    p.drawEllipse( (int)x() - radius, (int)y() - radius, diameter, diameter);
}

LinePath::LinePath() {
    m_RectList.setAutoDelete( true );
    m_LineList.setAutoDelete( true );
    m_HeadList.setAutoDelete( true );
    m_ParallelList.setAutoDelete( true );
    m_bSelected = false;
    m_pClearPoly = 0;
    m_pCircle = 0;
    m_PointArray.resize( 4 );
    m_ParallelLines.resize( 4 );
    m_pAssociation = 0;
    m_bHeadCreated = false;
    m_bParallelLineCreated = false;
    m_DockRegion = TopBottom;
}

LinePath::~LinePath() {}

void LinePath::setAssociation(AssociationWidget * association ) {
    if( !association )
        return;
    cleanup();
    m_pAssociation = association;
    createHeadLines();
    if( getAssocType() == Uml::at_Coll_Message )
        setupParallelLine();
    UMLView * view =  (UMLView *)m_pAssociation -> parent();
    connect( view, SIGNAL( sigColorChanged( Uml::IDType ) ), this, SLOT( slotLineColorChanged( Uml::IDType ) ) );
    connect( view, SIGNAL( sigLineWidthChanged( Uml::IDType ) ), this, SLOT( slotLineWidthChanged( Uml::IDType ) ) );
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

bool LinePath::setPoint( int pointIndex, const QPoint &point ) {
    int count = m_LineList.count();
    if( count == 0 || pointIndex > count  || pointIndex < 0)
        return false;
    if (point.x() == 0 && point.y() == 0) {
        kError() << "LinePath::setPoint:ignoring request for (0,0)" << endl;
        return false;
    }

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

bool LinePath::isPoint( int pointIndex, const QPoint &point, unsigned short delta) {
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

bool LinePath::insertPoint( int pointIndex, const QPoint &point ) {
    int count = m_LineList.count();
    if( count == 0 )
        return false;
    const bool bLoading = UMLApp::app()->getDocument()->loading();

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
        if (!bLoading)
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
        if (!bLoading)
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
    if (!bLoading)
        setupSelected();
    return true;
}

bool LinePath::removePoint( int pointIndex, const QPoint &point, unsigned short delta )
{
    /* get the number of line segments */
    int count = m_LineList.count();
    if ( pointIndex >= count )
        return false;

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

bool LinePath::setStartEndPoints( const QPoint &start, const QPoint &end ) {
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

int LinePath::onLinePath( const QPoint &position ) {
    QCanvasItemList list = getCanvas() -> collisions( position );
    int index = -1;

    QCanvasItemList::iterator end(list.end());
    for(QCanvasItemList::iterator item_it(list.begin()); item_it != end; ++item_it ) {
        if( ( index = m_LineList.findRef( (QCanvasLine*)*item_it ) ) != -1 )
            break;
    }//end for
    return index;
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
    if( type == Uml::at_Coll_Message )
        setupParallelLine();
    else
        createHeadLines();
    update();
}

void LinePath::update() {
    if (getAssocType() == Uml::at_Coll_Message) {
        if (m_bParallelLineCreated) {
            calculateParallelLine();
            updateParallelLine();
        } else
            setupParallelLine();
    } else if (m_bHeadCreated) {
        calculateHead();
        updateHead();
    } else {
        createHeadLines();
    }
}

void LinePath::slotLineColorChanged( Uml::IDType viewID ) {
    if(m_pAssociation->getUMLView()->getID() != viewID) {
        return;
    }
    setLineColor( m_pAssociation->getUMLView()->getLineColor() );
}


void LinePath::setLineColor( const QColor &color ) {
    QCanvasLine * line = 0;
    uint linewidth = 0;
    LineListIt it( m_LineList );
    while( ( line = it.current() ) ) {
        linewidth = line->pen().width();
        line -> setPen( QPen( color, linewidth ) );
        ++it;
    }
    LineListIt hit( m_HeadList );
    while( ( line = hit.current() ) ) {
        linewidth = line->pen().width();
        line -> setPen( QPen( color, linewidth ) );
        ++hit;
    }
    LineListIt pit( m_ParallelList );
    while( ( line = pit.current() ) ) {
        linewidth = line->pen().width();
        line -> setPen( QPen( color, linewidth ) );
        ++pit;
    }

    if( getAssocType() == Uml::at_Aggregation )
        if (m_pClearPoly) m_pClearPoly -> setBrush( QBrush( Qt::white ) );
        else if( getAssocType() == Uml::at_Composition )
            if (m_pClearPoly) m_pClearPoly -> setBrush( QBrush( color ) );

    if( m_pCircle ) {
        linewidth = m_pCircle->pen().width();
        m_pCircle->setPen( QPen(color, linewidth) );
    }
}

void LinePath::slotLineWidthChanged( Uml::IDType viewID ) {
    if(m_pAssociation->getUMLView()->getID() != viewID) {
        return;
    }
    setLineWidth( m_pAssociation->getUMLView()->getLineWidth() );
}

void LinePath::setLineWidth( uint width ) {
    QCanvasLine * line = 0;
    QColor linecolor;
    LineListIt it( m_LineList );
    while( ( line = it.current() ) ) {
        linecolor = line->pen().color();
        line -> setPen( QPen( linecolor, width ) );
        ++it;
    }
    LineListIt hit( m_HeadList );
    while( ( line = hit.current() ) ) {
        linecolor = line->pen().color();
        line -> setPen( QPen( linecolor, width ) );
        ++hit;
    }
    LineListIt pit( m_ParallelList );
    while( ( line = pit.current() ) ) {
        linecolor = line->pen().color();
        line -> setPen( QPen( linecolor, width ) );
        ++pit;
    }

    if( m_pCircle ) {
        linecolor = m_pCircle->pen().color();
        m_pCircle->setPen( QPen(linecolor, width) );
    }
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
    QCanvasLine * line = 0;
    LineListIt it( m_LineList );
    while( ( line = it.current() ) ) {
        QPoint sp = line -> startPoint();
        QCanvasRectangle *rect = Widget_Utils::decoratePoint(sp);
        m_RectList.append( rect );
        ++it;
    }
    //special case for last point
    line = m_LineList.last();
    QPoint p = line -> endPoint();
    QCanvasRectangle *rect = Widget_Utils::decoratePoint(p);
    m_RectList.append( rect );
    update();
}

QPen LinePath::getPen() {
    Uml::Association_Type type = getAssocType();
    if( type == Uml::at_Dependency || type == Uml::at_Realization || type == Uml::at_Anchor )
        return QPen( getLineColor(), getLineWidth(), Qt::DashLine );
    return QPen( getLineColor(), getLineWidth() );
}

void LinePath::calculateHead() {
    uint size = m_LineList.count();
    QPoint farPoint;
    int halfLength = 10;
    double arrowAngle = 0.2618;   // 0.5 * atan(sqrt(3.0) / 3.0) = 0.2618
    Uml::Association_Type at = getAssocType();
    bool diamond = (at == Uml::at_Aggregation || at == Uml::at_Composition);
    if (diamond || at == Uml::at_Containment) {
        farPoint = getPoint(1);
        m_EgdePoint = getPoint(0);
        if (diamond) {
            arrowAngle *= 1.5;  // wider
            halfLength += 1;    // longer
        } else {
            // Containment has a circle-plus symbol at the
            // containing object.  What we are tweaking here
            // is the perpendicular line through the circle
            // (i.e. the horizontal line of the plus sign if
            // the objects are oriented north/south)
            arrowAngle *= 2.5;  // wider
            halfLength -= 4;    // shorter
        }
    } else {
        farPoint = getPoint(size - 1);
        m_EgdePoint = getPoint(size);
        // We have an arrow.
        arrowAngle *= 2.0;      // wider
        halfLength += 3;        // longer
    }
    int xa = farPoint.x();
    int ya = farPoint.y();
    int xb = m_EgdePoint.x();
    int yb = m_EgdePoint.y();
    double deltaX = xb - xa;
    double deltaY = yb - ya;
    double hypotenuse = sqrt(deltaX*deltaX + deltaY*deltaY); // the length
    double slope = atan2(deltaY, deltaX);       //slope of line
    double arrowSlope = slope + arrowAngle;
    double cosx, siny;
    if (hypotenuse < 1.0e-6) {
        cosx = 1.0;
        siny = 0.0;
    } else {
        cosx = halfLength * deltaX/hypotenuse;
        siny = halfLength * deltaY/hypotenuse;
    }

    m_ArrowPointA.setX( (int)rint(xb - halfLength * cos(arrowSlope)) );
    m_ArrowPointA.setY( (int)rint(yb - halfLength * sin(arrowSlope)) );
    arrowSlope = slope - arrowAngle;
    m_ArrowPointB.setX( (int)rint(xb - halfLength * cos(arrowSlope)) );
    m_ArrowPointB.setY( (int)rint(yb - halfLength * sin(arrowSlope)) );

    if(xa > xb)
        cosx = cosx > 0 ? cosx : cosx * -1;
    else
        cosx = cosx > 0 ? cosx * -1: cosx;

    if(ya > yb)
        siny = siny > 0 ? siny : siny * -1;
    else
        siny = siny > 0 ? siny * -1 : siny;

    m_MidPoint.setX( (int)rint(xb + cosx) );
    m_MidPoint.setY( (int)rint(yb + siny) );

    m_PointArray.setPoint(0, m_EgdePoint);
    m_PointArray.setPoint(1, m_ArrowPointA);
    if( getAssocType() == Uml::at_Realization ||
            getAssocType() == Uml::at_Generalization ) {
        m_PointArray.setPoint( 2, m_ArrowPointB );
        m_PointArray.setPoint( 3, m_EgdePoint );
    } else {
        QPoint diamondFarPoint;
        diamondFarPoint.setX( (int)rint(xb + cosx * 2) );
        diamondFarPoint.setY( (int)rint(yb + siny * 2) );
        m_PointArray.setPoint(2, diamondFarPoint);
        m_PointArray.setPoint(3, m_ArrowPointB);
    }

}

void LinePath::updateHead() {
    int count = m_HeadList.count();
    QCanvasLine * line = 0;

    switch( getAssocType() ) {
    case Uml::at_State:
    case Uml::at_Activity:
    case Uml::at_UniAssociation:
    case Uml::at_Dependency:
        if( count < 2)
            return;
        line = m_HeadList.at( 0 );
        line -> setPoints( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointA.x(), m_ArrowPointA.y() );

        line = m_HeadList.at( 1 );
        line -> setPoints( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );
        break;

    case Uml::at_Relationship:
        if (count < 2) {
            return;
        }
        {
            int xoffset = 0;
            int yoffset = 0;
            if( m_DockRegion == TopBottom )
                xoffset = 8;
            else
                yoffset = 8;
            line = m_HeadList.at( 0 );
            line->setPoints( m_PointArray[2].x(), m_PointArray[2].y(),
                             m_PointArray[0].x()-xoffset, m_PointArray[0].y()-yoffset );

            line = m_HeadList.at( 1 );
            line->setPoints( m_PointArray[2].x(), m_PointArray[2].y(),
                             m_PointArray[0].x()+xoffset, m_PointArray[0].y()+yoffset );
        }

    case Uml::at_Generalization:
    case Uml::at_Realization:
        if( count < 3)
            return;
        line = m_HeadList.at( 0 );
        line -> setPoints( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointA.x(), m_ArrowPointA.y() );

        line = m_HeadList.at( 1 );
        line -> setPoints( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );

        line = m_HeadList.at( 2 );
        line -> setPoints( m_ArrowPointA.x(), m_ArrowPointA.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );
        m_pClearPoly -> setPoints( m_PointArray );
        break;

    case Uml::at_Composition:
    case Uml::at_Aggregation:
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

    case Uml::at_Containment:
        if (count < 1)
            return;
        line = m_HeadList.at( 0 );
        line->setPoints( m_PointArray[ 1 ].x(), m_PointArray[ 1 ].y(),
                         m_PointArray[ 3 ].x(), m_PointArray[ 3 ].y() );
        m_pCircle -> setX( m_MidPoint.x() );
        m_pCircle -> setY( m_MidPoint.y() );
        break;
    default:
        break;
    }
}

void LinePath::growList(LineList &list, int by) {
    QPen pen( getLineColor(), getLineWidth() );
    for (int i = 0; i < by; i++) {
        QCanvasLine * line = new QCanvasLine( getCanvas() );
        line -> setZ( 0 );
        line -> setPen( pen );
        line -> setVisible( true );
        list.append( line );
    }
}

void LinePath::createHeadLines() {
    m_HeadList.clear();
    QCanvas * canvas = getCanvas();
    switch( getAssocType() ) {
    case Uml::at_Activity:
    case Uml::at_State:
    case Uml::at_Dependency:
    case Uml::at_UniAssociation:
    case Uml::at_Relationship:
        growList(m_HeadList, 2);
        break;

    case Uml::at_Generalization:
    case Uml::at_Realization:
        growList(m_HeadList, 3);
        m_pClearPoly = new QCanvasPolygon( canvas );
        m_pClearPoly -> setVisible( true );
        m_pClearPoly -> setBrush( QBrush( Qt::white ) );
        m_pClearPoly -> setZ( -1 );
        break;

    case Uml::at_Composition:
    case Uml::at_Aggregation:
        growList(m_HeadList, 4);
        m_pClearPoly = new QCanvasPolygon( canvas );
        m_pClearPoly -> setVisible( true );
        if( getAssocType() == Uml::at_Aggregation )
            m_pClearPoly -> setBrush( QBrush( Qt::white ) );
        else
            m_pClearPoly -> setBrush( QBrush( getLineColor() ) );
        m_pClearPoly -> setZ( -1 );
        break;

    case Uml::at_Containment:
        growList(m_HeadList, 1);
        if (!m_pCircle) {
            m_pCircle = new Circle( canvas, 6 );
            m_pCircle->show();
            m_pCircle->setPen( QPen( getLineColor(), getLineWidth() ) );
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
    growList(m_ParallelList, 3);
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
    const UMLView * view =  m_pAssociation->getUMLView();
    return view -> canvas();
}

Uml::Association_Type LinePath::getAssocType() {
    if( m_pAssociation )
        return m_pAssociation -> getAssocType();
    return Uml::at_Association;
}

QColor LinePath::getLineColor() {
    if( !m_pAssociation )
        return Qt::black;
    return m_pAssociation -> getLineColor();
}

uint LinePath::getLineWidth() {
    if( !m_pAssociation )
        return 0;
    int viewLineWidth = m_pAssociation -> getLineWidth();
    if ( viewLineWidth >= 0 && viewLineWidth <= 10 )
        return viewLineWidth;
    else {
        kWarning() << "Ignore wrong LineWidth of " << viewLineWidth
        << " in LinePath::getLineWidth" << endl;
        return 0;
    }
}

void LinePath::cleanup() {
    if (m_pAssociation)
        m_LineList.clear();
    m_HeadList.clear();
    m_RectList.clear();
    m_ParallelList.clear();

    if( m_pClearPoly )
        delete m_pClearPoly;
    if( m_pCircle )
        delete m_pCircle;
    m_pCircle = 0;
    m_pClearPoly = 0;
    m_bHeadCreated = m_bParallelLineCreated = false;
    if( m_pAssociation ) {
        UMLView * view =  (UMLView *)m_pAssociation -> parent();
        if(view) {
            disconnect( view, SIGNAL( sigColorChanged( Uml::IDType ) ), this, SLOT( slotLineColorChanged( Uml::IDType ) ) );
            disconnect( view, SIGNAL( sigLineWidthChanged( Uml::IDType ) ), this, SLOT( slotLineWidthChanged( Uml::IDType ) ) );
        }
        m_pAssociation = NULL;
    }
}

void LinePath::setDockRegion( Region region ) {
    m_DockRegion = region;
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
        kDebug()<<" * point x:"<<point.x()<<" y:"<<point.y()<<endl;
    }
}

void LinePath::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
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
