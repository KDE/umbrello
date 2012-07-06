/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "associationline.h"

#include "associationspacemanager.h"
#include "associationwidget.h"
#include "classifierwidget.h"
#include "debug_utils.h"
#include "umlobject.h"
#include "umlwidget.h"

// qt includes
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#define DBG_ASS "AssociationLine"

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
        //Defines an arc fitting in bound rect.
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
Symbol::Symbol(SymbolType symbolType, QGraphicsItem *parent) :
    QGraphicsItem(parent),
    m_firstTime(true)
{
    // Ensure SymbolTable is validly initialized.
    setupSymbolTable();
    setSymbolType(symbolType);
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
    if (m_firstTime) {
        m_firstTime = false;
        // dont crash during this func call in constructor.
    }
    else {
        prepareGeometryChange(); //calls update implicitly
    }
    m_symbolType = symbolType;
}

/**
 * Draws the current symbol using the QPainterPath stored for the current
 * symbol.
 */
void Symbol::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget *)
{
    painter->setPen(m_pen);
    painter->setBrush(m_brush);
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

    QLineF origAxis = axisLine();
    QLineF translatedAxis = origAxis.translated(toMapped.p2() - origAxis.p2());

    qreal angle = translatedAxis.angleTo(toMapped);
    rotate(-angle);

    QPointF delta = to.p2() - mapToParent(symbolEndPoints().second);
    moveBy(delta.x(), delta.y());
}

/**
 * @return The axis line for this item based on current symbol.
 */
QLineF Symbol::axisLine() const
{
    return Symbol::symbolTable[m_symbolType].axisLine;
}

/**
 * @return The end points for the symbol.
 *
 *         The first point is where the AssociationLine's visible line is
 *         supposed to end.
 *
 *         The second points is where the actual symbol part is to
 *         appear.
 */
SymbolEndPoints Symbol::symbolEndPoints() const
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


// Initialize static variables.
const qreal AssociationLine::Delta = 5;
const qreal AssociationLine::SelectedPointDiameter = 8;
const qreal AssociationLine::SelfAssociationMinimumHeight = 30;


/**
 * Constructs a AssociationLine item with its parent being \a parent.
 */
AssociationLine::AssociationLine(AssociationWidget *assoc)
  : QGraphicsObject(),
    m_associationWidget(assoc)
{
    Q_ASSERT(assoc);
    setFlag(QGraphicsLineItem::ItemIsSelectable);
    // initialisation
    m_activePointIndex = m_activeSegmentIndex = -1;
    m_startSymbol = m_endSymbol = m_subsetSymbol = 0;
    m_associationClassLine = 0;
    m_collaborationLineHead = 0;
    m_collaborationLineItem = 0;
    // This tracker is only for debugging and testing purpose.
    m_tracker = new QGraphicsLineItem;
    m_tracker->setPen(QPen(Qt::darkBlue, 1));
    m_tracker->setZValue(100);

    DEBUG_REGISTER(DBG_ASS);
}

/**
 * Destructor.
 */
AssociationLine::~AssociationLine()
{
}

/**
 * Returns the point with a given index.
 * @return point at given index
 */
UMLScenePoint AssociationLine::point(int index) const
{
    if ((index < 0) | (index > m_points.size() - 1)) {
        uError() << "Index " << index << " out of range [0.." << m_points.size() - 1 << "].";
        //:TODO: Q_ASSERT(index >= 0 && index < m_points.size() - 1);
        return QPointF(0.0, 0.0);
    }
    return m_points.at(index);
}

/**
 * Sets the point value at given index to \a point.  Also changes
 * the bounding rect appropriately.
 */
void AssociationLine::setPoint(int index, const UMLScenePoint &point)
{
    if (m_points.at(index) == point) {
        return; // Nothing to change
    }
    m_associationWidget->prepareGeometryChange();
    m_points[index] = point;
    calculateBoundingRect();
}

/**
 * Shortcut for point(0).
 */
UMLScenePoint AssociationLine::startPoint() const
{
    return m_points.at(0);
}

/**
 * Shortcut for point(count()-1).
 */
UMLScenePoint AssociationLine::endPoint() const
{
    return m_points.at(m_points.size()-1);
}

/**
 * Inserts the passed in \a point at the \a index passed in and
 * recalculates the bounding rect.
 */
void AssociationLine::insertPoint(int index, const UMLScenePoint &point)
{
    m_associationWidget->prepareGeometryChange();
    m_points.insert(index, point);
    calculateBoundingRect();
}

/**
 * Removes the point at \a index passed in.
 * @see removeNonEndPoint
 */
void AssociationLine::removePoint(int index)
{
    m_associationWidget->prepareGeometryChange();
    m_points.remove(index);
    calculateBoundingRect();
}

/**
 * @return The number of points in the AssociationLine.
 */
int AssociationLine::count() const
{
    return m_points.size();
}

/**
 * Removes all the points and signals a geometry updation.
 */
void AssociationLine::clear()
{
    if (!m_points.isEmpty()) {
        m_associationWidget->prepareGeometryChange();
        m_points.clear();
        calculateBoundingRect();
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
    m_associationWidget->prepareGeometryChange();
    while (i < m_points.size()) {
        if (m_points.at(i) == m_points.at(i-1)) {
            m_points.remove(i);
        }
        else {
            ++i;
        }
    }
    calculateBoundingRect();
    // TODO: Use delta comparison instead of exact comparison.
}

/**
 * @retval "Index" of the linepoint closest to the \a point
 *         passed.
 * @rertval -1 If no linepoint is closer to passed in \a point.
 *
 * @param point The point which is to be tested for closeness.
 * @param delta The closeness is measured by "delta" which indicates radius
 *              around the linepoint to be regarded as closer.
 */
int AssociationLine::closestPointIndex(const QPointF& point, qreal delta) const
{
    const int sz = m_points.size();
    for(int i = 0; i < sz; ++i) {
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
 * @retval "Index" of the segment on which the point lies (fuzzy)
 * @retval -1 If no segment contains the point
 *
 * @param point The point to be tested for line-segment closure.
 * @param delta Represents extent of fuzzy region around the line
 *              to be tested for point closure.
 */
int AssociationLine::segmentIndex(const QPointF& point, qreal delta) const
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
    const int sz = m_points.size();
    Q_ASSERT(index >= 0 && index < sz);

    return (index == 0 || index == (sz - 1));
}

/**
 * Retval True If segment at \a index is start or end.
 */
bool AssociationLine::isEndSegmentIndex(int index) const
{
    // num of seg = num of points - 1
    const int sz = m_points.size() - 1;
    Q_ASSERT(index >= 0 && index < sz);

    return (index == 0 || index == (sz - 1));
}

/**
 * Sets the start and end points to passed parameters.
 */
void AssociationLine::setEndPoints(const UMLScenePoint &start, const UMLScenePoint &end)
{
    int size = m_points.size();

    m_associationWidget->prepareGeometryChange();

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

    calculateBoundingRect();
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
    QPointF startPoint;
    startPoint.rx() = startElement.attribute("startx", "0").toDouble();
    startPoint.ry() = startElement.attribute("starty", "0").toDouble();
    startPoint = m_associationWidget->mapFromScene(startPoint);

    node = startElement.nextSibling();
    QDomElement endElement = node.toElement();
    if(endElement.isNull() || endElement.tagName() != "endpoint") {
        return false;
    }
    QPointF endPoint;
    endPoint.rx() = endElement.attribute("endx", "0").toDouble();
    endPoint.ry() = endElement.attribute("endy", "0").toDouble();
    endPoint = m_associationWidget->mapFromScene(endPoint);

    setEndPoints(startPoint, endPoint);

    QPointF point;
    node = endElement.nextSibling();
    QDomElement element = node.toElement();
    int i = 1;
    while(!element.isNull()) {
        if(element.tagName() == "point") {
            point.rx() = element.attribute("x", "0").toDouble();
            point.ry() = element.attribute("y", "0").toDouble();
            insertPoint(i++, m_associationWidget->mapFromScene(point));
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

QPen AssociationLine::pen() const
{
    QPen pen(m_associationWidget->lineColor(),
            m_associationWidget->lineWidth(),
            Qt::SolidLine,
            Qt::RoundCap,
            Qt::RoundJoin);
    return pen;
}

/**
 * Update the pen used to draw symbol.
 */
void AssociationLine::updatePenSettings()
{
    m_associationWidget->prepareGeometryChange();
    QPen changedPen = pen();
    if (m_startSymbol) {
        m_startSymbol->setPen(changedPen);
    }
    if (m_endSymbol) {
        m_endSymbol->setPen(changedPen);
    }
    if (m_subsetSymbol) {
        m_subsetSymbol->setPen(changedPen);
    }
    if (m_associationClassLine) {
        changedPen.setStyle(Qt::DashLine);
        m_associationClassLine->setPen(changedPen);
    }
    calculateBoundingRect();
}

/**
 * @return The bounding rectangle for the AssociationLine.
 */
QRectF AssociationLine::boundingRect() const
{
    return m_boundingRect;
}

/**
 * @return The shape of the AssociationLine.
 */
QPainterPath AssociationLine::shape() const
{
    return m_shape;
}

/**
 * Reimplemented from QGraphicsItem::paint.
 * Draws the AssociationLine and also takes care of highlighting active point or line.
 */
void AssociationLine::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(widget);
    QPen _pen = pen();
    const QColor orig = _pen.color().lighter();
    QColor invertedColor(orig.green(), orig.blue(), orig.red());
    if (invertedColor == _pen.color()) {
        // Ensure different color.
        invertedColor.setRed((invertedColor.red() + 50) % 256);
    }
    invertedColor.setAlpha(150);

    int sz = m_points.size();
    if (sz == 0) {
        return;
    }

    QPointF savedStart = m_points.first();
    QPointF savedEnd = m_points.last();

    // Modify the m_points array not to include the Symbol, the value depends on Symbol.
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

    painter->drawPolyline(m_points.constData(), m_points.size());

    if (option->state & QStyle::State_Selected) {
        QRectF ellipse(0, 0, SelectedPointDiameter, SelectedPointDiameter);
        painter->setBrush(_pen.color());
        painter->setPen(Qt::NoPen);
        for (int i = 1; i < sz - 1; ++i) {
            if (i != m_activePointIndex) {
                ellipse.moveCenter(m_points.at(i));
                painter->drawEllipse(ellipse);
            }
        }

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
    }

    // Now restore the points array
    m_points[0] = savedStart;
    m_points[sz - 1] = savedEnd;
}

/**
 * Determines the active point or segment, the latter being given more priority.
 */
void AssociationLine::mousePressEvent(UMLSceneMouseEvent *event)
{
    DEBUG(DBG_ASS) << "at " << event->pos();
    if (event->buttons() & Qt::LeftButton) {
        m_activePointIndex = closestPointIndex(event->pos());
        if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
            // End points are not drawn and hence not active.
            m_activePointIndex = -1;
        }
        // calculate only if active point index is -1
        m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : segmentIndex(event->pos());
    }
    else if (event->buttons() & Qt::RightButton) {
        DEBUG(DBG_ASS) << "context menu of association widget at " << event->pos();
        m_associationWidget->contextMenu(event->pos(), event->modifiers(), event->screenPos());
    }
    else {
        m_activePointIndex   = -1;
        m_activeSegmentIndex = -1;
    }
}

/**
 * Moves the point or line if active.
 */
void AssociationLine::mouseMoveEvent(UMLSceneMouseEvent *event)
{
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
    calculateEndPoints();
}

/**
 * Reset active indices and also push undo command.
 */
void AssociationLine::mouseReleaseEvent(UMLSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_activeSegmentIndex = -1;
        m_activePointIndex   = -1;
    }
}

/**
 * Inserts a new point at double click position.
 */
void AssociationLine::mouseDoubleClickEvent(UMLSceneMouseEvent *event)
{
    DEBUG(DBG_ASS) << "at " << event->pos();
    event->accept();
    int index = closestPointIndex(event->pos());
    // First check if double click was on a non end point.
    if (index != -1 && !isEndPointIndex(index)) {
        if (!m_associationWidget->isSelf() || count() > 4) {
            removePoint(index);
        }
    }
    else {
        // Else insert a new point on the line segment
        index = segmentIndex(event->pos());
        if (index != -1) {
            // Insert after the obtained index.
            insertPoint(index + 1, event->pos());
        }
    }
}

/**
 * Calculates the "to be highlighted" point and segment indicies
 * and updates if necessary.
 */
void AssociationLine::hoverEnterEvent(UMLSceneHoverEvent *event)
{
    DEBUG(DBG_ASS) << "at " << event->pos();
    int oldPointIndex = m_activePointIndex;
    int oldSegmentIndex = m_activeSegmentIndex;

    m_activePointIndex = closestPointIndex(event->pos());
    // End points are not drawn and hence not active.
    if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
        m_activePointIndex = -1;
    }
    // Activate segment index only if point index is -1
    m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : segmentIndex(event->pos());

    bool isChanged = (oldSegmentIndex != m_activeSegmentIndex || oldPointIndex != m_activePointIndex);
    if (isChanged) {
        m_associationWidget->update();
    }
}

/**
 * Calculates the "to be highlighted" point and segment indicies
 * and updates if necessary.
 */
void AssociationLine::hoverMoveEvent(UMLSceneHoverEvent *event)
{
    int oldPointIndex = m_activePointIndex;
    int oldSegmentIndex = m_activeSegmentIndex;

    m_activePointIndex = closestPointIndex(event->pos());
    // End points are not drawn and hence not active.
    if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
        m_activePointIndex = -1;
    }
    // Activate segment index only if point index is -1
    m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : segmentIndex(event->pos());

    bool isChanged = (oldSegmentIndex != m_activeSegmentIndex || oldPointIndex != m_activePointIndex);
    if (isChanged) {
        m_associationWidget->update();
    }
}

/**
 * Reset active indicies and updates.
 */
void AssociationLine::hoverLeaveEvent(UMLSceneHoverEvent *event)
{
    DEBUG(DBG_ASS) << "at " << event->pos();
    //Q_UNUSED(event);
    m_activePointIndex   = -1;
    m_activeSegmentIndex = -1;
    m_associationWidget->update();
}

/**
 * Calculates the "shape" and also the "bounding rectangle"
 * required by GraphicsView framework.
 */
void AssociationLine::calculateBoundingRect()
{
    if (m_points.isEmpty()) {
        m_shape = QPainterPath();
        m_boundingRect = QRectF();
        return;
    }

    QPainterPath path(m_points.first());
    for(int i = 1; i < m_points.size(); ++i) {
        path.lineTo(m_points.at(i));
    }

    QRectF ellipse(0, 0, SelectedPointDiameter, SelectedPointDiameter);
    foreach(QPointF point, m_points) {
        ellipse.moveCenter(point);
        path.addEllipse(ellipse);
    }

    QPainterPathStroker stroker;
    stroker.setWidth(qMax(AssociationLine::Delta, pen().widthF()) + 2); // allow delta region

    m_shape = stroker.createStroke(path);
    m_boundingRect = m_shape.boundingRect();

    alignSymbols();
}

/**
 * @internal
 * @short A helper method to return the region of rect intersected by line.
 */
static Uml::Region intersectedRegion(const QRectF& rect,
        const QLineF& line, QPointF *intersectionPoint = 0)
{
    // This maps the region of rect to QLineF objects representing rects' edges.
    QMap<Uml::Region, QLineF> rectLines;

    // Do the mapping.
    rectLines[Uml::reg_West]  = QLineF(rect.topLeft(), rect.bottomLeft());
    rectLines[Uml::reg_North] = QLineF(rect.topLeft(), rect.topRight());
    rectLines[Uml::reg_East]  = QLineF(rect.topRight(), rect.bottomRight());
    rectLines[Uml::reg_South] = QLineF(rect.bottomLeft(), rect.bottomRight());

    // This holds whether a given rect edge(represented by QLineF
    // objects) is intersected by line.
    typedef QPair<bool, QPointF> BoolPointPair;

    QMap<Uml::Region, BoolPointPair> intersectionMap;
    for (int i = Uml::reg_West; i <= Uml::reg_South; ++i) {
        Uml::Region r = (Uml::Region)i;

        BoolPointPair value;
        value.first = (line.intersect(rectLines[r], &value.second) ==
                QLineF::BoundedIntersection);
        intersectionMap[r] = value;
    }

    {
        // Do intersection mapping separately for corner regions.
        BoolPointPair value;

        value.first = intersectionMap[Uml::reg_North].first == true
            && intersectionMap[Uml::reg_West].first == true;
        value.second = rect.topLeft();
        intersectionMap[Uml::reg_NorthWest] = value;

        value.first = intersectionMap[Uml::reg_North].first == true
            && intersectionMap[Uml::reg_East].first == true;
        value.second = rect.topRight();
        intersectionMap[Uml::reg_NorthEast] = value;

        value.first = intersectionMap[Uml::reg_South].first == true
            && intersectionMap[Uml::reg_West].first == true;
        value.second = rect.bottomLeft();
        intersectionMap[Uml::reg_SouthWest] = value;

        value.first = intersectionMap[Uml::reg_South].first == true
            && intersectionMap[Uml::reg_East].first == true;
        value.second = rect.bottomRight();
        intersectionMap[Uml::reg_SouthEast] = value;
    }

    Uml::Region intersection = Uml::reg_Error;
    for (int i = Uml::reg_West; i <= Uml::reg_SouthWest; ++i) {
        const Uml::Region reg = (Uml::Region)i;
        BoolPointPair value = intersectionMap[reg];
        if (value.first == true) {
            intersection = reg;
            if (intersectionPoint) {
                *intersectionPoint = value.second;
            }
        }
    }

    return intersection;
}

/**
 * This method determines new regions for both ends of this AssociationLine
 * @see intersectedRegion()
 */
RegionPair AssociationLine::determineRegions()
{
    if (m_associationWidget->isSelf() && count() < 4) {
        return RegionPair();
    }
    if (!m_associationWidget->isSelf() && count() < 2) {
        return RegionPair();
    }

    UMLWidget *widA = m_associationWidget->widgetForRole(Uml::A);
    UMLWidget *widB = m_associationWidget->widgetForRole(Uml::B);

    if (!widA || !widB) {
        uError() << "AssociationWidget is only partially constructed."
            "One or both UMLWidget are null";
        return RegionPair();
    }

    QRectF aRect = widA->sceneRect();
    QRectF bRect = widB->sceneRect();

    // aLine and bLine are QLineF objects whose intersection with the
    // rectangle edges determine the AssociationLine's endings regions
    // occupied in both the UMLWidgets.
    // Intially for count <= 2, this is line joining centres of two
    // rects.
    // For count > 2, we set the AssociationLine's penultimate point from
    // corresponding widgets, as line's other end.
    QLineF  aLine = QLineF(aRect.center(), bRect.center());
    QLineF bLine = aLine;
    if (count() > 2) {
        aLine.setP2(m_associationWidget->mapToScene(point(1)));
        bLine.setP1(m_associationWidget->mapToScene(point(count()-2)));
    }

    RegionPair result;
    result[Uml::A] = intersectedRegion(aRect, aLine);
    result[Uml::B] = intersectedRegion(bRect, bLine);
    if (result[Uml::A] == Uml::reg_Error) {
        result[Uml::A] = Uml::reg_North;
    }
    if (result[Uml::B] == Uml::reg_Error) {
        result[Uml::B] = Uml::reg_North;
    }

    return result;
}
/**
 * This important method is responsible for determining the appropriate
 * regions of UMLWidget to be occupied by this AssociationLine ends.
 *
 * After determining, AssociationSpaceManager::arrange() is invoked for
 * the UMLWidget's modified region space to set the actual end points of
 * this AssociationLine.
 */
void AssociationLine::calculateEndPoints()
{
    UMLWidget *aWid = m_associationWidget->widgetForRole(Uml::A);
    UMLWidget *bWid = m_associationWidget->widgetForRole(Uml::B);

    if (!aWid || !bWid) {
        uError() << "AssociationWidget is only partially constructed."
            "One or both UMLWidget are null";
        return;
    }

    AssociationSpaceManager *aSpaceManager =
        aWid->associationSpaceManager();
    AssociationSpaceManager *bSpaceManager =
        bWid->associationSpaceManager();

    RegionPair prevRegions(
            aSpaceManager->region(m_associationWidget)[Uml::A],
            bSpaceManager->region(m_associationWidget)[Uml::B]);
    RegionPair newRegions = determineRegions();

    aSpaceManager->remove(m_associationWidget);
    bSpaceManager->remove(m_associationWidget);

    aSpaceManager->add(m_associationWidget, newRegions);
    if (aWid != bWid) {
        bSpaceManager->add(m_associationWidget, newRegions);
    }

    // To minimize multiple calls of arrange on same region.
    typedef QPair<AssociationSpaceManager*, Uml::Region> Pair;
    QSet<Pair> toArrange;
    toArrange << qMakePair(aSpaceManager, prevRegions[Uml::A]);
    toArrange << qMakePair(bSpaceManager, prevRegions[Uml::B]);
    toArrange << qMakePair(aSpaceManager, newRegions[Uml::A]);
    toArrange << qMakePair(bSpaceManager, newRegions[Uml::B]);
    foreach (Pair p, toArrange) {
        p.first->arrange(p.second);
    }
}

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
        UMLWidget *wid = m_associationWidget->widgetForRole(Uml::A);
        if (!wid) {
            uError() << "AssociationWidget is partially constructed."
                "UMLWidget for role A is null.";
            return;
        }
        const QRectF rect = m_associationWidget->mapFromScene(
                wid->sceneRect()).boundingRect();

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
    calculateEndPoints();
}

/**
 * This method calculates the line to represent association class if
 * the AssociationWidget represented by this line is an Association
 * class.
 */
void AssociationLine::calculateAssociationClassLine()
{
    ClassifierWidget* assocClass = m_associationWidget->associationClass();
    if (!assocClass) {
        delete m_associationClassLine;
        m_associationClassLine = 0;
        return;
    }

    QPointF other = assocClass->sceneRect().center();

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
    intersectedRegion(assocClass->sceneRect(), possibleAssocLine,
            &intersectionPoint);
    possibleAssocLine.setP2(intersectionPoint);

    m_associationClassLine->setLine(possibleAssocLine);
}

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
}

/**
 * Constructs a new subset symbol.
 */
void AssociationLine::createSubsetSymbol()
{
    delete m_subsetSymbol; // recreate
    m_subsetSymbol = new Symbol(Symbol::Subset, m_associationWidget);
    m_subsetSymbol->setPen(pen());
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
        // Cannot align if there is no line (one line = 2 points)
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
        DEBUG(DBG_ASS) << "points: " << m_points.at(0) << m_points.at(1);
        DEBUG(DBG_ASS) << "segment: " << segment;
        m_subsetSymbol->alignTo(segment);
    }

    if (m_collaborationLineItem) {
        Q_ASSERT(m_collaborationLineHead != 0);
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
