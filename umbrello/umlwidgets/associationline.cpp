/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "associationline.h"

// application includes
#include "associationwidget.h"
#include "debug_utils.h"
#include "optionstate.h"
#include "umlapp.h"
#include "umldoc.h"
#include "umlwidget.h"

// qt includes
#include <QDomDocument>
#include <QPainter>
#include <QXmlStreamWriter>

// system includes
#include <cstdlib>
#include <cmath>

DEBUG_REGISTER_DISABLED(AssociationLine)

// Initialize static variables.
const qreal AssociationLine::Delta = 5;
const qreal AssociationLine::SelectedPointDiameter = 4;
const qreal AssociationLine::SelfAssociationMinimumHeight = 30;

/**
 * Constructor.
 * Constructs an AssociationLine item slaved to the given AssociationWidget.
 */
AssociationLine::AssociationLine(AssociationWidget *association)
  : QGraphicsObject(association),
    m_associationWidget(association),
    m_activePointIndex(-1),
    m_activeSegmentIndex(-1),
    m_startSymbol(nullptr),
    m_endSymbol(nullptr),
    m_subsetSymbol(nullptr),
    m_collaborationLineItem(nullptr),
    m_collaborationLineHead(nullptr),
    m_layout(Settings::optionState().generalState.layoutType),
    m_autoLayoutSpline(true)
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
    delete m_startSymbol;
    delete m_endSymbol;
    delete m_subsetSymbol;
    removeCollaborationLine();
}

/**
 * Returns the point at the point index.
 * @return point at given index
 */
QPointF AssociationLine::point(int index) const
{
    if ((index < 0) || (index >= m_points.size())) {
        logWarn2("AssociationLine::point: Index %1 out of range [0..%2]",
                 index, m_points.size() - 1);
        return QPointF(-1.0, -1.0);
    }
    return m_points.at(index);
}

/**
 * Sets the point value at given index to \a point.
 */
bool AssociationLine::setPoint(int index, const QPointF &point)
{
    if ((index < 0) | (index >= m_points.size())) {
        logWarn2("AssociationLine::setPoint: Index %1 out of range [0..%2]",
                 index, m_points.size() - 1);
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

void AssociationLine::addPoint(const QPointF &point)
{
    m_points.append(point);
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
    m_activePointIndex = -1;
    m_activeSegmentIndex = -1;
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
 * TODO: Use delta comparison 'closestPointIndex' instead of exact comparison.
 * TODO: Not used anywhere.
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
    m_activePointIndex = -1;
    m_activeSegmentIndex = -1;
    alignSymbols();
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
 * @param delta The distance in pixels that the point may be removed from
 *              a segment but is still considered to be on the segment.
 *
 * @return Index of the line segment closest to the \a point passed;
 *         -1 if no line segment is closer to passed in \a point.
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

bool AssociationLine::isAutoLayouted() const
{
    return m_autoLayoutSpline;
}

bool AssociationLine::enableAutoLayout()
{
    m_autoLayoutSpline = true;
    createSplinePoints();
    path();
    createSplinePoints();
    path();
    update();
    return true;
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
        DEBUG() << i << ". point x:" << p.x() << " / y:" << p.y();
    }
}

/**
 * Loads AssociationLine information saved in \a qElement XMI element.
 */
bool AssociationLine::loadFromXMI(QDomElement &qElement)
{
    QString layout = qElement.attribute(QStringLiteral("layout"), QStringLiteral("polyline"));
    m_layout = Uml::LayoutType::fromString(layout);

    QDomNode node = qElement.firstChild();

    m_points.clear();

    QDomElement startElement = node.toElement();
    if(startElement.isNull() || startElement.tagName() != QStringLiteral("startpoint")) {
        return false;
    }
    UMLScene* umlScene = m_associationWidget->umlScene();
    qreal dpiScale = UMLApp::app()->document()->dpiScale();
    QString x = startElement.attribute(QStringLiteral("startx"), QStringLiteral("0"));
    qreal nX = toDoubleFromAnyLocale(x) + umlScene->fixX();
    QString y = startElement.attribute(QStringLiteral("starty"), QStringLiteral("0"));
    qreal nY = toDoubleFromAnyLocale(y) + umlScene->fixY();
    QPointF startPoint(nX, nY);

    node = startElement.nextSibling();
    QDomElement endElement = node.toElement();
    if(endElement.isNull() || endElement.tagName() != QStringLiteral("endpoint")) {
        return false;
    }
    x = endElement.attribute(QStringLiteral("endx"), QStringLiteral("0"));
    nX = toDoubleFromAnyLocale(x) + umlScene->fixX();
    y = endElement.attribute(QStringLiteral("endy"), QStringLiteral("0"));
    nY = toDoubleFromAnyLocale(y) + umlScene->fixY();
    QPointF endPoint(nX, nY);
    setEndPoints(startPoint * dpiScale, endPoint * dpiScale);
    QPointF point;
    node = endElement.nextSibling();
    QDomElement element = node.toElement();
    int i = 1;
    while (!element.isNull()) {
        if (element.tagName() == QStringLiteral("point")) {
            x = element.attribute(QStringLiteral("x"), QStringLiteral("0"));
            y = element.attribute(QStringLiteral("y"), QStringLiteral("0"));
            point.setX(toDoubleFromAnyLocale(x) + umlScene->fixX());
            point.setY(toDoubleFromAnyLocale(y) + umlScene->fixY());
            insertPoint(i++, point * dpiScale);
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    return true;
}

/**
 * Saves association line information into XMI element named "linepath".
 * @note Stored as linepath for backward compatibility
 */
void AssociationLine::saveToXMI(QXmlStreamWriter& writer)
{
    if (count() == 0)
        return;

    writer.writeStartElement(QStringLiteral("linepath"));
    writer.writeAttribute(QStringLiteral("layout"), Uml::LayoutType::toString(m_layout));
    writer.writeStartElement(QStringLiteral("startpoint"));

    qreal dpiScale = UMLApp::app()->document()->dpiScale();
    QPointF point = m_associationWidget->mapToScene(startPoint());
    point /= dpiScale;
    writer.writeAttribute(QStringLiteral("startx"), QString::number(point.x()));
    writer.writeAttribute(QStringLiteral("starty"), QString::number(point.y()));
    writer.writeEndElement();            // startpoint
    writer.writeStartElement(QStringLiteral("endpoint"));
    point = m_associationWidget->mapToScene(endPoint());
    point /= dpiScale;
    writer.writeAttribute(QStringLiteral("endx"), QString::number(point.x()));
    writer.writeAttribute(QStringLiteral("endy"), QString::number(point.y()));
    writer.writeEndElement();            // endpoint
    for(int i = 1; i < count()-1; ++i) {
        writer.writeStartElement(QStringLiteral("point"));
        point = m_associationWidget->mapToScene(this->point(i));
        point /= dpiScale;
        writer.writeAttribute(QStringLiteral("x"), QString::number(point.x()));
        writer.writeAttribute(QStringLiteral("y"), QString::number(point.y()));
        writer.writeEndElement();        // point
    }
    writer.writeEndElement();   // linepath
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
 * Returns the pen used for drawing.
 */
QPen AssociationLine::pen() const
{
    return m_pen;
}

/**
 * Setup new pen.
 */
void AssociationLine::setPen(const QPen &pen)
{
    if (m_startSymbol) {
        m_startSymbol->setPen(pen);
        // update brush fill color
        m_startSymbol->setBrush(brush());
    }
    if (m_subsetSymbol) {
        m_subsetSymbol->setPen(pen);
        m_subsetSymbol->setBrush(brush());
    }
    if (m_endSymbol) {
        m_endSymbol->setPen(pen);
        m_endSymbol->setBrush(brush());
    }

    prepareGeometryChange();
    m_pen = pen;
    m_pen.setCapStyle(Qt::RoundCap);
    m_pen.setJoinStyle(Qt::RoundJoin);
    updatePenStyle();
}

/**
 * Update pen style depending on the association type of the related AssociationWidget instance.
 */
void AssociationLine::updatePenStyle()
{
    Uml::AssociationType::Enum type = m_associationWidget->associationType();
    if (type == Uml::AssociationType::Dependency  ||
        type == Uml::AssociationType::Realization ||
        type == Uml::AssociationType::Anchor) {
        m_pen.setStyle(Qt::DashLine);
    }
    else {
        m_pen.setStyle(Qt::SolidLine);
    }
}

/**
 * This method simply ensures presence of two points and
 * adds the needed points for self associations.
 */
void AssociationLine::calculateInitialEndPoints()
{
    if (m_associationWidget->isSelf() && count() < 4) {
        for (int i = count(); i < 4; ++i) {
            insertPoint(i, QPointF());
        }
        UMLWidget *wid = m_associationWidget->widgetForRole(Uml::RoleType::B);
        if (!wid) {
            logError0("AssociationLine::calculateInitialEndPoints: "
                      "AssociationWidget is partially constructed."
                      "UMLWidget for role B is null.");
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
    if (m_layout == Uml::LayoutType::Spline)
        createSplinePoints();
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
        case Uml::AssociationType::Exception:
            setLayout(Uml::LayoutType::Polyline);
            // fall through
        case Uml::AssociationType::State:
        case Uml::AssociationType::Activity:
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

        case Uml::AssociationType::Coll_Mesg_Sync:
        case Uml::AssociationType::Coll_Mesg_Async:
        case Uml::AssociationType::Coll_Mesg_Self:
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
        m_startSymbol = nullptr;
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
        m_endSymbol = nullptr;
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
    m_subsetSymbol = nullptr;
}

/**
 * Constructs the open arrow symbol and arrow line, that would represent Collaboration line.
 */
void AssociationLine::createCollaborationLine()
{
    const QPen p = pen();

    //recreate
    removeCollaborationLine();

    m_collaborationLineItem = new QGraphicsLineItem(m_associationWidget);
    m_collaborationLineItem->setPen(p);

    if (m_associationWidget->associationType() == Uml::AssociationType::Coll_Mesg_Sync) {
        m_collaborationLineHead = new Symbol(Symbol::ClosedArrow, m_associationWidget);
        m_collaborationLineHead->setBrush(p.color());
    }
    else
        m_collaborationLineHead = new Symbol(Symbol::OpenArrow, m_associationWidget);
    m_collaborationLineHead->setPen(p);
}

/**
 * Removes collaboration line by deleting the head and line item.
 */
void AssociationLine::removeCollaborationLine()
{
    delete m_collaborationLineItem;
    m_collaborationLineItem = nullptr;

    delete m_collaborationLineHead;
    m_collaborationLineHead = nullptr;
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

    QList<QPolygonF> polygons = path().toSubpathPolygons();

    if (m_startSymbol && polygons.size() > 0) {
        QPolygonF firstLine = polygons.first();
        QLineF segment(firstLine.at(1), firstLine.at(0));
        m_startSymbol->alignTo(segment);
    }

    if (m_endSymbol && polygons.size() > 0) {
        QPolygonF lastLine = polygons.last();
        int maxIndex = lastLine.size();
        QLineF segment(lastLine.at(maxIndex-2), lastLine.at(maxIndex-1));
        m_endSymbol->alignTo(segment);
    }

    if (m_subsetSymbol) {
        QPointF p1 = path().pointAtPercent(0.4);
        QPointF p2 = path().pointAtPercent(0.5);
        QLineF segment(p1, p2);
        m_subsetSymbol->alignTo(segment);
    }

    if (m_collaborationLineItem) {
        const qreal distance = 10;
        const int midSegmentIndex = (sz - 1) / 2;

        const QPointF a = m_points.at(midSegmentIndex);
        const QPointF b = m_points.at(midSegmentIndex + 1);

        if (a == b)
            return;

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
 * @return The path of the AssociationLine.
 */
QPainterPath AssociationLine::path() const
{
    if (m_points.count() == 0) {
        return QPainterPath();
    }
    QPainterPath path;
    switch (m_layout) {
        case Uml::LayoutType::Direct:
            path.moveTo(m_points.first());
            path.lineTo(m_points.last());
            break;

        case Uml::LayoutType::Spline:
            path = createBezierCurve(m_points);
            break;

        case Uml::LayoutType::Orthogonal:
            path = createOrthogonalPath(m_points);
            break;

        case Uml::LayoutType::Polyline:
        default:
            QPolygonF polygon(m_points);
            path.addPolygon(polygon);
            break;
    }
    return path;
}

/**
 * The points are used for the bounding rect. The reason is,
 * that for splines the control points are further away from the path.
 * @return The bounding rectangle for the AssociationLine.
 */
QRectF AssociationLine::boundingRect() const
{
    QPolygonF polygon(m_points);
    QRectF rect = polygon.boundingRect();
    const qreal margin(5.0);
    rect.adjust(-margin, -margin, margin, margin);
    return rect;
}

/**
 * @return The shape of the AssociationLine.
 */
QPainterPath AssociationLine::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(qMax<qreal>(2*SelectedPointDiameter, pen().widthF()) + 2.0);  // allow delta region
    stroker.setCapStyle(Qt::FlatCap);
    return stroker.createStroke(path());
}

/**
 * Convert enum LayoutType to string.
 */
QString AssociationLine::toString(Uml::LayoutType::Enum layout)
{
    return Uml::LayoutType::toString(layout);
}

/**
 * Convert string to enum LayoutType.
 */
Uml::LayoutType::Enum AssociationLine::fromString(const QString &layout)
{
    if (layout == QStringLiteral("Direct"))
        return Uml::LayoutType::Direct;
    if (layout == QStringLiteral("Spline"))
        return Uml::LayoutType::Spline;
    if (layout == QStringLiteral("Orthogonal"))
        return Uml::LayoutType::Orthogonal;
    return Uml::LayoutType::Polyline;
}

/**
 * Return the layout type of the association line.
 * @return   the currently used layout
 */
Uml::LayoutType::Enum AssociationLine::layout() const
{
    return m_layout;
}

/**
 * Set the layout type of the association line.
 * @param layout   the desired layout to set
 */
void AssociationLine::setLayout(Uml::LayoutType::Enum layout)
{
    prepareGeometryChange();
    m_layout = layout;
    DEBUG() << "new layout = " << Uml::LayoutType::toString(m_layout);
    if (m_layout == Uml::LayoutType::Spline) {
        createSplinePoints();
    }
    alignSymbols();
}

/**
 * For a cubic Bezier curve at least four points are needed.
 * If there are less, the missing points will be created.
 * Note: Implementation is only for two points.
 */
void AssociationLine::createSplinePoints()
{
    QPointF c1, c2;
    QPointF p1 = m_points.first();  // start point
    QPointF p2 = m_points.last();   // end point
        
    if (m_autoLayoutSpline) {
        qreal dx = p2.x() - p1.x();
        qreal dy = p2.y() - p1.y();
        /*qreal oneThirdX = 0.33 * dx;
        qreal oneThirdY = 0.33 * dy;
        QPointF c1(p1.x() + oneThirdX,  // control point 1
                   p1.y() - oneThirdY);
        QPointF c2(p2.x() - oneThirdX,  // control point 2
                   p2.y() + oneThirdY);*/
        qreal oneHalfX = 0.5 * dx;
        qreal oneHalfY = 0.5 * dy;
        if (dx > dy) {
            m_c1dx = oneHalfX;
            m_c1dy = 0;
            m_c2dx = -oneHalfX;
            m_c2dy = 0;
        }
        else {
            m_c1dx = 0;
            m_c1dy = oneHalfY;
            m_c2dx = 0;
            m_c2dy = -oneHalfY;
        }
        
        c1 = QPointF(p1.x() + m_c1dx,  // control point 1
                     p1.y() + m_c1dy);
        c2 = QPointF(p2.x() + m_c2dx,  // control point 2
                     p2.y() + m_c2dy);
    } else {
        //c1 = m_points[1];
        //c2 = m_points[2];
        
        //m_c1dx = c1.x() - p1.x();
        //m_c1dy = c1.y() - p1.y();
        //m_c2dx = c2.x() - p2.x();
        //m_c2dy = c2.y() - p2.y();
        
        c1 = QPointF(p1.x() + m_c1dx,  // control point 1
                     p1.y() + m_c1dy);
        c2 = QPointF(p2.x() + m_c2dx,  // control point 2
                     p2.y() + m_c2dy);
    }
    if (m_points.size() == 2) {  // create two points
        insertPoint(1, c1);
        insertPoint(2, c2);
    }
    if (m_points.size() == 4) {  // change bezier points
        setPoint(1, c1);
        setPoint(2, c2);
    }
    if (m_points.size() == 3) {  // create one point
        // insertPoint(1 or 2, );
        // Note: For now we use a quadratic Bezier curve in createBezierCurve(...).
    }
}

/**
 * Returns a Bézier path from given points.
 * @param points   points which define the Bézier curve
 * @return   cubic Bézier spline
 */
QPainterPath AssociationLine::createBezierCurve(QVector<QPointF> points)
{
    std::string autoLayout;

    QPainterPath path;
    if (points.size() > 3) {  // cubic Bezier curve(s)
        path.moveTo(points.at(0));
        int i = 1;
        while (i + 2 < points.size()) {
            path.cubicTo(points.at(i), points.at(i+1), points.at(i+2));
            i += 3;
        }
        while (i < points.size()) {  // draw a line if points are not modulo 3
            path.lineTo(points.at(i));
            ++i;
        }
    }
    else {
        if (points.size() == 3) {  // quadratic Bezier curve
            path.moveTo(points.at(0));
            path.quadTo(points.at(1), points.at(2));
        }
        else {  // should not be reached
            QPolygonF polygon(points);
            path.addPolygon(polygon);
        }
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
        qreal deltaX = fabs(start.x() - end.x());
        qreal deltaY = fabs(start.y() - end.y());
        // DEBUG() << "start=" << start << " / end=" << end
        //               << " / deltaX=" << deltaX << " / deltaY=" << deltaY;
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
    const QColor orig = _pen.color().lighter();
    QColor invertedColor(orig.green(), orig.blue(), orig.red());
    if (invertedColor == _pen.color()) {
        // Ensure different color.
        invertedColor.setRed((invertedColor.red() + 50) % 256);
    }
    invertedColor.setAlpha(150);

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
    painter->drawPath(path());

    if (option->state & QStyle::State_Selected) {
        // make the association broader in the selected state
        QPainterPathStroker stroker;
        stroker.setWidth(3.0);
        QPainterPath outline = stroker.createStroke(path());
        QColor shadowColor(Qt::lightGray);
        shadowColor.setAlpha(80);
        QBrush shadowBrush(shadowColor);
        painter->setBrush(shadowBrush);
        painter->setPen(Qt::NoPen);
        painter->drawPath(outline);

        // set color for selected painting
        _pen.setColor(Qt::blue);
        QRectF circle(0, 0, SelectedPointDiameter, SelectedPointDiameter);
        painter->setBrush(_pen.color());
        painter->setPen(Qt::NoPen);

        // draw points
        circle.moveCenter(savedStart);
        painter->drawRect(circle);
        for (int i = 1; i < sz-1; ++i) {
            if (i != m_activePointIndex) {
                circle.moveCenter(m_points.at(i));
                painter->drawRect(circle);
            }
        }
        
        //circle.moveCenter(savedStart);
        //painter->drawRect(circle);
        
        // draw bezier handles
        if (m_layout == Uml::LayoutType::Spline) {
            for (int i = 2; i < sz-1; ++i) {
                painter->setPen(QPen(invertedColor, _pen.widthF() + 1));
                //    if(m_layout == Uml::LayoutType::Spline) {
                QLineF mysegmentLine(savedStart, m_points[i-1]);
                painter->drawLine(mysegmentLine);
                
                QLineF mysegmentLine2(m_points[i], savedEnd);
                painter->drawLine(mysegmentLine2);
            }            
        }
        
        circle.moveCenter(savedEnd);
        painter->drawRect(circle);

        if (m_activePointIndex != -1) {
            painter->setBrush(invertedColor);
            painter->setPen(Qt::NoPen);
            circle.setWidth(1.5*SelectedPointDiameter);
            circle.setHeight(1.5*SelectedPointDiameter);
            circle.moveCenter(m_points.at(m_activePointIndex));
            painter->drawEllipse(circle);
        }
        else if (m_activeSegmentIndex != -1) {
            if (m_layout == Uml::LayoutType::Polyline) {
                painter->setPen(QPen(invertedColor, _pen.widthF() + 1));
                painter->setBrush(Qt::NoBrush);

                QLineF segmentLine(m_points[m_activeSegmentIndex], m_points[m_activeSegmentIndex + 1]);
                painter->drawLine(segmentLine);
            }
        }
        
        // debug info
        if (Tracer::instance()->isEnabled(QString::fromLatin1(metaObject()->className()))) {
            QPen p(Qt::green);
            p.setWidthF(1.0);
            painter->setPen(p);
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(shape());
            painter->setPen(Qt::blue);
            painter->drawRect(boundingRect());
            // origin
            painter->drawLine(-10, 0, 10, 0);
            painter->drawLine(0, -10, 0, 10);
        }
    }

    if (m_associationWidget->highLighted()) {
        QPainterPathStroker stroker;
        stroker.setWidth(3.0);
        QPainterPath outline = stroker.createStroke(path());
        QColor shadowColor(Qt::blue);
        shadowColor.setAlpha(80);
        QBrush shadowBrush(shadowColor);
        painter->setBrush(shadowBrush);
        painter->setPen(Qt::NoPen);
        painter->drawPath(outline);
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
    DEBUG() << "at " << event->scenePos();
    if (event->buttons() & Qt::LeftButton) {
        m_activePointIndex = closestPointIndex(event->scenePos());
        // calculate only if active point index is -1
        m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : closestSegmentIndex(event->scenePos());
    }
    else if (event->buttons() & Qt::RightButton) {
        DEBUG() << "call context menu of association widget at " << event->scenePos();
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
    UMLScene* scene = m_associationWidget->umlScene();

    QPointF unsnappedPos = event->scenePos();
    QPointF newPos(
        scene->snappedX(unsnappedPos.x()),
        scene->snappedY(unsnappedPos.y())
    );

    // Prevent the moving vertex from disappearing underneath a widget
    // (else there's no way to get it back.)
    UMLWidget *onW = scene->widgetAt(newPos);
    if (onW && !onW->isBoxWidget()) {  // boxes are transparent
        const qreal pX = newPos.x();
        const qreal pY = newPos.y();
        const qreal wX = onW->x();
        const qreal wY = onW->y();
        const qreal wWidth = onW->width();
        const qreal wHeight = onW->height();
        if (pX > wX && pX < wX + wWidth) {
            const qreal midX = wX + wWidth / 2.0;
            if (pX <= midX)
                newPos.setX(wX);
            else
                newPos.setX(wX + wWidth);
        }
        if (pY > wY && pY < wY + wHeight) {
            const qreal midY = wY + wHeight / 2.0;
            if (pY <= midY)
                newPos.setY(wY);
            else
                newPos.setY(wY + wHeight);
        }
    }

    if (m_activePointIndex != -1) {
        const int nPoints = m_points.size();
        // Move a single point (snap behaviour)
        if (m_activePointIndex == 1) {
            m_c1dx = newPos.x() - m_points.at(0).x();
            m_c1dy = newPos.y() - m_points.at(0).y();
        } else if (nPoints > 3 && m_activePointIndex == nPoints - 2) {
            m_c2dx = newPos.x() - m_points.at(nPoints-1).x();
            m_c2dy = newPos.y() - m_points.at(nPoints-1).y();
        }
        setPoint(m_activePointIndex, newPos);
        m_autoLayoutSpline = false;
    }
    else if (m_activeSegmentIndex != -1 && !isEndSegmentIndex(m_activeSegmentIndex)) {
        // Move a segment (between two points, snap behaviour not implemented)
        QPointF delta = event->scenePos() - event->lastScenePos();
        setPoint(m_activeSegmentIndex, m_points[m_activeSegmentIndex] + delta);
        setPoint(m_activeSegmentIndex + 1, m_points[m_activeSegmentIndex + 1] + delta);
    }
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
 * Calculates the "to be highlighted" point and segment indices
 * and updates if necessary.
 */
void AssociationLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    DEBUG() << "at " << event->scenePos();
    int oldPointIndex = m_activePointIndex;
    int oldSegmentIndex = m_activeSegmentIndex;

    m_activePointIndex = closestPointIndex(event->scenePos());
    // End points are not drawn and hence not active.
    if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
        m_activePointIndex = -1;
    }
    // Activate segment index only if point index is -1
    m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : closestSegmentIndex(event->scenePos());

    bool isChanged = (oldSegmentIndex != m_activeSegmentIndex || oldPointIndex != m_activePointIndex);
    if (isChanged) {
        m_associationWidget->update();
    }
}

/**
 * Calculates the "to be highlighted" point and segment indices
 * and updates if necessary.
 */
void AssociationLine::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    int oldPointIndex = m_activePointIndex;
    int oldSegmentIndex = m_activeSegmentIndex;

    m_activePointIndex = closestPointIndex(event->scenePos());
    // End points are not drawn and hence not active.
    if (m_activePointIndex != -1 && isEndPointIndex(m_activePointIndex)) {
        m_activePointIndex = -1;
    }
    // Activate segment index only if point index is -1
    m_activeSegmentIndex = (m_activePointIndex != -1) ? -1 : closestSegmentIndex(event->scenePos());

    bool isChanged = (oldSegmentIndex != m_activeSegmentIndex || oldPointIndex != m_activePointIndex);
    if (isChanged) {
        m_associationWidget->update();
    }
}

/**
 * Reset active indices and updates.
 */
void AssociationLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    DEBUG() << "at " << event->scenePos();
    //Q_UNUSED(event)
    m_activePointIndex   = -1;
    m_activeSegmentIndex = -1;
    m_associationWidget->update();
}

//-----------------------------------------------------------------------------

/**
 * Auxiliary struct for Symbol::SymbolProperty in C Plain Old Data format
 * avoids danger of "C++ static initialization order fiasco", see
 * https://en.cppreference.com/w/cpp/language/siof
 */
struct SymbolPropertyPOD {
    int boundRectX, boundRectY, boundRectW, boundRectH;
    int axisLineStartX, axisLineStartY, axisLineEndX, axisLineEndY;
    int endPoint0x, endPoint0y, endPoint1x, endPoint1y;
};

static const SymbolPropertyPOD symPropData[] =
{
    {
        -6,  0, 12, 10,
         0,  0,  0, 10,
         0, 10,  0, 10
    },
    {
        -6,  0, 12, 10,
         0,  0,  0, 10,
         0,  0,  0, 10
    },
    {
        -6,  0, 12, 10,
         0,  0,  0, 10,
         0, 10,  0, 10
    },
    {
        -5, -10, 10, 20,
         0, -10,  0, 10,
         0, -10,  0, 10
    },
    {
        -15, -10, 30, 20,
        -10,   0,  0,  0,
          0,   0,  0,  0
    },
    {
        -8, -8, 16, 16,
         0, -8,  0,  8,
         0, -8,  0,  8
    }
};

/**
 * SymbolEndPoints:
 * The first point is where the AssociationLine's visible line is
 * supposed to end.
 * The second points is where the actual symbol part is to appear.
 */
Symbol::SymbolProperty *Symbol::symbolTable;

/**
 * @internal A convenience method to setup shapes of all symbols.
 */
void Symbol::setupSymbolTable()
{
    if (symbolTable)
        return;
    symbolTable = new SymbolProperty[Count];

    // Copy C symPropData to C++ symbolTable
    for (int i = 0; i < Count; i++) {
        const SymbolPropertyPOD& c = symPropData[i];
        SymbolProperty& prop = symbolTable[i];
        prop.boundRect = QRectF(c.boundRectX, c.boundRectY, c.boundRectW, c.boundRectH);
        prop.axisLine  = QLineF(c.axisLineStartX, c.axisLineStartY, c.axisLineEndX, c.axisLineEndY);
        prop.endPoints = SymbolEndPoints(QPointF(c.endPoint0x, c.endPoint0y),
                                         QPointF(c.endPoint1x, c.endPoint1y));
    }

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

        // right leg
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
    painter->drawPath(symbolTable[m_symbolType].shape);
}

/**
 * @return The bound rectangle for this based on current symbol.
 */
QRectF Symbol::boundingRect() const
{
    const qreal adj = .5 * m_pen.widthF();
    return symbolTable[m_symbolType].boundRect.
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

    QLineF origAxis = symbolTable[m_symbolType].axisLine;
    QLineF translatedAxis = origAxis.translated(toMapped.p2() - origAxis.p2());

    qreal angle = translatedAxis.angleTo(toMapped);
    setRotation(rotation() - angle);

    QPointF delta = to.p2() - mapToParent(symbolEndPoints().second);
    moveBy(delta.x(), delta.y());
}

/**
 * @return The end points for the symbol.
 */
Symbol::SymbolEndPoints Symbol::symbolEndPoints() const
{
    return symbolTable[m_symbolType].endPoints;
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
    m_pen.setStyle(Qt::SolidLine);
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

