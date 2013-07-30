/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "associationline.h"

// application includes
#include "associationwidget.h"
#include "classifierwidget.h"
#include "debug_utils.h"
#include "widget_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlwidget.h"

// qt includes
#include <QDomDocument>
#include <QPainter>

// system includes
#include <cstdlib>
#include <cmath>

DEBUG_REGISTER_DISABLED(AssociationLine)

// Initialize static variables.
const qreal AssociationLine::Delta = 5;
const qreal AssociationLine::SelectedPointDiameter = 4;
const qreal AssociationLine::SelfAssociationMinimumHeight = 30;

//#define HOVER_EFFECT

/**
 * Constructor.
 * Constructs an AssociationLine item with its parent being \a parent.
 */
AssociationLine::AssociationLine(AssociationWidget *association)
  : QGraphicsObject(association),
    m_associationWidget(association),
    m_activePointIndex(-1),
    m_activeSegmentIndex(-1),
    m_startSymbol(0),
    m_endSymbol(0),
    m_subsetSymbol(0),
    m_collaborationLineItem(0),
    m_collaborationLineHead(0),
    m_associationClassLine(0),
    m_layout(Polyline)
{
    Q_ASSERT(association);
    setFlag(QGraphicsLineItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setZValue(3);
}

/**
 * Destructor.
 */
AssociationLine::~AssociationLine()
{
}

/**
 * Returns the point at the point index.
 * @return point at given index
 */
QPointF AssociationLine::point(int index) const
{
    if ((index < 0) | (index > m_points.size() - 1)) {
        uWarning() << "Index " << index << " out of range [0.." << m_points.size() - 1 << "].";
        return QPointF(-1.0, -1.0);
    }
    return m_points.at(index);
}

/**
 * Sets the point value at given index to \a point.
 */
bool AssociationLine::setPoint(int index, const QPointF &point)
{
    if (index < 0) {
        uWarning() << "Index out of range < 0";
        return false;
    }
    if (m_points.at(index) == point) {
        return false;  // nothing to change
    }
    prepareGeometryChange();
    m_points[index] = point;
    alignSymbols();
    return true;
}

/**
 * Shortcut for point(0).
 */
QPointF AssociationLine::startPoint() const
{
    return m_points.at(0);
}

/**
 * Shortcut for end point.
 */
QPointF AssociationLine::endPoint() const
{
    return m_points.at(m_points.size()-1);
}

/**
 * Inserts the passed in \a point at the \a index passed in and
 * recalculates the bounding rect.
 */
void AssociationLine::insertPoint(int index, const QPointF &point)
{
    prepareGeometryChange();
    m_points.insert(index, point);
    alignSymbols();
}

/**
 * Removes the point at \a index passed in.
 * @see removeNonEndPoint
 */
void AssociationLine::removePoint(int index)
{
    prepareGeometryChange();
    m_points.remove(index);
    alignSymbols();
}

/**
 * Returns the amount of POINTS on the line.
 * Includes start and end points.
 * @return   number of points in the AssociationLine
 */
int AssociationLine::count() const 
{
    return m_points.size();
}

/**
 * Removes all the points and signals a geometry update.
 */
void AssociationLine::cleanup()
{
    if (!m_points.isEmpty()) {
        prepareGeometryChange();
        m_points.clear();
        alignSymbols();
    }
}

/**
 * This method optimizes the number of points in the
 * AssociationLine. This can be used to reduce the clutter caused
 * due to too many points.
 */
void AssociationLine::optimizeLinePoints()
{
    int i = 1;
    prepareGeometryChange();
    while (i < m_points.size()) {
        if (m_points.at(i) == m_points.at(i-1)) {
            m_points.remove(i);
        }
        else {
            ++i;
        }
    }
    alignSymbols();
    // TODO: Use delta comparison 'closestPointIndex' instead of exact comparison.
}

/**
 * Return index of point closer a given delta.
 *
 * @param point The point which is to be tested for closeness.
 * @param delta The distance the point should be closer to.
 *
 * @retval "Index" of the first line point closer to the \a point passed.
 * @retval -1 If no line point is closer to passed in \a point.
 */
int AssociationLine::closestPointIndex(const QPointF &point, qreal delta) const
{
    for(int i = 0; i < m_points.size(); ++i) {
        const QPointF& linePoint = m_points.at(i);
        // Apply distance formula to see point closeness.
        qreal deltaXSquare = (point.x() - linePoint.x()) * (point.x() - linePoint.x());
        qreal deltaYSquare = (point.y() - linePoint.y()) * (point.y() - linePoint.y());

        qreal lhs = deltaXSquare + deltaYSquare;
        qreal rhs = delta * delta;

        if (lhs <= rhs) {
            return i;
        }
    }
    return -1;
}

/**
 * Return index of closest segment.
 *
 * @param point The point which is to be tested for closeness.
 *
 * @retval "Index" of the line segment closest to the \a point passed.
 * @retval -1 If no line segment is closer to passed in \a point.
 */
int AssociationLine::closestSegmentIndex(const QPointF &point, qreal delta) const
{
    QPainterPathStroker stroker;
    stroker.setWidth(delta);

    for(int i = 1; i < m_points.size(); ++i) {
        QLineF segment(m_points[i-1], m_points[i]);

        QPainterPath path;
        path.moveTo(segment.p1());
        path.lineTo(segment.p2());

        path = stroker.createStroke(path);

        if (path.contains(point)) {
            return i-1;
        }
    }
    return -1;
}

/**
 * Retval True If point at \a index is start or end.
 */
bool AssociationLine::isEndPointIndex(int index) const
{
    const int size = m_points.size();
    Q_ASSERT(index >= 0 && index < size);

    return (index == 0 || index == (size - 1));
}

/**
 * Retval True If segment at \a index is start or end.
 */
bool AssociationLine::isEndSegmentIndex(int index) const
{
    // num of seg = num of points - 1
    const int size = m_points.size() - 1;
    Q_ASSERT(index >= 0 && index < size);

    return (index == 0 || index == (size - 1));
}

/**
 * Sets the start and end points.
 */
bool AssociationLine::setEndPoints(const QPointF &start, const QPointF &end)
{
    const int size = m_points.size();

    prepareGeometryChange();

    if (size == 0) {
        m_points.insert(0, start);
        m_points.insert(1, end);
    }
    else if (size == 1) {
        m_points[0] = start;
        m_points.insert(1, end);
    }
    else {
        m_points[0] = start;
        m_points[size-1] = end;
    }

    alignSymbols();
    return true;
}

/**
 * Debug helper method to write out the points.
 */
void AssociationLine::dumpPoints()
{
    for (int i = 1; i < m_points.size(); ++i) {
        QPointF p = m_points.at(i);
        DEBUG(DBG_SRC) << i << ". point x:" << p.x() << " / y:" << p.y();
    }
}

/**
 * Loads AssociationLine information saved in \a qElement XMI element.
 */
bool AssociationLine::loadFromXMI(QDomElement &qElement)
{
    QDomNode node = qElement.firstChild();

    m_points.clear();

    QDomElement startElement = node.toElement();
    if(startElement.isNull() || startElement.tagName() != "startpoint") {
        return false;
    }
    QString x = startElement.attribute( "startx", "0" );
    qreal nX = x.toFloat();
    QString y = startElement.attribute( "starty", "0" );
    qreal nY = y.toFloat();
    QPointF startPoint( nX, nY );

    node = startElement.nextSibling();
    QDomElement endElement = node.toElement();
    if(endElement.isNull() || endElement.tagName() != "endpoint") {
        return false;
    }
    x = endElement.attribute( "endx", "0" );
    nX = x.toFloat();
    y = endElement.attribute( "endy", "0" );
    nY = y.toFloat();
    QPointF endPoint( nX, nY );
    setEndPoints( startPoint, endPoint );
    QPointF point;
    node = endElement.nextSibling();
    QDomElement element = node.toElement();
    int i = 1;
    while(!element.isNull()) {
        if(element.tagName() == "point") {
            x = element.attribute( "x", "0" );
            y = element.attribute( "y", "0" );
            point.setX( x.toFloat() );
            point.setY( y.toFloat() );
            insertPoint( i++, point );
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    return true;
}

/**
 * Saves association line information into XMI element named "linepath".
 * @note Stored as linepath for backwared compatibility
 */
void AssociationLine::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QPointF point = m_associationWidget->mapToScene(startPoint());
    QDomElement lineElement = qDoc.createElement("linepath");
    QDomElement startElement = qDoc.createElement("startpoint");
    startElement.setAttribute("startx", point.x());
    startElement.setAttribute("starty", point.y());
    lineElement.appendChild(startElement);
    QDomElement endElement = qDoc.createElement("endpoint");
    point = m_associationWidget->mapToScene(endPoint());
    endElement.setAttribute("endx", point.x());
    endElement.setAttribute("endy", point.y());
    lineElement.appendChild(endElement);
    for(int i = 1; i < count()-1; ++i) {
        QDomElement pointElement = qDoc.createElement("point");
        point = m_associationWidget->mapToScene(this->point(i));
        pointElement.setAttribute("x", point.x());
        pointElement.setAttribute("y", point.y());
        lineElement.appendChild(pointElement);
    }
    qElement.appendChild(lineElement);
}

/**
 * Returns the type of brush to use depending on the type of Association.
 */
QBrush AssociationLine::brush() const
{
    QBrush brush(Qt::SolidPattern);
    Uml::AssociationType::Enum type = m_associationWidget->associationType();
    if (type == Uml::AssociationType::Aggregation    ||
        type == Uml::AssociationType::Generalization ||
        type == Uml::AssociationType::Realization) {
        brush.setColor(Qt::white);
    }
    if (type == Uml::AssociationType::Composition) {
        brush.setColor(m_associationWidget->lineColor());
    }
    return brush;
}

/**
 * Returns the type of pen to use depending on the type of Association.
 */
QPen AssociationLine::pen() const
{
    QPen pen(m_associationWidget->lineColor(),
             m_associationWidget->lineWidth(),
             Qt::SolidLine,
             Qt::RoundCap,
             Qt::RoundJoin);
    Uml::AssociationType::Enum type = m_associationWidget->associationType();
    if (type == Uml::AssociationType::Dependency  ||
        type == Uml::AssociationType::Realization ||
        type == Uml::AssociationType::Anchor) {
        pen.setStyle(Qt::DashLine);
    }
    return pen;
}

//:TODO: not used
///**
// * Update the pen and brush used to draw symbol.
// */
//void AssociationLine::updateDrawSettings()
//{
//    prepareGeometryChange();
//    QPen changedPen = pen();
//    QBrush changedBrush = brush();
//    if (m_startSymbol) {
//        m_startSymbol->setPen(changedPen);
//        m_startSymbol->setBrush(changedBrush);
//    }
//    if (m_endSymbol) {
//        m_endSymbol->setPen(changedPen);
//        m_endSymbol->setBrush(changedBrush);
//    }
//    if (m_subsetSymbol) {
//        m_subsetSymbol->setPen(changedPen);
//        m_subsetSymbol->setBrush(changedBrush);
//    }
//    if (m_associationClassLine) {
//        changedPen.setStyle(Qt::DashLine);
//        m_associationClassLine->setPen(changedPen);
//    }
//    alignSymbols();
//}

/**
 * Tell the line where the line docks.
 */
void AssociationLine::setDockRegion(Region region)
{
//:TODO:    m_DockRegion = region;
    DEBUG(DBG_SRC) << "region=" << region;
}

/**
 * Calls a group of methods to update the line. Used to save you calling multiple methods.
 */
//:TODO:
//void AssociationLine::update()
//{
//    if (getAssocType() == Uml::AssociationType::Coll_Message) {
//        if (m_bParallelLineCreated) {
//            calculateParallelLine();
//            updateParallelLine();
//        } else
//            setupParallelLine();
//    } else if (m_bHeadCreated) {
//        calculateHead();
//        updateHead();
//    } else {
//        createHeadLines();
//    }

//    if ( m_bSubsetSymbolCreated ) {
//        updateSubsetSymbol();
//    } else {
//        createSubsetSymbol();
//    }
//}

/**
 * Calculates the position of the parallel line.
 */
//:TODO:
//void AssociationLine::calculateParallelLine()
//{
//    int midCount = count() / 2;
//    double ATAN = atan(1.0);
//    int lineDist = 10;
//    //get  1/8(M) and 7/8(T) point
//    QPointF a = point( midCount - 1 );
//    QPointF b = point( midCount );
//    int mx = ( a.x() + b.x() ) / 2;
//    int my = ( a.y() + b.y() ) / 2;
//    int tx = ( mx + b.x() ) / 2;
//    int ty = ( my + b.y() ) / 2;
//    //find dist between M and T points
//    int distX = ( mx - tx );
//    distX *= distX;
//    int distY = ( my - ty );
//    distY *= distY;
//    double angle = atan2( double(ty - my), double(tx - mx) ) + ( ATAN * 2 );
//    //find point from M to start line from.
//    double cosx = cos( angle ) * lineDist;
//    double siny = sin( angle ) * lineDist;
//    QPointF pointM( mx + (int)cosx, my + (int)siny );
//    //find dist between P(xb, yb)
//    distX = ( tx - b.x() );
//    distX *= distX;
//    distY = ( ty - b.y() );
//    distY *= distY;
//    //find point from T to end line
//    cosx = cos( angle ) * lineDist;
//    siny = sin( angle ) * lineDist;
//    QPointF pointT( tx + (int)cosx, ty + (int)siny );
//    m_ParallelLines[ 1 ] = pointM;
//    m_ParallelLines[ 0 ] = pointT;

//    int arrowDist = 5;
//    angle = atan2( double(pointT.y() - pointM.y()),
//                   double(pointT.x() - pointM.x()) );
//    double arrowSlope = angle + ATAN;
//    cosx = ( cos( arrowSlope ) ) * arrowDist;
//    siny = ( sin( arrowSlope ) ) * arrowDist;
//    m_ParallelLines[ 2 ] = QPoint( pointT.x() - (int)cosx, pointT.y() - (int)siny );
//    arrowSlope = angle - ATAN;
//    cosx = ( cos( arrowSlope ) ) * arrowDist;
//    siny = ( sin( arrowSlope ) ) * arrowDist;
//    m_ParallelLines[ 3 ] = QPoint( pointT.x() - (int)cosx, pointT.y() - (int)siny );
//}

/**
 * Updates the parallel line.
 * Call after calculating the new position.
 */
//:TODO:
//void AssociationLine::updateParallelLine()
//{
//    if( !m_bParallelLineCreated )
//        return;
//    QGraphicsLineItem* line = 0;
//    QPointF common = m_ParallelLines.at( 0 );
//    QPointF p = m_ParallelLines.at( 1 );
//    line = m_ParallelList.at( 0 );
//    line->setLine( common.x(), common.y(), p.x(), p.y() );

//    p = m_ParallelLines.at( 2 );
//    line = m_ParallelList.at( 1 );
//    line->setLine( common.x(), common.y(), p.x(), p.y() );

//    p = m_ParallelLines.at( 3 );
//    line = m_ParallelList.at( 2 );
//    line->setLine( common.x(), common.y(), p.x(), p.y() );
//}

/**
 * This method simply ensures presence of two points and delegates
 * calculation to AssociationLine::calculateEndPoints()
 */
void AssociationLine::calculateInitialEndPoints()
{
    if (m_associationWidget->isSelf() && count() < 4) {
        for (int i = count(); i < 4; ++i) {
            insertPoint(i, QPointF());
        }
        UMLWidget *wid = m_associationWidget->widgetForRole(Uml::RoleType::B);
        if (!wid) {
            uError() << "AssociationWidget is partially constructed."
                "UMLWidget for role A is null.";
            return;
        }
        const QRectF rect = m_associationWidget->mapFromScene(
                mapToScene(wid->rect()).boundingRect()).boundingRect();

        qreal l = rect.left() + .25 * rect.width();
        qreal r = rect.left() + .75 * rect.width();
        bool drawAbove = rect.top() >= SelfAssociationMinimumHeight;
        qreal y = drawAbove ? rect.top() : rect.bottom();
        qreal yOffset = SelfAssociationMinimumHeight;
        if (drawAbove) {
            yOffset *= -1.0;
        }

        setPoint(0, QPointF(l, y));
        setPoint(1, QPointF(l, y + yOffset));
        setPoint(2, QPointF(r, y + yOffset));
        setPoint(3, QPointF(r, y));
    } else if (!m_associationWidget->isSelf() && count() < 2) {
        setEndPoints(QPointF(), QPointF());
    }
//:TODO:    calculateEndPoints();
}

/**
 * This method calculates the line to represent association class if
 * the AssociationWidget represented by this line is an Association
 * class.
 */
/*:TODO:
void AssociationLine::calculateAssociationClassLine()
{
    ClassifierWidget* assocClass = m_associationWidget->associationClass();
    if (!assocClass) {
        delete m_associationClassLine;
        m_associationClassLine = 0;
        return;
    }

    QPointF other = mapToScene(assocClass->rect()).boundingRect().center();

    QLineF possibleAssocLine;

    int numSegments = count()-1;
    for (int i = 0; i < numSegments; ++i) {
        QPointF mid = (m_points.at(i) + m_points.at(i+1)) * 0.5;
        mid = m_associationWidget->mapToScene(mid);
        QLineF newLine(mid, other);

        if (possibleAssocLine.isNull() ||
                (newLine.length() < possibleAssocLine.length())) {
            possibleAssocLine = newLine;
        }
    }

    if (!m_associationClassLine) {
        m_associationClassLine = new QGraphicsLineItem(0);

        QPen p = pen();
        p.setStyle(Qt::DashLine);
        m_associationClassLine->setPen(p);
    }

    QPointF intersectionPoint;
    intersectedRegion(mapToScene(assocClass->rect()).boundingRect(), possibleAssocLine,
            &intersectionPoint);
    possibleAssocLine.setP2(intersectionPoint);

    m_associationClassLine->setLine(possibleAssocLine);
}
*/

/**
 * @return Whether the association class line contains \a pos or not.
 */
bool AssociationLine::onAssociationClassLine(const QPointF& pos) const
{
    if (!m_associationClassLine) {
        return false;
    }
    const QPointF mapped = m_associationClassLine->mapFromParent(pos);
    return m_associationClassLine->contains(mapped);
}
/**
 * This method creates, deletes symbols and collaboration lines based on
 * m_associationWidget->associationType().
 *
 * Call this method when associationType of m_associationWidget changes.
 */
void AssociationLine::reconstructSymbols()
{
    switch( m_associationWidget->associationType() ) {
        case Uml::AssociationType::State:
        case Uml::AssociationType::Activity:
        case Uml::AssociationType::Exception:
        case Uml::AssociationType::UniAssociation:
        case Uml::AssociationType::Dependency:
            setStartSymbol(Symbol::None);
            setEndSymbol(Symbol::OpenArrow);
            removeSubsetSymbol();
            removeCollaborationLine();
            break;

        case Uml::AssociationType::Relationship:
            setStartSymbol(Symbol::None);
            setEndSymbol(Symbol::CrowFeet);
            removeSubsetSymbol();
            removeCollaborationLine();
            break;

        case Uml::AssociationType::Generalization:
        case Uml::AssociationType::Realization:
            setStartSymbol(Symbol::None);
            setEndSymbol(Symbol::ClosedArrow);
            removeSubsetSymbol();
            removeCollaborationLine();
            break;

        case Uml::AssociationType::Composition:
        case Uml::AssociationType::Aggregation:
            setStartSymbol(Symbol::Diamond);
            setEndSymbol(Symbol::None);
            removeSubsetSymbol();
            removeCollaborationLine();
            break;

        case Uml::AssociationType::Containment:
            setStartSymbol(Symbol::Circle);
            setEndSymbol(Symbol::None);
            removeSubsetSymbol();
            removeCollaborationLine();
            break;

        case Uml::AssociationType::Child2Category:
            setStartSymbol(Symbol::None);
            setEndSymbol(Symbol::None);
            createSubsetSymbol();
            removeCollaborationLine();
            break;

        case Uml::AssociationType::Coll_Message:
        case Uml::AssociationType::Coll_Message_Self:
            setStartSymbol(Symbol::None);
            setEndSymbol(Symbol::None);
            removeSubsetSymbol();
            createCollaborationLine();
            break;

        default:
            break;
    }
    alignSymbols();
}

/**
 * Sets the Symbol to appear at the first line segment to \a symbol.
 *
 * If symbol == Symbol::None , then it deletes the symbol item.
 */
void AssociationLine::setStartSymbol(Symbol::SymbolType symbolType)
{
    Q_ASSERT(symbolType != Symbol::Count);
    if (symbolType == Symbol::None) {
        delete m_startSymbol;
        m_startSymbol = 0;
        return;
    }

    if (m_startSymbol) {
        m_startSymbol->setSymbolType(symbolType);
    }
    else {
        m_startSymbol = new Symbol(symbolType, m_associationWidget);
    }
    m_startSymbol->setPen(pen());
    m_startSymbol->setBrush(brush());
}

/**
 * Sets the Symbol to appear at the last line segment to \a symbol.
 *
 * If symbol == Symbol::None , then it deletes the symbol item.
 */
void AssociationLine::setEndSymbol(Symbol::SymbolType symbolType)
{
    Q_ASSERT(symbolType != Symbol::Count);
    if (symbolType == Symbol::None) {
        delete m_endSymbol;
        m_endSymbol = 0;
        return;
    }

    if (m_endSymbol) {
        m_endSymbol->setSymbolType(symbolType);
    }
    else {
        m_endSymbol = new Symbol(symbolType, m_associationWidget);
    }
    m_endSymbol->setPen(pen());
    m_endSymbol->setBrush(brush());
}

/**
 * Constructs a new subset symbol.
 */
void AssociationLine::createSubsetSymbol()
{
    delete m_subsetSymbol; // recreate
    m_subsetSymbol = new Symbol(Symbol::Subset, m_associationWidget);
    m_subsetSymbol->setPen(pen());
    m_subsetSymbol->setBrush(brush());
}

/**
 * Removes the subset symbol if it existed by deleting appropriate items.
 */
void AssociationLine::removeSubsetSymbol()
{
    delete m_subsetSymbol;
    m_subsetSymbol = 0;
}

/**
 * Constructs the open arrow symbol and arrow line, that would represent Collaboration line.
 */
void AssociationLine::createCollaborationLine()
{
    const QPen p = pen();

    // recreate
    delete m_collaborationLineItem;
    delete m_collaborationLineHead;

    m_collaborationLineItem = new QGraphicsLineItem(m_associationWidget);
    m_collaborationLineItem->setPen(p);

    m_collaborationLineHead = new Symbol(Symbol::OpenArrow, m_associationWidget);
    m_collaborationLineHead->setPen(p);
}

/**
 * Removes collaboration line by deleting the head and line item.
 */
void AssociationLine::removeCollaborationLine()
{
    delete m_collaborationLineItem;
    m_collaborationLineItem = 0;

    delete m_collaborationLineHead;
    m_collaborationLineHead = 0;
}

/**
 * This method aligns both the \b "start" and \b "end" symbols to
 * the current angles of the \b "first" and the \b "last" line
 * segment respectively.
 */
void AssociationLine::alignSymbols()
{
    const int sz = m_points.size();
    if (sz < 2) {
        // cannot align if there is no line (one line = 2 points)
        return;
    }

    if (m_startSymbol) {
        QLineF segment(m_points[1], m_points[0]);
        m_startSymbol->alignTo(segment);
    }

    if (m_endSymbol) {
        QLineF segment(m_points[sz-2], m_points[sz - 1]);
        m_endSymbol->alignTo(segment);
    }

    if (m_subsetSymbol) {
        QLineF segment(m_points.at(0), (m_points.at(0) + m_points.at(1)) * .5);
        DEBUG(DBG_SRC) << "points: " << m_points.at(0) << m_points.at(1);
        DEBUG(DBG_SRC) << "segment: " << segment;
        m_subsetSymbol->alignTo(segment);
    }

    if (m_collaborationLineItem) {
        const qreal distance = 10;
        const int midSegmentIndex = (sz - 1) / 2;

        const QPointF a = m_points.at(midSegmentIndex);
        const QPointF b = m_points.at(midSegmentIndex + 1);

        const QPointF p1 = (a + b) / 2.0;
        const QPointF p2 = (p1 + b) / 2.0;

        // Reversed line as we want normal in opposite direction.
        QLineF segment(p2, p1);
        QLineF normal = segment.normalVector().unitVector();
        normal.setLength(distance);

        QLineF actualLine;
        actualLine.setP2(normal.p2());

        normal.translate(p1 - p2);
        actualLine.setP1(normal.p2());

        m_collaborationLineItem->setLine(actualLine);
        m_collaborationLineHead->alignTo(actualLine);
    }
}

/**
 * @return The bounding rectangle for the AssociationLine.
 */
QRectF AssociationLine::boundingRect() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(qMax<qreal>(SelectedPointDiameter, pen().widthF()) + 2.0);  // allow delta region
    return stroker.createStroke(shape()).boundingRect();
}

/**
 * @return The shape of the AssociationLine.
 */
QPainterPath AssociationLine::shape() const
{
    if (m_points.count() > 0) {
        QPainterPath path;
        switch (m_layout) {
        case Direct:
            path.moveTo(m_points.first());
            path.lineTo(m_points.last());
            break;

        case Spline:
            path = createCubicBezierCurve(m_points);
            break;

        case Orthogonal:
            path = createOrthogonalPath(m_points);
            break;

        case Polyline:
        default:
            QPolygonF polygon(m_points);
            path.addPolygon(polygon);
            break;
        }
        return path;
    }
    else {
        return QPainterPath();
    }
}

/**
 * Convert enum LayoutType to string.
 */
QString AssociationLine::toString(LayoutType layout)
{
    return QLatin1String(ENUM_NAME(AssociationLine, LayoutType, layout));
}

/**
 * Set the layout type of the association line.
 * @param layout   the desired layout to set
 */
void AssociationLine::setLayout(LayoutType layout)
{
    m_layout = layout;
    DEBUG(DBG_SRC) << "new layout = " << toString(m_layout);
    update();
}

/**
 * Returns a Bézier path from given points.
 * @param points   points which define the Bézier curve
 * @return   cubic Bézier spline
 */
QPainterPath AssociationLine::createCubicBezierCurve(QVector<QPointF> points)
{
    QPainterPath path;
    if (points.size() > 3) {
        path.moveTo(points.at(0));
        int i = 1;
        while (i + 2 < points.size()) {
            path.cubicTo(points.at(i), points.at(i+1), points.at(i+2));
            i += 3;
        }
        while (i < points.size()) {
            path.lineTo(points.at(i));
            ++i;
        }
    }
    else {
        QPolygonF polygon(points);
        path.addPolygon(polygon);
    }
    return path;
}

/**
 * Returns an orthogonal path constructed of vertical and horizontal segments
 * through the given points.
 * @param points   base points for the path
 * @return   orthogonal path
 */
QPainterPath AssociationLine::createOrthogonalPath(QVector<QPointF> points)
{
    QPainterPath path;
    if (points.size() > 1) {
        QPointF start  = points.first();
        QPointF end    = points.last();
        qreal deltaX = abs(start.x() - end.x());
        qreal deltaY = abs(start.y() - end.y());
        DEBUG("AssociationLine") << "start=" << start << " / end=" << end
                       << " / deltaX=" << deltaX << " / deltaY=" << deltaY;
        QVector<QPointF> vector;
        for (int i = 0; i < points.size() - 1; ++i) {
            QPointF curr = points.at(i);
            QPointF next = points.at(i+1);
            QPointF center = (next + curr)/2.0;

            vector.append(curr);
            if (deltaX < deltaY) {
                // go vertical first
                vector.append(QPointF(curr.x(), center.y()));
                vector.append(QPointF(next.x(), center.y()));
            }
            else {
                // go horizontal first
                vector.append(QPointF(center.x(), curr.y()));
                vector.append(QPointF(center.x(), next.y()));
            }
            vector.append(next);
        }

        QPolygonF rectLine(vector);
        path.addPolygon(rectLine);
    }
    else {
        QPolygonF polygon(points);
        path.addPolygon(polygon);
    }
    return path;
}

/**
 * Reimplemented from QGraphicsItem::paint.
 * Draws the AssociationLine and also takes care of highlighting active point or line.
 */
void AssociationLine::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget)
    QPen _pen = pen();
#ifdef HOVER_EFFECT
    const QColor orig = _pen.color().lighter();
    QColor invertedColor(orig.green(), orig.blue(), orig.red());
    if (invertedColor == _pen.color()) {
        // Ensure different color.
        invertedColor.setRed((invertedColor.red() + 50) % 256);
    }
    invertedColor.setAlpha(150);
#endif

    int sz = m_points.size();
    if (sz < 1) {
        // not enough points - do nothing
        return;
    }

    QPointF savedStart = m_points.first();
    QPointF savedEnd = m_points.last();

    // modify the m_points array not to include the Symbol, the value depends on Symbol
    if (m_startSymbol) {
        QPointF newStart = m_startSymbol->mapToParent(m_startSymbol->symbolEndPoints().first);
        m_points[0] = newStart;
    }

    if (m_endSymbol) {
        QPointF newEnd = m_endSymbol->mapToParent(m_endSymbol->symbolEndPoints().first);
        m_points[sz - 1] = newEnd;
    }

    painter->setPen(_pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(shape());

    if (option->state & QStyle::State_Selected) {
        // set color for selected painting
//:TODO:
//        const QColor orig = _pen.color().lighter();
//        QColor invertedColor(orig.green(), orig.blue(), orig.red());
//        if (invertedColor == _pen.color()) {
//            // Ensure different color.
//            invertedColor.setRed((invertedColor.red() + 50) % 256);
//        }
//        invertedColor.setAlpha(150);
        _pen.setColor(Qt::blue);
        QRectF ellipse(0, 0, SelectedPointDiameter, SelectedPointDiameter);
        painter->setBrush(_pen.color());
        painter->setPen(Qt::NoPen);
        ellipse.moveCenter(savedStart);
        painter->drawRect(ellipse);
        for (int i = 1; i < sz-1; ++i) {
#ifdef HOVER_EFFECT
            if (i != m_activePointIndex) {
#endif
                ellipse.moveCenter(m_points.at(i));
                painter->drawRect(ellipse);
#ifdef HOVER_EFFECT
            }
#endif
        }
        ellipse.moveCenter(savedEnd);
        painter->drawRect(ellipse);

#ifdef HOVER_EFFECT
        if (m_activePointIndex != -1) {
            ellipse.moveCenter(m_points.at(m_activePointIndex));
            painter->setBrush(invertedColor);
            painter->setPen(Qt::NoPen);
            painter->drawEllipse(ellipse);
        }
        else if (m_activeSegmentIndex != -1) {
            painter->setPen(QPen(invertedColor, _pen.widthF() + 1));
            painter->setBrush(Qt::NoBrush);

            QLineF segmentLine(m_points[m_activeSegmentIndex], m_points[m_activeSegmentIndex + 1]);
            painter->drawLine(segmentLine);
        }
#endif

//:TODO:
//        QPainterPathStroker stroker;
//        stroker.setWidth(3.0);
//        QPainterPath outline = stroker.createStroke(shape());
//        painter->drawPath(outline);
        if (Tracer::instance()->isEnabled(metaObject()->className())) {
            QPen p(Qt::green);
            painter->setPen(p);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(shape());
        }
    }

    // now restore the points array
    m_points[0] = savedStart;
    m_points[sz - 1] = savedEnd;
}

/**
 * Determines the active point or segment, the latter being given more priority.
 */
void AssociationLine::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    DEBUG(DBG_SRC) << "at " << event->pos();
    if (event->buttons() & Qt::LeftButton) {
#ifdef HOVER_EFFECT
        m_activePointIndex = closestPointIndex(event->pos());
        if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
            // end points are not drawn and hence not active
            m_activePointIndex = -1;
        }
        // calculate only if active point index is -1
        m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : closestSegmentIndex(event->pos());
#endif
    }
    else if (event->buttons() & Qt::RightButton) {
        DEBUG(DBG_SRC) << "call context menu of association widget at " << event->pos();
    }
    else {
        m_activePointIndex   = -1;
        m_activeSegmentIndex = -1;
    }
}

/**
 * Moves the point or line if active.
 */
void AssociationLine::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
#ifdef HOVER_EFFECT
    if (m_activePointIndex != -1) {
        setPoint(m_activePointIndex, event->pos());
    }
    else if (m_activeSegmentIndex != -1 && !isEndSegmentIndex(m_activeSegmentIndex)) {
        QPointF delta = event->scenePos() - event->lastScenePos();
        setPoint(m_activeSegmentIndex, m_points[m_activeSegmentIndex] + delta);
        setPoint(m_activeSegmentIndex + 1, m_points[m_activeSegmentIndex + 1] + delta);
    }
    else {
        return;
    }
#endif
}

/**
 * Reset active indices and also push undo command.
 */
void AssociationLine::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_activeSegmentIndex = -1;
        m_activePointIndex   = -1;
    }
}

/**
 * Inserts a new point at double click position.
 */
//void AssociationLine::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
//{
//    DEBUG(DBG_SRC) << "at " << event->pos();
//    event->accept();
//    int index = closestPointIndex(event->pos());
//    // First check if double click was on a non end point.
//    if (index != -1 && !isEndPointIndex(index)) {
//        if (!m_associationWidget->isSelf() || count() > 4) {
//            removePoint(index);
//        }
//    }
//    else {
//        // Else insert a new point on the line segment
//        index = closestSegmentIndex(event->pos());
//        if (index != -1) {
//            // Insert after the obtained index.
//            insertPoint(index + 1, event->pos());
//        }
//    }
//}

/**
 * Calculates the "to be highlighted" point and segment indicies
 * and updates if necessary.
 */
void AssociationLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
#ifdef HOVER_EFFECT
    DEBUG(DBG_SRC) << "at " << event->pos();
    int oldPointIndex = m_activePointIndex;
    int oldSegmentIndex = m_activeSegmentIndex;

    m_activePointIndex = closestPointIndex(event->pos());
    // End points are not drawn and hence not active.
    if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
        m_activePointIndex = -1;
    }
    // Activate segment index only if point index is -1
    m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : closestSegmentIndex(event->pos());

    bool isChanged = (oldSegmentIndex != m_activeSegmentIndex || oldPointIndex != m_activePointIndex);
    if (isChanged) {
        m_associationWidget->update();
    }
#endif
}

/**
 * Calculates the "to be highlighted" point and segment indicies
 * and updates if necessary.
 */
void AssociationLine::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
#ifdef HOVER_EFFECT
    int oldPointIndex = m_activePointIndex;
    int oldSegmentIndex = m_activeSegmentIndex;

    m_activePointIndex = closestPointIndex(event->pos());
    // End points are not drawn and hence not active.
    if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
        m_activePointIndex = -1;
    }
    // Activate segment index only if point index is -1
    m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : closestSegmentIndex(event->pos());

    bool isChanged = (oldSegmentIndex != m_activeSegmentIndex || oldPointIndex != m_activePointIndex);
    if (isChanged) {
        m_associationWidget->update();
    }
#endif
}

/**
 * Reset active indicies and updates.
 */
void AssociationLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
#ifdef HOVER_EFFECT
    DEBUG(DBG_SRC) << "at " << event->pos();
    //Q_UNUSED(event)
    m_activePointIndex   = -1;
    m_activeSegmentIndex = -1;
    m_associationWidget->update();
#endif
}

//-----------------------------------------------------------------------------

/**
 * SymbolEndPoints:
 * The first point is where the AssociationLine's visible line is
 * supposed to end.
 * The second points is where the actual symbol part is to appear.
 */
Symbol::SymbolProperty Symbol::symbolTable[Count] =
{
    {
        QRectF(-6, 0, 12, 10), QPainterPath(), QLineF(0, 0, 0, 10),
        SymbolEndPoints(QPointF(0, 10), QPointF(0, 10))
    },
    {
        QRectF(-6, 0, 12, 10), QPainterPath(), QLineF(0, 0, 0, 10),
        SymbolEndPoints(QPointF(0, 0), QPointF(0, 10))
    },
    {
        QRectF(-6, 0, 12, 10), QPainterPath(), QLineF(0, 0, 0, 10),
        SymbolEndPoints(QPointF(0, 10), QPointF(0, 10))
    },
    {
        QRectF(-5, -10, 10, 20), QPainterPath(), QLineF(0, -10, 0, 10),
        SymbolEndPoints(QPointF(0, -10), QPointF(0, 10))
    },
    {
        QRectF(-15, -10, 30, 20), QPainterPath(), QLineF(-10, 0, 0, 0),
        SymbolEndPoints(QPointF(0, 0), QPointF(0, 0))
    },
    {
        QRectF(-8, -8, 16, 16), QPainterPath(), QLineF(0, -8, 0, 8),
        SymbolEndPoints(QPointF(0, -8), QPointF(0, 8))
    }

};

/**
 * @internal A convenience method to setup shapes of all symbols.
 */
void Symbol::setupSymbolTable()
{
    SymbolProperty &openArrow = symbolTable[OpenArrow];
    if (openArrow.shape.isEmpty()) {
        QRectF rect = openArrow.boundRect;
        // Defines a 'V' shape arrow fitting in the bound rect.
        openArrow.shape.moveTo(rect.topLeft());
        openArrow.shape.lineTo(rect.center().x(), rect.bottom());
        openArrow.shape.lineTo(rect.topRight());
    }

    SymbolProperty &closedArrow = symbolTable[ClosedArrow];
    if (closedArrow.shape.isEmpty()) {
        QRectF rect = closedArrow.boundRect;
        // Defines a 'V' shape arrow fitting in the bound rect.
        closedArrow.shape.moveTo(rect.topLeft());
        closedArrow.shape.lineTo(rect.center().x(), rect.bottom());
        closedArrow.shape.lineTo(rect.topRight());
        closedArrow.shape.lineTo(rect.topLeft());
    }

    SymbolProperty &crowFeet = symbolTable[CrowFeet];
    if (crowFeet.shape.isEmpty()) {
        QRectF rect = crowFeet.boundRect;
        // Defines a crowFeet fitting in the bound rect.
        QPointF topMid(rect.center().x(), rect.top());

        // left leg
        crowFeet.shape.moveTo(rect.bottomLeft());
        crowFeet.shape.lineTo(topMid);

        // middle leg
        crowFeet.shape.moveTo(rect.center().x(), rect.bottom());
        crowFeet.shape.lineTo(topMid);

        // left leg
        crowFeet.shape.moveTo(rect.bottomRight());
        crowFeet.shape.lineTo(topMid);
    }

    SymbolProperty &diamond = symbolTable[Diamond];
    if (diamond.shape.isEmpty()) {
        QRectF rect = diamond.boundRect;
        // Defines a 'diamond' shape fitting in the bound rect.
        diamond.shape.moveTo(rect.center().x(), rect.top());
        diamond.shape.lineTo(rect.left(), rect.center().y());
        diamond.shape.lineTo(rect.center().x(), rect.bottom());
        diamond.shape.lineTo(rect.right(), rect.center().y());
        diamond.shape.lineTo(rect.center().x(), rect.top());
    }

    SymbolProperty &subset = symbolTable[Subset];
    if (subset.shape.isEmpty()) {
        QRectF rect = subset.boundRect;
        // Defines an arc fitting in bound rect.
        qreal start = 90, span = 180;
        subset.shape.arcMoveTo(rect, start);
        subset.shape.arcTo(rect, start, span);
    }

    SymbolProperty &circle = symbolTable[Circle];
    if (circle.shape.isEmpty()) {
        QRectF rect = circle.boundRect;
        // Defines a circle with a horizontal-vertical cross lines.
        circle.shape.addEllipse(rect);

        circle.shape.moveTo(rect.center().x(), rect.top());
        circle.shape.lineTo(rect.center().x(), rect.bottom());

        circle.shape.moveTo(rect.left(), rect.center().y());
        circle.shape.lineTo(rect.right(), rect.center().y());
    }

}

/**
 * Constructs a Symbol with current symbol being \a symbol and
 * parented to \a parent.
 */
Symbol::Symbol(SymbolType symbolType, QGraphicsItem *parent)
  : QGraphicsItem(parent),
    m_symbolType(symbolType)
{
    // ensure SymbolTable is validly initialized
    setupSymbolTable();
}

/**
 * Destructor.
 */
Symbol::~Symbol()
{
}

/**
 * @return The current symbol being represented.
 */
Symbol::SymbolType Symbol::symbolType() const
{
    return m_symbolType;
}

/**
 * Sets the current symbol type to \a symbol and updates the geometry.
 */
void Symbol::setSymbolType(SymbolType symbolType)
{
    prepareGeometryChange();  // calls update implicitly
    m_symbolType = symbolType;
}

/**
 * Draws the current symbol using the QPainterPath stored for the current
 * symbol.
 */
void Symbol::paint(QPainter *painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    Q_UNUSED(option) Q_UNUSED(widget)
    painter->setPen(m_pen);
    switch (m_symbolType) {
    case ClosedArrow:
    case CrowFeet:
    case Diamond:
        painter->setBrush(m_brush);
        break;
    default:
        break;
    }
    painter->drawPath(Symbol::symbolTable[m_symbolType].shape);
}

/**
 * @return The bound rectangle for this based on current symbol.
 */
QRectF Symbol::boundingRect() const
{
    const qreal adj = .5 * m_pen.widthF();
    return Symbol::symbolTable[m_symbolType].boundRect.
        adjusted(-adj, -adj, adj, adj);
}

/**
 * @return The path for this based on current symbol.
 */
QPainterPath Symbol::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

/**
 * This method aligns *this* Symbol to the line being
 * passed. That is, it ensures that the axis of this symbol aligns
 * exactly with the \a "to" line passed.
 *
 * Also this item is moved such that the second end point of the
 * SymbolEndPoints for the current symbol *collides* with the second end
 * point of \a "to" line.
 */
void Symbol::alignTo(const QLineF& to)
{
    QLineF toMapped(mapFromParent(to.p1()), mapFromParent(to.p2()));

    QLineF origAxis = Symbol::symbolTable[m_symbolType].axisLine;
    QLineF translatedAxis = origAxis.translated(toMapped.p2() - origAxis.p2());

    qreal angle = translatedAxis.angleTo(toMapped);
    rotate(-angle);

    QPointF delta = to.p2() - mapToParent(symbolEndPoints().second);
    moveBy(delta.x(), delta.y());
}

/**
 * @return The end points for the symbol.
 */
Symbol::SymbolEndPoints Symbol::symbolEndPoints() const
{
    return Symbol::symbolTable[m_symbolType].endPoints;
}

/**
 * @return The pen used to draw symbol.
 */
QPen Symbol::pen() const
{
    return m_pen;
}

/**
 * Sets the pen used to draw the symbol.
 */
void Symbol::setPen(const QPen& pen)
{
    prepareGeometryChange();
    m_pen = pen;
}

/**
 * @return The brush used to fill symbol.
 */
QBrush Symbol::brush() const
{
    return m_brush;
}

/**
 * Sets the brush used to fill symbol.
 */
void Symbol::setBrush(const QBrush &brush)
{
    m_brush = brush;
    update();
}

#include "associationline.moc"
