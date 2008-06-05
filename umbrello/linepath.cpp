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
#include <QGraphicsScene>
#include <QDomDocument>
#include <QPainter>

// kde includes
#include <kdebug.h>

// application includes
#include "associationwidget.h"
#include "activitywidget.h"
#include "widget_utils.h"
#include "umlview.h"
#include "umldoc.h"
#include "uml.h"
#include "umlscene.h"


LinePath::Circle::Circle(QGraphicsScene *scene, qreal radius /* = 0 */)
    : QGraphicsEllipseItem(0, 0, radius * 2, radius * 2)
{
    if(scene) {
        scene->addItem(this);
    }
}

void LinePath::Circle::setX(qreal x)
{
    setPos(x, QGraphicsEllipseItem::y());
}

void LinePath::Circle::setY(qreal y)
{
    setPos(QGraphicsEllipseItem::x(), y);
}

void LinePath::Circle::setRadius(qreal radius)
{
    setRect(0, 0, radius*2, radius*2);
}

qreal LinePath::Circle::getRadius() const
{
    return (QGraphicsEllipseItem::rect().height() / 2.0);
}

void LinePath::Circle::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    // The base method does draw with pen unlike Q3CanvasEllipse.
    QGraphicsEllipseItem::paint(p, o, w);
}

LinePath::LinePath() {
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

LinePath::~LinePath()
{
}

void LinePath::setAssociation(AssociationWidget * association )
{
    if( !association )
        return;
    cleanup();
    m_pAssociation = association;
    createHeadLines();
    createSubsetSymbol();
    if( getAssocType() == Uml::at_Coll_Message )
        setupParallelLine();
    UMLScene *scene =  const_cast<UMLScene *>(m_pAssociation -> getUMLScene());
    connect( scene, SIGNAL( sigColorChanged( Uml::IDType ) ), this, SLOT( slotLineColorChanged( Uml::IDType ) ) );
    connect( scene, SIGNAL( sigLineWidthChanged( Uml::IDType ) ), this, SLOT( slotLineWidthChanged( Uml::IDType ) ) );
}

QPointF LinePath::getPoint( int pointIndex ) const {
    int count = m_LineList.count();
    if( count == 0 || pointIndex > count  || pointIndex < 0)
        return QPointF( -1, -1 );

    if( pointIndex == count ) {
        QGraphicsLineItem * lineItem = m_LineList.last();
        return lineItem->line().p2();
    }
    QGraphicsLineItem * lineItem = m_LineList.at( pointIndex );
    return lineItem->line().p1();
}

bool LinePath::setPoint( int pointIndex, const QPointF &point ) {
    int count = m_LineList.count();
    if( count == 0 || pointIndex > count  || pointIndex < 0)
        return false;
    if (point.x() == 0 && point.y() == 0) {
        uError() << "ignoring request for (0,0)" << endl;
        return false;
    }

    if( pointIndex == count) {
        QGraphicsLineItem * line = m_LineList.last();
        QPointF p = line->line().p1();
        line->setLine( p.x(), p.y(), point.x(), point.y() );
        moveSelected( pointIndex );
        update();
        return true;
    }
    if( pointIndex == 0 ) {
        QGraphicsLineItem * line = m_LineList.first();
        QPointF p = line->line().p2();
        line->setLine( point.x(), point.y(), p.x(), p.y() );
        moveSelected( pointIndex );
        update();
        return true;
    }
    QGraphicsLineItem * line = m_LineList.at( pointIndex  );
    QPointF p = line->line().p2();
    line->setLine( point.x(), point.y(), p.x(), p.y() );
    line = m_LineList.at( pointIndex - 1 );
    p = line->line().p1();
    line->setLine( p.x(), p.y(), point.x(), point.y() );
    moveSelected( pointIndex );
    update();
    return true;
}

bool LinePath::isPoint( int pointIndex, const QPointF &point, qreal delta) {
    int count = m_LineList.count();
    if ( pointIndex >= count )
        return false;

    QGraphicsLineItem * line = m_LineList.at( pointIndex );

    /* check if the given point is the start or end point of the line */
    if ( (
             qAbs( line->line().p2().x() - point.x() ) <= delta
             &&
             qAbs( line->line().p2().y() - point.y() ) <= delta
             ) || (
                 qAbs( line->line().p1().x() - point.x() ) <= delta
                 &&
                 qAbs( line->line().p1().y() - point.y() ) <= delta
                 ) )
        return true;

    /* check if the given point is the start or end point of the line */
    return false;
}

bool LinePath::insertPoint( int pointIndex, const QPointF &point ) {
    int count = m_LineList.count();
    if( count == 0 )
        return false;
    const bool bLoading = UMLApp::app()->getDocument()->loading();

    if( count == 1 || pointIndex == 1) {
        QGraphicsLineItem * first = m_LineList.first();
        QPointF sp = first->line().p1();
        QPointF ep = first->line().p2();
        first->setLine( sp.x(), sp.y(), point.x(), point.y() );
        QGraphicsLineItem * line = new QGraphicsLineItem();
        getScene()->addItem(line);
        line->setZValue( -2 );
        line->setLine( point.x(), point.y(), ep.x(), ep.y() );
        line->setPen( getPen() );
        line->setVisible( true );
        m_LineList.insert( 1, line );
        if (!bLoading)
            setupSelected();
        return true;
    }
    if( count + 1 == pointIndex ) {
        QGraphicsLineItem * before = m_LineList.last();
        QPointF sp = before->line().p1();
        QPointF ep = before->line().p2();
        before->setLine( sp.x(), sp.y(), point.x(), point.y() );
        QGraphicsLineItem * line = new QGraphicsLineItem();
        getScene()->addItem(line);
        line->setLine( point.x(), point.y(), ep.x(), ep.y() );
        line->setZValue( -2 );
        line->setPen( getPen() );
        line->setVisible( true );
        m_LineList.append( line );
        if (!bLoading)
            setupSelected();
        return true;
    }
    QGraphicsLineItem * before = m_LineList.at( pointIndex - 1 );
    QPointF sp = before->line().p1();
    QPointF ep = before->line().p2();
    before->setLine( sp.x(), sp.y(), point.x(), point.y() );
    QGraphicsLineItem * line = new QGraphicsLineItem();
    getScene()->addItem(line);
    line->setLine( point.x(), point.y(), ep.x(), ep.y() );
    line->setZValue( -2 );
    line->setPen( getPen() );
    line->setVisible( true );
    m_LineList.insert( pointIndex, line );
    if (!bLoading)
        setupSelected();
    return true;
}

bool LinePath::removePoint( int pointIndex, const QPointF &point, qreal delta )
{
    /* get the number of line segments */
    int count = m_LineList.count();
    if ( pointIndex >= count )
        return false;

    /* we don't know if the user clicked on the start- or endpoint of a
     * line segment */
    QGraphicsLineItem * current_line = m_LineList.at( pointIndex );
    if (qAbs( current_line->line().p2().x() - point.x() ) <= delta
        &&
        qAbs( current_line->line().p2().y() - point.y() ) <= delta)
    {
        /* the user clicked on the end point of the line;
         * we have to make sure that this isn't the last line segment */
        if (pointIndex >= count - 1)
            return false;

        /* the next segment will get the starting point from the current one,
         * which is going to be removed */
        QGraphicsLineItem * next_line = m_LineList.at( pointIndex + 1 );
        QPointF startPoint = current_line->line().p1();
        QPointF endPoint = next_line->line().p2();
        next_line->setLine(startPoint.x(), startPoint.y(),
                           endPoint.x(), endPoint.y());

    } else
        if (qAbs( current_line->line().p1().x() - point.x() ) <= delta
            &&
            qAbs( current_line->line().p1().y() - point.y() ) <= delta)
        {
            /* the user clicked on the start point of the line;
             * we have to make sure that this isn't the first line segment */
            if (pointIndex < 1)
                return false;

            /* the previous segment will get the end point from the current one,
             * which is going to be removed */
            QGraphicsLineItem * previous_line = m_LineList.at( pointIndex - 1 );
            QPointF startPoint = previous_line->line().p1();
            QPointF endPoint = current_line->line().p2();
            previous_line->setLine(startPoint.x(), startPoint.y(),
                                   endPoint.x(), endPoint.y());
        } else {
            /* the user clicked neither on the start- nor on the end point of
             * the line; this really shouldn't happen, but just make sure */
            return false;
        }


    /* remove the segment from the list */
    delete m_LineList.takeAt( pointIndex );

    return true;
}

bool LinePath::setStartEndPoints( const QPointF &start, const QPointF &end ) {
    int count = m_LineList.count();
    if( count == 0 ) {
        QGraphicsLineItem * line = new QGraphicsLineItem();
        getScene()->addItem(line);
        line->setLine( start.x(), start.y(),end.x(),end.y() );
        line->setZValue( -2 );
        line->setPen( getPen() );
        line->setVisible( true );
        m_LineList.append( line );
        return true;
    }
    bool status = setPoint( 0, start );
    if( status)
        return setPoint(count,end);
    return false;
}

int LinePath::count() const {
    return m_LineList.count() + 1;
}

int LinePath::onLinePath( const QPointF &position ) {
    QList<QGraphicsItem*> list = getScene()->items(position);

    foreach(QGraphicsItem *item, list) {
        QGraphicsLineItem *lineItem = dynamic_cast<QGraphicsLineItem*>(item);
        if(lineItem) {
            int index = m_LineList.indexOf(lineItem);
            if(index != -1)
                return index;
        }
    }
    return -1;
}

void LinePath::setSelected( bool select ) {
    if(select) {
        setupSelected();
    }
    else if(!m_RectList.isEmpty()) {
        qDeleteAll(m_RectList);
        m_RectList.clear();
    }
}

void LinePath::setAssocType( Uml::Association_Type type )
{
    QList<QGraphicsLineItem*>::Iterator it = m_LineList.begin();
    QList<QGraphicsLineItem*>::Iterator end = m_LineList.end();

    for( ; it != end; ++it )
        (*it)->setPen( getPen() );

    delete m_pClearPoly;
    m_pClearPoly = 0;

    if( type == Uml::at_Coll_Message ) {
        setupParallelLine();
    }
    else {
        createHeadLines();
        createSubsetSymbol();
    }
    update();
}

void LinePath::update()
{
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

    if ( m_bSubsetSymbolCreated ) {
        updateSubsetSymbol();
    } else {
        createSubsetSymbol();
    }
}

void LinePath::slotLineColorChanged( Uml::IDType viewID )
{
    if(m_pAssociation->getUMLScene()->getID() != viewID) {
        return;
    }
    setLineColor( m_pAssociation->getUMLScene()->getLineColor() );
}


void LinePath::setLineColor( const QColor &color )
{
    uint linewidth = 0;
    QGraphicsLineItem * line = 0;

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

    if (getAssocType() == Uml::at_Aggregation) {
        if (m_pClearPoly) {
            m_pClearPoly->setBrush( QBrush( Qt::white ) );
        }
        else if( getAssocType() == Uml::at_Composition ) {
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

void LinePath::slotLineWidthChanged( Uml::IDType viewID ) {
    if(m_pAssociation->getUMLScene()->getID() != viewID) {
        return;
    }
    setLineWidth( m_pAssociation->getUMLScene()->getLineWidth() );
}

void LinePath::setLineWidth( uint width )
{
    QColor linecolor;
    QGraphicsLineItem * line = 0;

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

void LinePath::moveSelected( int pointIndex )
{
    int lineCount = m_LineList.count();
    if( !m_bSelected ) {
        qDeleteAll( m_RectList.begin(), m_RectList.end() );
        m_RectList.clear();
        return;
    }
    if( (int)m_RectList.count() + 1 != lineCount )
        setupSelected();
    QGraphicsRectItem * rect = 0;
    QGraphicsLineItem * line = 0;
    if( pointIndex == lineCount || lineCount == 1) {
        line = m_LineList.last();
        QPointF p = line->line().p2();
        rect = m_RectList.last();
        rect->setPos(p.x(), p.y());
        rect->setZValue( 4 );
        return;
    }
    line = m_LineList.at( pointIndex );
    QPointF p = line->line().p1();
    rect = m_RectList.at( pointIndex );
    rect->setPos(p.x(), p.y());
    rect->setZValue( 4 );
}

void LinePath::setupSelected()
{
    qDeleteAll( m_RectList.begin(), m_RectList.end() );
    m_RectList.clear();
    QGraphicsLineItem *line = 0;

    Q_FOREACH(line, m_LineList) {
        QPointF sp = line->line().p1();
        QGraphicsRectItem *rect = Widget_Utils::decoratePoint(sp);
        m_RectList.append(rect);
    }
    //special case for last point
    line = m_LineList.last();
    QPointF p = line->line().p2();
    QGraphicsRectItem *rect = Widget_Utils::decoratePoint(p);
    m_RectList.append( rect );
    update();
}

QPen LinePath::getPen()
{
    Uml::Association_Type type = getAssocType();
    if( type == Uml::at_Dependency || type == Uml::at_Realization || type == Uml::at_Anchor )
        return QPen( getLineColor(), getLineWidth(), Qt::DashLine );
    return QPen( getLineColor(), getLineWidth() );
}

void LinePath::calculateHead()
{
    uint size = m_LineList.count();
    QPointF farPoint;
    qreal halfLength = 10;
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
    qreal xa = farPoint.x();
    qreal ya = farPoint.y();
    qreal xb = m_EgdePoint.x();
    qreal yb = m_EgdePoint.y();
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

    m_ArrowPointA.setX( (qreal)rint(xb - halfLength * cos(arrowSlope)) );
    m_ArrowPointA.setY( (qreal)rint(yb - halfLength * sin(arrowSlope)) );
    arrowSlope = slope - arrowAngle;
    m_ArrowPointB.setX( (qreal)rint(xb - halfLength * cos(arrowSlope)) );
    m_ArrowPointB.setY( (qreal)rint(yb - halfLength * sin(arrowSlope)) );

    if(xa > xb)
        cosx = cosx > 0 ? cosx : cosx * -1;
    else
        cosx = cosx > 0 ? cosx * -1: cosx;

    if(ya > yb)
        siny = siny > 0 ? siny : siny * -1;
    else
        siny = siny > 0 ? siny * -1 : siny;

    m_MidPoint.setX( (qreal)rint(xb + cosx) );
    m_MidPoint.setY( (qreal)rint(yb + siny) );

    m_PointArray.setPoint(0, m_EgdePoint.toPoint());
    m_PointArray.setPoint(1, m_ArrowPointA.toPoint());
    if( getAssocType() == Uml::at_Realization ||
        getAssocType() == Uml::at_Generalization ) {
        m_PointArray.setPoint( 2, m_ArrowPointB.toPoint() );
        m_PointArray.setPoint( 3, m_EgdePoint.toPoint() );
    } else {
        QPointF diamondFarPoint;
        diamondFarPoint.setX( (qreal)rint(xb + cosx * 2) );
        diamondFarPoint.setY( (qreal)rint(yb + siny * 2) );
        m_PointArray.setPoint(2, diamondFarPoint.toPoint());
        m_PointArray.setPoint(3, m_ArrowPointB.toPoint());
    }

}

void LinePath::updateHead()
{
    int count = m_HeadList.count();
    QGraphicsLineItem * line = 0;

    switch( getAssocType() ) {
    case Uml::at_State:
    case Uml::at_Activity:
    case Uml::at_Exception:
    case Uml::at_UniAssociation:
    case Uml::at_Dependency:
        if( count < 2)
            return;

        line = m_HeadList.at( 0 );
        line->setLine( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointA.x(), m_ArrowPointA.y() );

        line = m_HeadList.at( 1 );
        line->setLine( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );
        break;

    case Uml::at_Relationship:
        if (count < 2) {
            return;
        }
        {
            qreal xoffset = 0;
            qreal yoffset = 0;
            if( m_DockRegion == TopBottom )
                xoffset = 8;
            else
                yoffset = 8;
            line = m_HeadList.at( 0 );
            line->setLine( m_PointArray[2].x(), m_PointArray[2].y(),
                           m_PointArray[0].x()-xoffset, m_PointArray[0].y()-yoffset );

            line = m_HeadList.at( 1 );
            line->setLine( m_PointArray[2].x(), m_PointArray[2].y(),
                           m_PointArray[0].x()+xoffset, m_PointArray[0].y()+yoffset );
        }

    case Uml::at_Generalization:
    case Uml::at_Realization:
        if( count < 3)
            return;
        line = m_HeadList.at( 0 );
        line->setLine( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointA.x(), m_ArrowPointA.y() );

        line = m_HeadList.at( 1 );
        line->setLine( m_EgdePoint.x(), m_EgdePoint.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );

        line = m_HeadList.at( 2 );
        line->setLine( m_ArrowPointA.x(), m_ArrowPointA.y(), m_ArrowPointB.x(), m_ArrowPointB.y() );
        m_pClearPoly->setPolygon( QPolygonF(m_PointArray) );
        break;

    case Uml::at_Composition:
    case Uml::at_Aggregation:
        if( count < 4)
            return;
        line = m_HeadList.at( 0 );
        line->setLine( m_PointArray[ 0 ].x(), m_PointArray[ 0 ].y(), m_PointArray[ 1 ].x(), m_PointArray[ 1 ].y() );

        line = m_HeadList.at( 1 );
        line->setLine( m_PointArray[ 1 ].x(), m_PointArray[ 1 ].y(), m_PointArray[ 2 ].x(), m_PointArray[ 2 ].y() );

        line = m_HeadList.at( 2 );
        line->setLine( m_PointArray[ 2 ].x(), m_PointArray[ 2 ].y(), m_PointArray[ 3 ].x(), m_PointArray[ 3 ].y() );

        line = m_HeadList.at( 3 );
        line->setLine( m_PointArray[ 3 ].x(), m_PointArray[ 3 ].y(), m_PointArray[ 0 ].x(), m_PointArray[ 0 ].y() );
        m_pClearPoly->setPolygon( QPolygonF(m_PointArray) );
        break;

    case Uml::at_Containment:
        if (count < 1)
            return;
        line = m_HeadList.at( 0 );
        line->setLine( m_PointArray[ 1 ].x(), m_PointArray[ 1 ].y(),
                       m_PointArray[ 3 ].x(), m_PointArray[ 3 ].y() );
        m_pCircle->setX( m_MidPoint.x() );
        m_pCircle->setY( m_MidPoint.y() );
        break;
    default:
        break;
    }
}

void LinePath::growList(LineList &list, int by)
{
    QPen pen( getLineColor(), getLineWidth() );
    for (int i = 0; i < by; i++) {
        QGraphicsLineItem * line = new QGraphicsLineItem();
        getScene()->addItem(line);
        line->setZValue( 0 );
        line->setPen( pen );
        line->setVisible( true );
        list.append( line );
    }
}

void LinePath::createHeadLines()
{
    qDeleteAll( m_HeadList.begin(), m_HeadList.end() );
    m_HeadList.clear();
    QGraphicsScene *scene = getScene();
    switch( getAssocType() ) {
    case Uml::at_Activity:
    case Uml::at_Exception:
    case Uml::at_State:
    case Uml::at_Dependency:
    case Uml::at_UniAssociation:
    case Uml::at_Relationship:
        growList(m_HeadList, 2);
        break;

    case Uml::at_Generalization:
    case Uml::at_Realization:
        growList(m_HeadList, 3);
        m_pClearPoly = new QGraphicsPolygonItem();
        scene->addItem(m_pClearPoly);
        m_pClearPoly->setBrush( QBrush( Qt::white ) );
        m_pClearPoly->setZValue( -1 );
        break;

    case Uml::at_Composition:
    case Uml::at_Aggregation:
        growList(m_HeadList, 4);
        m_pClearPoly = new QGraphicsPolygonItem();
        scene->addItem(m_pClearPoly);
        m_pClearPoly->setVisible( true );
        if( getAssocType() == Uml::at_Aggregation )
            m_pClearPoly->setBrush( QBrush( Qt::white ) );
        else
            m_pClearPoly->setBrush( QBrush( getLineColor() ) );
        m_pClearPoly->setZValue( -1 );
        break;

    case Uml::at_Containment:
        growList(m_HeadList, 1);
        if (!m_pCircle) {
            m_pCircle = new Circle(scene, 6);
            m_pCircle->show();
            m_pCircle->setPen( QPen( getLineColor(), getLineWidth() ) );
        }
        break;
    default:
        break;
    }
    m_bHeadCreated = true;
}

void LinePath::calculateParallelLine()
{
    int midCount = count() / 2;
    double ATAN = atan(1.0);
    qreal lineDist = 10;
    //get  1/8(M) and 7/8(T) point
    QPointF a = getPoint( midCount - 1 );
    QPointF b = getPoint( midCount );
    qreal mx = ( a.x() + b.x() ) / 2;
    qreal my = ( a.y() + b.y() ) / 2;
    qreal tx = ( mx + b.x() ) / 2;
    qreal ty = ( my + b.y() ) / 2;
    //find dist between M and T points
    qreal distX = ( mx - tx );
    distX *= distX;
    qreal distY = ( my - ty );
    distY *= distY;
    double dist = sqrt( double(distX + distY) );
    double angle = atan2( double(ty - my), double(tx - mx) ) + ( ATAN * 2 );
    //find point from M to start line from.
    double cosx = cos( angle ) * lineDist;
    double siny = sin( angle ) * lineDist;
    QPointF pointM( mx + (qreal)cosx, my + (qreal)siny );
    //find dist between P(xb, yb)
    distX = ( tx - b.x() );
    distX *= distX;
    distY = ( ty - b.y() );
    distY *= distY;
    dist = sqrt( double(distX + distY) );
    //find point from T to end line
    cosx = cos( angle ) * lineDist;
    siny = sin( angle ) * lineDist;
    QPointF pointT( tx + (qreal)cosx, ty + (qreal)siny );
    m_ParallelLines.setPoint(1, pointM.toPoint());
    m_ParallelLines.setPoint(0, pointT.toPoint());

    qreal arrowDist = 5;
    angle = atan2( double(pointT.y() - pointM.y()),
                   double(pointT.x() - pointM.x()) );
    double arrowSlope = angle + ATAN;
    cosx = ( cos( arrowSlope ) ) * arrowDist;
    siny = ( sin( arrowSlope ) ) * arrowDist;
    m_ParallelLines.setPoint(2, QPointF( pointT.x() - (qreal)cosx, pointT.y() - (qreal)siny ).toPoint());
    arrowSlope = angle - ATAN;
    cosx = ( cos( arrowSlope )  ) * arrowDist;
    siny = ( sin( arrowSlope ) ) * arrowDist;
    m_ParallelLines.setPoint(3, QPointF( pointT.x() - (qreal)cosx, pointT.y() - (qreal)siny ).toPoint());
}

void LinePath::setupParallelLine()
{
    qDeleteAll( m_ParallelList.begin(), m_ParallelList.end() );
    m_ParallelList.clear();
    growList(m_ParallelList, 3);
    m_bParallelLineCreated = true;
}

void LinePath::updateParallelLine() {
    if( !m_bParallelLineCreated )
        return;
    QGraphicsLineItem * line = 0;
    QPointF common = m_ParallelLines.at( 0 );
    QPointF p = m_ParallelLines.at( 1 );
    line = m_ParallelList.at( 0 );
    line->setLine( common.x(), common.y(), p.x(), p.y() );

    p = m_ParallelLines.at( 2 );
    line = m_ParallelList.at( 1 );
    line->setLine( common.x(), common.y(), p.x(), p.y() );

    p = m_ParallelLines.at( 3 );
    line = m_ParallelList.at( 2 );
    line->setLine( common.x(), common.y(), p.x(), p.y() );
}

bool LinePath::operator==( const LinePath & rhs ) {
    if( this->m_LineList.count() != rhs.m_LineList.count() )
        return false;

    //Check to see if all points at the same position
    for( int i = 0; i< rhs.count() ; i++ ) {
        if( this->getPoint( i ) != rhs.getPoint( i ) )
            return false;
    }
    return true;
}

LinePath & LinePath::operator=( const LinePath & rhs ) {
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

QGraphicsScene * LinePath::getScene()
{
    if( !m_pAssociation )
        return 0;
    return const_cast<UMLScene*>(m_pAssociation->getUMLScene());
}

Uml::Association_Type LinePath::getAssocType() const {
    if( m_pAssociation )
        return m_pAssociation->getAssocType();
    return Uml::at_Association;
}

QColor LinePath::getLineColor() {
    if( !m_pAssociation )
        return Qt::black;
    return m_pAssociation->getLineColor();
}

uint LinePath::getLineWidth() {
    if( !m_pAssociation )
        return 0;
    int viewLineWidth = m_pAssociation->getLineWidth();
    if ( viewLineWidth >= 0 && viewLineWidth <= 10 )
        return viewLineWidth;
    else {
        uWarning() << "Ignore wrong LineWidth of " << viewLineWidth
                   << " in LinePath::getLineWidth" << endl;
        return 0;
    }
}

void LinePath::cleanup()
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
    if( m_pAssociation ) {
        UMLScene * scene =  const_cast<UMLScene *>(m_pAssociation->getUMLScene());
        if(scene) {
            disconnect( scene, SIGNAL( sigColorChanged( Uml::IDType ) ), this, SLOT( slotLineColorChanged( Uml::IDType ) ) );
            disconnect( scene, SIGNAL( sigLineWidthChanged( Uml::IDType ) ), this, SLOT( slotLineWidthChanged( Uml::IDType ) ) );
        }
        m_pAssociation = NULL;
    }
}

void LinePath::setDockRegion( Region region )
{
    m_DockRegion = region;
}

bool LinePath::hasPoints () const
{
    int count = m_LineList.count();
    if (count>1)
        return true;
    return false;
}

void LinePath::dumpPoints ()
{
    int count = m_LineList.count();
    for( int i = 1; i < count; i++ ) {
        QPointF point = getPoint( i );
        uDebug()<<" * point x:"<<point.x()<<" y:"<<point.y();
    }
}

void LinePath::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    int count = m_LineList.count();
    QPointF point = getPoint( 0 );
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

bool LinePath::loadFromXMI( QDomElement & qElement )
{
    QDomNode node = qElement.firstChild();
    QDomElement startElement = node.toElement();
    if( startElement.isNull() || startElement.tagName() != "startpoint" )
        return false;
    QString x = startElement.attribute( "startx", "0" );
    qreal nX = x.toDouble();
    QString y = startElement.attribute( "starty", "0" );
    qreal nY = y.toDouble();
    QPointF startPoint( nX, nY );

    node = startElement.nextSibling();
    QDomElement endElement = node.toElement();
    if( endElement.isNull() || endElement.tagName() != "endpoint" )
        return false;
    x = endElement.attribute( "endx", "0" );
    nX = x.toDouble();
    y = endElement.attribute( "endy", "0" );
    nY = y.toDouble();
    QPointF endPoint( nX, nY );
    setStartEndPoints( startPoint, endPoint );
    QPointF point;
    node = endElement.nextSibling();
    QDomElement element = node.toElement();
    int i = 1;
    while( !element.isNull() ) {
        if( element.tagName() == "point" ) {
            x = element.attribute( "x", "0" );
            y = element.attribute( "y", "0" );
            point.setX( x.toDouble() );
            point.setY( y.toDouble() );
            insertPoint( i++, point );
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    return true;
}


void LinePath::activate()
{
    int count = m_LineList.count();
    if (count == 0)
        return;
    QGraphicsScene *scene = getScene();
    if (scene == NULL)
        return;
    for (int i = 0; i < count ; i++) {
        QGraphicsLineItem *line = m_LineList.at(i);
        scene->addItem(line);
        line->setPen( getPen() );
    }
}



void LinePath::createSubsetSymbol()
{
    if ( m_LineList.count() < 1 ) {
        return;
    }

    switch( getAssocType() ) {
    case Uml::at_Child2Category:
        m_pSubsetSymbol = new SubsetSymbol(getScene());
        m_pSubsetSymbol->setPen(QPen(getLineColor(), getLineWidth()));
        updateSubsetSymbol();
        m_pSubsetSymbol->show();
        break;
    default:
        break;
    }
    m_bSubsetSymbolCreated = true;
}


void LinePath::updateSubsetSymbol()
{
    if ( m_LineList.count() < 1 ) {
        return;
    }
    QGraphicsLineItem* firstLine = m_LineList.first();
    QPointF startPoint = firstLine->line().p1();
    QPointF endPoint = firstLine->line().p2();
    QPointF centrePoint;
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
        m_pSubsetSymbol->setPos(centrePoint.x(), centrePoint.y());
    }
}

// [PORT]
LinePath::SubsetSymbol::SubsetSymbol(QGraphicsScene *scene)
    : QGraphicsEllipseItem(0, 0, 30, 20)
{
    setStartAngle(90 * 16);
    setSpanAngle(180 * 16);
}

#include "linepath.moc"
