/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "associationline.h"

// system includes
#include <cstdlib>
#include <cmath>

// qt includes
#include <QtXml/QDomDocument>
#include <QtGui/QPainter>

// application includes
#include "associationwidget.h"
#include "activitywidget.h"
#include "debug_utils.h"
#include "widget_utils.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"

AssociationLine::Circle::Circle(UMLViewCanvas * canvas, int radius /* = 0 */)
        : UMLSceneEllipse(radius * 2, radius * 2, canvas)
{
}

void AssociationLine::Circle::setX(int x)
{
    UMLSceneItem::setX( (double) x );
}

void AssociationLine::Circle::setY(int y)
{
    UMLSceneItem::setY( (double) y );
}

void AssociationLine::Circle::setRadius(int radius)
{
    UMLSceneEllipse::setSize(radius * 2, radius * 2);
}

int AssociationLine::Circle::getRadius() const
{
    return (UMLSceneEllipse::height() / 2);
}

void AssociationLine::Circle::drawShape(QPainter& p)
{
    int diameter = height();
    int radius = diameter / 2;
    p.drawEllipse( (int)x() - radius, (int)y() - radius, diameter, diameter);
}

/**
 * Constructor.
 */
AssociationLine::AssociationLine()
{
    m_bSelected = false;
    m_pClearPoly = 0;
    m_pCircle = 0;
    m_pSubsetSymbol = 0;
    m_PointArray.resize( 4 );
    m_ParallelLines.resize( 4 );
    m_pAssociation = 0;
    m_bHeadCreated = false;
    m_bSubsetSymbolCreated = false;
    m_bParallelLineCreated = false;
    m_DockRegion = TopBottom;
}

/**
 * Destructor.
 */
AssociationLine::~AssociationLine()
{
}

/**
 * This will setup the class ready to display the line correctly.
 * This MUST be called before you can use this class.
 */
void AssociationLine::setAssociation(AssociationWidget * association)
{
    if( !association )
        return;
    cleanup();
    m_pAssociation = association;
    createHeadLines();
    createSubsetSymbol();
    if( getAssocType() == Uml::AssociationType::Coll_Message )
        setupParallelLine();
    UMLView * view =  (UMLView *)m_pAssociation->parent();
    if (view) {
        connect(view, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotLineColorChanged(Uml::IDType)));
        connect(view, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));
    }
    else {
        uWarning() << "Parent is null. Can not connect SIGNAL/SLOT.";
    }

}

/**
 * Returns the point at the point index.
 */
QPoint AssociationLine::getPoint( int pointIndex ) const 
{
    int count = m_LineList.count();
    if( count == 0 || pointIndex > count  || pointIndex < 0)
        return QPoint( -1, -1 );

    if( pointIndex == count ) {
        UMLSceneLine * line = m_LineList.last();
        return line -> endPoint();
    }
    UMLSceneLine * line = m_LineList.at( pointIndex );
    return line -> startPoint();
}

/**
 * Sets the position of an already set point.
 */
bool AssociationLine::setPoint( int pointIndex, const QPoint &point )
{
    int count = m_LineList.count();
    if( count == 0 || pointIndex > count  || pointIndex < 0)
        return false;
    if (point.x() == 0 && point.y() == 0) {
        uError() << "ignoring request for (0,0)";
        return false;
    }

    if( pointIndex == count) {
        UMLSceneLine * line = m_LineList.last();
        QPoint p = line -> startPoint();
        line -> setPoints( p.x(), p.y(), point.x(), point.y() );
        moveSelected( pointIndex );
        update();
        return true;
    }
    if( pointIndex == 0 ) {
        UMLSceneLine * line = m_LineList.first();
        QPoint p = line -> endPoint();
        line -> setPoints( point.x(), point.y(), p.x(), p.y() );
        moveSelected( pointIndex );
        update();
        return true;
    }
    UMLSceneLine * line = m_LineList.at( pointIndex  );
    QPoint p = line -> endPoint();
    line -> setPoints( point.x(), point.y(), p.x(), p.y() );
    line = m_LineList.at( pointIndex - 1 );
    p = line -> startPoint();
    line -> setPoints( p.x(), p.y(), point.x(), point.y() );
    moveSelected( pointIndex );
    update();
    return true;
}

/**
 * Checks, if we are at an end of the segment or somewhere in the middle.
 * We use the delta, because with the mouse it is hard to find the
 * exactly point.
 */
bool AssociationLine::isPoint( int pointIndex, const QPoint &point, unsigned short delta)
{
    int count = m_LineList.count();
    if ( pointIndex >= count )
        return false;

    UMLSceneLine * line = m_LineList.at( pointIndex );

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

/**
 * Inserts a point at the given index.
 */
bool AssociationLine::insertPoint( int pointIndex, const QPoint &point )
{
    int count = m_LineList.count();
    if( count == 0 )
        return false;
    const bool bLoading = UMLApp::app()->document()->loading();

    if( count == 1 || pointIndex == 1) {
        UMLSceneLine * first = m_LineList.first();
        QPoint sp = first -> startPoint();
        QPoint ep = first -> endPoint();
        first -> setPoints( sp.x(), sp.y(), point.x(), point.y() );
        UMLSceneLine * line = new UMLSceneLine( getScene() );
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
        UMLSceneLine * before = m_LineList.last();
        QPoint sp = before -> startPoint();
        QPoint ep = before -> endPoint();
        before -> setPoints( sp.x(), sp.y(), point.x(), point.y() );
        UMLSceneLine * line = new UMLSceneLine( getScene() );
        line -> setPoints( point.x(), point.y(), ep.x(), ep.y() );
        line -> setZ( -2 );
        line -> setPen( getPen() );
        line -> setVisible( true );
        m_LineList.append( line );
        if (!bLoading)
            setupSelected();
        return true;
    }
    UMLSceneLine * before = m_LineList.at( pointIndex - 1 );
    QPoint sp = before -> startPoint();
    QPoint ep = before -> endPoint();
    before -> setPoints( sp.x(), sp.y(), point.x(), point.y() );
    UMLSceneLine * line = new UMLSceneLine(getScene() );
    line -> setPoints( point.x(), point.y(), ep.x(), ep.y() );
    line -> setZ( -2 );
    line -> setPen( getPen() );
    line -> setVisible( true );
    m_LineList.insert( pointIndex, line );
    if (!bLoading)
        setupSelected();
    return true;
}

/**
 * Removes the point on the line given by the index, at the coordinates
 * given by point with a fuzzy of delta.
 */
bool AssociationLine::removePoint( int pointIndex, const QPoint &point, unsigned short delta )
{
    /* get the number of line segments */
    int count = m_LineList.count();
    if ( pointIndex >= count )
        return false;

    if (!point.isNull()) {
        /* we don't know if the user clicked on the start- or endpoint of a
        * line segment */
        UMLSceneLine * current_line = m_LineList.at( pointIndex );
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
            UMLSceneLine * next_line = m_LineList.at( pointIndex + 1 );
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
                UMLSceneLine * previous_line = m_LineList.at( pointIndex - 1 );
                QPoint startPoint = previous_line -> startPoint();
                QPoint endPoint = current_line -> endPoint();
                previous_line -> setPoints(startPoint.x(), startPoint.y(),
                                        endPoint.x(), endPoint.y());
            } else {
                /* the user clicked neither on the start- nor on the end point of
                * the line; this really shouldn't happen, but just make sure */
                return false;
            }
    }
    /* remove the segment from the list */
    delete m_LineList.takeAt( pointIndex );

    return true;
}

/**
 * Sets the start and end points.
 */
bool AssociationLine::setStartEndPoints( const QPoint &start, const QPoint &end )
{
    int count = m_LineList.count();
    if( count == 0 ) {
        UMLSceneLine * line = new UMLSceneLine(getScene() );
        line -> setPoints( start.x(), start.y(),end.x(),end.y() );
        line -> setZ( -2 );
        line -> setPen( getPen() );
        line -> setVisible( true );
        m_LineList.append( line );
        return true;
    }
    bool status = setPoint( 0, start );
    if( status)
        return setPoint(count,end);
    return false;
}

/**
 * Returns the amount of POINTS on the line.
 * Includes start and end points.
 */
int AssociationLine::count() const 
{
    return m_LineList.count() + 1;
}

/**
 * Returns -1 if the given point is not on the line.
 * else returns the line segment the point is on.
 * Use the value to insert points at the point position.
 */
int AssociationLine::onLinePath( const QPoint &position )
{
    UMLSceneItemList list = getScene()->collisions( position );
    int index = -1;

    UMLSceneItemList::iterator end(list.end());
    for(UMLSceneItemList::iterator item_it(list.begin()); item_it != end; ++item_it ) {
        if( ( index = m_LineList.indexOf( (UMLSceneLine*)*item_it ) ) != -1 )
            break;
    }//end for
    return index;
}

/**
 * Sets the status of whether the line is selected or not.
 */
void AssociationLine::setSelected( bool select ) 
{
    if(select) {
        setupSelected();
    }
    else if(!m_RectList.isEmpty()) {
        qDeleteAll(m_RectList);
        m_RectList.clear();
    }
}

/**
 * Sets the Association type.
 */
void AssociationLine::setAssocType( Uml::AssociationType type )
{
    QList<UMLSceneLine*>::Iterator it = m_LineList.begin();
    QList<UMLSceneLine*>::Iterator end = m_LineList.end();

    for( ; it != end; ++it )
        (*it) -> setPen( getPen() );

    delete m_pClearPoly;
    m_pClearPoly = 0;

    if( type == Uml::AssociationType::Coll_Message ) {
        setupParallelLine();
    }
    else {
        createHeadLines();
        createSubsetSymbol();
    }
    update();
}

/**
 * Calls a group of methods to update the line. Used to save you calling multiple methods.
 */
void AssociationLine::update()
{
    if (getAssocType() == Uml::AssociationType::Coll_Message) {
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

    if ( m_bSubsetSymbolCreated ) {
        updateSubsetSymbol();
    } else {
        createSubsetSymbol();
    }
}

/**
 * Sets the line color used by the line.
 *
 * @param viewID The id of the object behind the widget.
 */
void AssociationLine::slotLineColorChanged( Uml::IDType viewID )
{
    if(m_pAssociation->umlScene()->getID() != viewID) {
        return;
    }
    setLineColor( m_pAssociation->umlScene()->lineColor() );
}

/**
 * Sets the line color used by the line.
 */
void AssociationLine::setLineColor( const QColor &color )
{
    uint linewidth = 0;
    UMLSceneLine * line = 0;

    Q_FOREACH( line, m_LineList ) {
        linewidth = line->pen().width();
        line->setPen( QPen( color, linewidth ) );
    }

    Q_FOREACH( line, m_HeadList ) {
        linewidth = line->pen().width();
        line->setPen( QPen( color, linewidth ) );
    }

    Q_FOREACH( line, m_ParallelList ) {
        linewidth = line->pen().width();
        line->setPen( QPen( color, linewidth ) );
    }

    if (getAssocType() == Uml::AssociationType::Aggregation) {
        if (m_pClearPoly) {
            m_pClearPoly->setBrush( QBrush( Qt::white ) );
        }
        else if( getAssocType() == Uml::AssociationType::Composition ) {
            if (m_pClearPoly) {
                m_pClearPoly->setBrush( QBrush( color ) );
            }
        }
    }

    if( m_pCircle ) {
        linewidth = m_pCircle->pen().width();
        m_pCircle->setPen( QPen(color, linewidth) );
    }
}

/**
 * Sets the line width used by the line.
 *
 * @param viewID The id of the object behind the widget.
 */
void AssociationLine::slotLineWidthChanged( Uml::IDType viewID )
{
    if(m_pAssociation->umlScene()->getID() != viewID) {
        return;
    }
    setLineWidth( m_pAssociation->umlScene()->lineWidth() );
}

/**
 * Sets the line width used by the line.
 * @param width   the new width of the line
 */
void AssociationLine::setLineWidth( uint width )
{
    QColor linecolor;
    UMLSceneLine * line = 0;

    Q_FOREACH( line, m_LineList ) {
        linecolor = line->pen().color();
        line->setPen( QPen( linecolor, width ) );
    }

    Q_FOREACH( line, m_HeadList ) {
        linecolor = line->pen().color();
        line->setPen( QPen( linecolor, width ) );
    }

    Q_FOREACH( line, m_ParallelList ) {
        linecolor = line->pen().color();
        line->setPen( QPen( linecolor, width ) );
    }

    if( m_pCircle ) {
        linecolor = m_pCircle->pen().color();
        m_pCircle->setPen( QPen(linecolor, width) );
    }
}

/**
 * Moves the selected canvas widgets.
 */
void AssociationLine::moveSelected( int pointIndex )
{
    int lineCount = m_LineList.count();
    if( !m_bSelected ) {
        qDeleteAll( m_RectList.begin(), m_RectList.end() );
        m_RectList.clear();
        return;
    }
    if( (int)m_RectList.count() + 1 != lineCount )
        setupSelected();
    UMLSceneRectangle * rect = 0;
    UMLSceneLine * line = 0;
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

/**
 * Sets up the selected canvases needed.
 */
void AssociationLine::setupSelected()
{
    qDeleteAll( m_RectList.begin(), m_RectList.end() );
    m_RectList.clear();
    UMLSceneLine * line = 0;

    Q_FOREACH( line, m_LineList ) {
        QPoint sp = line -> startPoint();
        UMLSceneRectangle *rect = Widget_Utils::decoratePoint(sp);
        m_RectList.append( rect );
    }
    //special case for last point
    line = m_LineList.last();
    QPoint p = line -> endPoint();
    UMLSceneRectangle *rect = Widget_Utils::decoratePoint(p);
    m_RectList.append( rect );
    update();
}

/**
 * Returns the type of pen to use depending on the type of Association.
 */
QPen AssociationLine::getPen()
{
    Uml::AssociationType type = getAssocType();
    if( type == Uml::AssociationType::Dependency || type == Uml::AssociationType::Realization || type == Uml::AssociationType::Anchor )
        return QPen( lineColor(), lineWidth(), Qt::DashLine );
    return QPen( lineColor(), lineWidth() );
}

/**
 * Calculates the head points.
 */
void AssociationLine::calculateHead()
{
    uint size = m_LineList.count();
    QPoint farPoint;
    int halfLength = 10;
    double arrowAngle = 0.2618;   // 0.5 * atan(sqrt(3.0) / 3.0) = 0.2618
    Uml::AssociationType at = getAssocType();
    bool diamond = (at == Uml::AssociationType::Aggregation || at == Uml::AssociationType::Composition);
    if (diamond || at == Uml::AssociationType::Containment) {
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
    if( getAssocType() == Uml::AssociationType::Realization ||
            getAssocType() == Uml::AssociationType::Generalization ) {
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

/**
 * Updates the head lines. Call after calculating the new points.
 */
void AssociationLine::updateHead()
{
    int count = m_HeadList.count();
    UMLSceneLine * line = 0;

    switch( getAssocType() ) {
    case Uml::AssociationType::State:
    case Uml::AssociationType::Activity:
    case Uml::AssociationType::Exception:
    case Uml::AssociationType::UniAssociation:
    case Uml::AssociationType::Dependency:
        if( count < 2)
            return;

        line = m_HeadList.at( 0 );
        line -> setPoints( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointA.x(), m_ArrowPointA.y() );

        line = m_HeadList.at( 1 );
        line -> setPoints( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );
        break;

    case Uml::AssociationType::Relationship:
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

    case Uml::AssociationType::Generalization:
    case Uml::AssociationType::Realization:
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

    case Uml::AssociationType::Composition:
    case Uml::AssociationType::Aggregation:
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

    case Uml::AssociationType::Containment:
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

/**
 * Create a number of new lines and append them to the given list.
 *
 * @param list  The list into which to append lines.
 * @param by    The number of lines to insert into the given list.
 */
void AssociationLine::growList(LineList &list, int by)
{
    QPen pen( lineColor(), lineWidth() );
    for (int i = 0; i < by; i++) {
        UMLSceneLine * line = new UMLSceneLine( getScene() );
        line -> setZ( 0 );
        line -> setPen( pen );
        line -> setVisible( true );
        list.append( line );
    }
}

/**
 * Creates the head lines to display the head.
 */
void AssociationLine::createHeadLines()
{
    qDeleteAll( m_HeadList.begin(), m_HeadList.end() );
    m_HeadList.clear();
    UMLViewCanvas * canvas = getScene();
    switch( getAssocType() ) {
    case Uml::AssociationType::Activity:
    case Uml::AssociationType::Exception:
    case Uml::AssociationType::State:
    case Uml::AssociationType::Dependency:
    case Uml::AssociationType::UniAssociation:
    case Uml::AssociationType::Relationship:
        growList(m_HeadList, 2);
        break;

    case Uml::AssociationType::Generalization:
    case Uml::AssociationType::Realization:
        growList(m_HeadList, 3);
        m_pClearPoly = new UMLScenePolygon( canvas );
        m_pClearPoly -> setVisible( true );
        m_pClearPoly -> setBrush( QBrush( Qt::white ) );
        m_pClearPoly -> setZ( -1 );
        break;

    case Uml::AssociationType::Composition:
    case Uml::AssociationType::Aggregation:
        growList(m_HeadList, 4);
        m_pClearPoly = new UMLScenePolygon( canvas );
        m_pClearPoly -> setVisible( true );
        if( getAssocType() == Uml::AssociationType::Aggregation )
            m_pClearPoly->setBrush( QBrush( Qt::white ) );
        else
            m_pClearPoly -> setBrush( QBrush( lineColor() ) );
        m_pClearPoly -> setZ( -1 );
        break;

    case Uml::AssociationType::Containment:
        growList(m_HeadList, 1);
        if (!m_pCircle) {
            m_pCircle = new Circle( canvas, 6 );
            m_pCircle->show();
            m_pCircle->setPen( QPen( lineColor(), lineWidth() ) );
        }
        break;
    default:
        break;
    }
    m_bHeadCreated = true;
}

/**
 * Calculates the position of the parallel line.
 */
void AssociationLine::calculateParallelLine()
{
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

/**
 * Creates the line objects to display the parallel line.
 */
void AssociationLine::setupParallelLine()
{
    qDeleteAll( m_ParallelList.begin(), m_ParallelList.end() );
    m_ParallelList.clear();
    growList(m_ParallelList, 3);
    m_bParallelLineCreated = true;
}

/**
 * Updates the parallel line.
 * Call after calculating the new position.
 */
void AssociationLine::updateParallelLine() 
{
    if( !m_bParallelLineCreated )
        return;
    UMLSceneLine * line = 0;
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

/**
 * Equal to (==) operator.
 */
bool AssociationLine::operator==( const AssociationLine & rhs ) 
{
    if( this->m_LineList.count() != rhs.m_LineList.count() )
        return false;

    //Check to see if all points at the same position
    for( int i = 0; i< rhs.count() ; i++ ) {
        if( this->getPoint( i ) != rhs.getPoint( i ) )
            return false;
    }
    return true;
}

/**
 * Copy ( = ) operator.
 */
AssociationLine & AssociationLine::operator=( const AssociationLine & rhs )
{
    if( this == &rhs )
        return *this;
    //clear out the old canvas objects
    this->cleanup();

    int count = rhs.m_LineList.count();
    //setup start end points
    this->setStartEndPoints( rhs.getPoint( 0 ), rhs.getPoint( count) );
    //now insert the rest
    for( int i = 1; i < count ; i++ ) {
        this->insertPoint( i, rhs.getPoint ( i ) );
    }
    this->setAssocType( rhs.getAssocType() );

    return *this;
}

/**
 * Returns the canvas being used.
 * Will return zero if the Association hasn't been set.
 *
 * This class doesn't hold this information but is a wrapper
 * method to stop calls to undefined variable like m_pAssociation.
 */
UMLViewCanvas * AssociationLine::getScene()
{
    if( !m_pAssociation )
        return 0;
    const UMLView * view =  m_pAssociation->umlScene();
    return static_cast<UMLViewCanvas *>(view->canvas());
}

/**
 * Returns the Association type.
 * Returns Uml::AssociationType::Association if association hasn't been set.
 *
 * This class doesn't hold this information but is a wrapper
 * method to stop calls to undefined variable like m_pAssociation.
 */
Uml::AssociationType AssociationLine::getAssocType() const 
{
    if( m_pAssociation )
        return m_pAssociation->associationType();
    return Uml::AssociationType::Association;
}

/**
 * Returns the Line Color to use.
 * Returns black if association not set.
 *
 * This class doesn't hold this information but is a wrapper
 * method to stop calls to undefined variable like m_pAssociation.
 */
QColor AssociationLine::lineColor()
{
    if( !m_pAssociation )
        return Qt::black;
    return m_pAssociation->lineColor();
}

/**
 * Returns the Line Width to use.
 * Returns 0 if association not set.
 *
 * This class doesn't hold this information but is a wrapper
 * method to stop calls to undefined variable like m_pAssociation.
 */
uint AssociationLine::lineWidth()
{
    if( !m_pAssociation )
        return 0;
    int viewLineWidth = m_pAssociation->lineWidth();
    if ( viewLineWidth >= 0 && viewLineWidth <= 10 )
        return viewLineWidth;
    else {
        uWarning() << "Ignore wrong LineWidth of " << viewLineWidth
                   << " in AssociationLine::lineWidth";
        return 0;
    }
}

/**
 * Removes and item created that are no longer needed.
 */
void AssociationLine::cleanup()
{
    if (m_pAssociation) {
        qDeleteAll( m_LineList.begin(), m_LineList.end() );
        m_LineList.clear();
    }

    qDeleteAll( m_HeadList.begin(), m_HeadList.end() );
    m_HeadList.clear();

    qDeleteAll( m_RectList.begin(), m_RectList.end() );
    m_RectList.clear();

    qDeleteAll( m_ParallelList.begin(), m_ParallelList.end() );
    m_ParallelList.clear();

    if( m_pClearPoly )
        delete m_pClearPoly;
    if( m_pCircle )
        delete m_pCircle;
    if( m_pSubsetSymbol )
        delete m_pSubsetSymbol;
    m_pCircle = 0;
    m_pClearPoly = 0;
    m_pSubsetSymbol = 0;
    m_bHeadCreated = m_bParallelLineCreated = m_bSubsetSymbolCreated = false;
    if (m_pAssociation) {
        UMLView * view =  (UMLView *)m_pAssociation->parent();
        if (view) {
            disconnect(view, SIGNAL(sigColorChanged(Uml::IDType)), this, SLOT(slotLineColorChanged(Uml::IDType)));
            disconnect(view, SIGNAL(sigLineWidthChanged(Uml::IDType)), this, SLOT(slotLineWidthChanged(Uml::IDType)));
        }
        m_pAssociation = 0;
    }
}

/**
 * Tell the line where the line docks.
 */
void AssociationLine::setDockRegion( Region region )
{
    m_DockRegion = region;
}

bool AssociationLine::hasPoints () const
{
    int count = m_LineList.count();
    if (count>1)
        return true;
    return false;
}

void AssociationLine::dumpPoints ()
{
    int count = m_LineList.count();
    for( int i = 1; i < count; i++ ) {
        QPoint point = getPoint( i );
        uDebug()<<" * point x:"<<point.x()<<" y:"<<point.y();
    }
}

void AssociationLine::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
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

bool AssociationLine::loadFromXMI( QDomElement & qElement )
{
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

/**
 * Activates the line list.
 * This is needed because the m_pAssociation does not yet
 * exist at the time of the AssociationLine::loadFromXMI call.
 * However, this means that the points in the m_LineList
 * do not have a parent when they are loaded.
 * They need to be reparented by calling AssociationLine::activate()
 * once the m_pAssociation exists.
 */
void AssociationLine::activate()
{
    int count = m_LineList.count();
    if (count == 0)
        return;
    UMLViewCanvas * canvas = getScene();
    if (canvas == NULL)
        return;
    for (int i = 0; i < count ; i++) {
        UMLSceneLine *line = m_LineList.at(i);
        line -> setCanvas( canvas );
        line -> setPen( getPen() );
    }
}

/**
 * Creates the subset symbol.
 */
void AssociationLine::createSubsetSymbol()
{
    if ( m_LineList.count() < 1 ) {
        return;
    }

    switch( getAssocType() ) {
       case Uml::AssociationType::Child2Category:
           m_pSubsetSymbol = new SubsetSymbol(getScene());
           m_pSubsetSymbol->setPen( QPen( lineColor(), lineWidth() ) );
           updateSubsetSymbol();
           m_pSubsetSymbol->show();
           break;
       default:
           break;
    }
    m_bSubsetSymbolCreated = true;
}


/**
 * Updates the subset symbol.Call after calculating the new points.
 */
void AssociationLine::updateSubsetSymbol()
{
    if ( m_LineList.count() < 1 ) {
        return;
    }
    UMLSceneLine* firstLine = m_LineList.first();
    QPoint startPoint = firstLine->startPoint();
    QPoint endPoint = firstLine->endPoint();
    QPoint centrePoint;
    centrePoint.setX( ( startPoint.x() + endPoint.x() )/2 );
    centrePoint.setY( ( startPoint.y() + endPoint.y() )/2 );

    if ( m_pSubsetSymbol ) {

        double xDiff = endPoint.x() - startPoint.x();
        double yDiff = endPoint.y() - startPoint.y();

        int inclination;
        if ( xDiff == 0 ) {
            if ( yDiff > 0 )
                inclination = 90;
            else // yDiff < 0
                inclination = 270;
        } else {
            inclination = (int)(atan( yDiff/xDiff )*180/3.14159) ;
            // convert to 360 degree scale
            if (  xDiff < 0 ) {
                inclination = 180 + inclination ;
            } else if ( xDiff > 0 && yDiff < 0 ) {
                inclination = 360 +  inclination;
            }
        }

        m_pSubsetSymbol->setInclination(inclination);
        m_pSubsetSymbol->setX(centrePoint.x());
        m_pSubsetSymbol->setY(centrePoint.y());
    }
}

AssociationLine::SubsetSymbol::SubsetSymbol(UMLViewCanvas* canvas)
    : UMLSceneEllipse(canvas) {
    inclination = 0;
}

void AssociationLine::SubsetSymbol::drawShape(QPainter& p) {
    p.translate(QPoint( ( int )x(), ( int )y() ) );
    p.rotate( inclination );
    int width = 30, height = 20;
    int startAngle = 90, endAngle = 180;
    p.drawArc( 0 ,-height/2, width, height, startAngle*16, endAngle*16 );
    // revert back
    p.rotate( -inclination );
    p.translate( QPoint( ( int )-x(), ( int )-y() ) );
}

#include "associationline.moc"
