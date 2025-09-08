/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "widget_utils.h"

// app includes
#define DBG_SRC QStringLiteral("Widget_Utils")
#include "debug_utils.h"
#include "objectwidget.h"
#include "messagewidget.h"
#include "umlwidget.h"
#include "umlapp.h"  // only needed for log{Warn,Error}

#include <KLocalizedString>

// qt includes
#include <QBuffer>
#include <QImageReader>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QPolygonF>
#include <QXmlStreamWriter>

// c++ include
#include <cmath>

DEBUG_REGISTER(Widget_Utils)

namespace Widget_Utils
{

    /**
     * Find the widget identified by the given ID in the given widget
     * or message list.
     *
     * @param id         The unique ID to find.
     * @param widgets    The UMLWidgetList to search in.
     * @param messages   Optional pointer to a MessageWidgetList to search in.
     */
    UMLWidget* findWidget(Uml::ID::Type id,
                          const UMLWidgetList& widgets,
                          const MessageWidgetList *messages /* = nullptr */)
    {
        for(UMLWidget *obj : widgets) {
            if (obj->isObjectWidget()) {
                if (obj->localID() == id)
                    return obj;
            } else if (obj->id() == id) {
                return obj;
            }
        }

        if (messages) {
            for(UMLWidget *obj : *messages) {
                if (obj->id() == id)
                    return obj;
            }
        }
        return nullptr;
    }

    /**
     * Creates the decoration point.
     * @param p        base point to decorate
     * @param parent   parent item
     * @return         decoration point
     */
    QGraphicsRectItem* decoratePoint(const QPointF &p, QGraphicsItem* parent)
    {
        const qreal SIZE = 4.0;
        const qreal SIZE_HALF = SIZE / 2.0;
        QGraphicsRectItem *rect = new QGraphicsRectItem(p.x() - SIZE_HALF,
                                                        p.y() - SIZE_HALF,
                                                        SIZE, SIZE,
                                                        parent);
        rect->setBrush(QBrush(Qt::blue));
        rect->setPen(QPen(Qt::blue));
        return rect;
    }

    /**
     * Calculates and draws a cross inside an ellipse
     * @param p  Pointer to a QPainter object.
     * @param r  The rectangle describing the ellipse.
     */
    void drawCrossInEllipse(QPainter *p, const QRectF& r)
    {
        QRectF ellipse = r;
        ellipse.moveCenter(QPointF(0, 0));
        qreal a = ellipse.width() * 0.5;
        qreal b = ellipse.height() * .5;
        qreal xc = ellipse.center().x();
        qreal yc = ellipse.center().y();

        // The first point's x value is chosen to be center.x() + 70% of x radius.
        qreal x1 = ellipse.center().x() + .7 * .5 * ellipse.width();
        // Calculate y1 corresponding to x1 using formula.
        qreal y1_sqr = b*b*(1 - (x1 * x1) / (a*a));
        qreal y1 = std::sqrt(y1_sqr);

        // Mirror x1, y1 along both the axes to get 4 points for the cross.
        QPointF p1(xc + x1, yc + y1);
        QPointF p2(xc - x1, yc + y1);
        QPointF p3(xc + x1, yc - y1);
        QPointF p4(xc - x1, yc - y1);

        // Translate as we calculate for ellipse with (0, 0) as center.
        p->translate(r.center().x(), r.center().y());

        // Draw the cross now
        p->drawLine(QLineF(p1, p4));
        p->drawLine(QLineF(p2, p3));

        // Restore the translate on painter.
        p->translate(-r.center().x(), -r.center().y());
    }

    /**
     * Draws a polygon which is almost rectangular except for the top
     * right corner. A triangle is drawn in top right corner of the
     * rectangle.
     *
     * @param painter The painter with which this shape is to be drawn.
     * @param rect    The rectangle dimensions.
     * @param triSize The size of the triangle in the top-right corner.
     */
    void drawTriangledRect(QPainter *painter,
                           const QRectF &rect, const QSizeF &triSize)
    {
        // Draw outer boundary defined by polygon "poly".
        QPolygonF poly(5);
        poly[0] = rect.topLeft();
        poly[1] = rect.topRight() - QPointF(triSize.width(), 0);
        poly[2] = rect.topRight() + QPointF(0, triSize.height());
        poly[3] = rect.bottomRight();
        poly[4] = rect.bottomLeft();
        painter->drawPolygon(poly);

        // Now draw the triangle base and height edges.
        QLineF heightEdge(poly[1], poly[1] + QPointF(0, triSize.height()));
        painter->drawLine(heightEdge);
        QLineF baseEdge(heightEdge.p2(), poly[2]);
        painter->drawLine(baseEdge);
    }

//    /**
//     * Draws an arrow head with the given painter, with the arrow
//     * sharp point at \a headPos.
//     *
//     *  param painter    The painter with which this arrow should be drawn.
//     *  param headPos    The position where the head of the arrow should lie.
//     *  param arrowSize  This indicates the size of the arrow head.
//     *  param arrowType  This indicates direction of arrow as in LeftArrow, RightArrow..
//     *  param solid      If true, a solid head is drawn. Otherwise 2 lines are drawn.
//     */
//    void drawArrowHead(QPainter *painter, const QPointF &arrowPos,
//                       const QSizeF& arrowSize, Qt::ArrowType arrowType,
//                       bool  solid)
//    {
//        QPolygonF poly;
//        if (arrowType == Qt::LeftArrow) {
//            poly << QPointF(arrowPos.x() + arrowSize.width(), arrowPos.y() - .5 * arrowSize.height())
//                 << arrowPos
//                 << QPointF(arrowPos.x() + arrowSize.width(), arrowPos.y() + .5 * arrowSize.height());
//        }
//        else if (arrowType == Qt::RightArrow) {
//            poly << QPointF(arrowPos.x() - arrowSize.width(), arrowPos.y() - .5 * arrowSize.height())
//                 << arrowPos
//                 << QPointF(arrowPos.x() - arrowSize.width(), arrowPos.y() + .5 * arrowSize.height());
//        }

//        if (solid) {
//            painter->drawPolygon(poly);
//        }
//        else {
//            painter->drawPolyline(poly);
//        }
//    }

//    /**
//     * Draws a rounded rect rounded at specified corners.
//     *
//     *  param painter The painter with which this round rect should be drawn.
//     *  param rect    The rectangle to be drawn.
//     *  param xRadius The x radius of rounded corner.
//     *  param yRadius The y radius of rounded corner.
//     *  param corners The corners to be rounded.
//     */
//    void drawRoundedRect(QPainter *painter, const QRectF& rect, qreal xRadius,
//            qreal yRadius, Uml::Corners corners)
//    {
//        if (xRadius < 0 || yRadius < 0) {
//            painter->drawRect(rect);
//            return;
//        }
//        QRectF arcRect(0, 0, 2 * xRadius, 2 * yRadius);

//        QPainterPath path;
//        path.moveTo(rect.left(), rect.top() + yRadius);
//        if (corners.testFlag(Uml::Corner::TopLeft)) {
//            arcRect.moveTopLeft(rect.topLeft());
//            path.arcTo(arcRect, 180, -90);
//        } else {
//            path.lineTo(rect.topLeft());
//        }

//        path.lineTo(rect.right() - xRadius, rect.top());

//        if (corners.testFlag(Uml::Corner::TopRight)) {
//            arcRect.moveTopRight(rect.topRight());
//            path.arcTo(arcRect, 90, -90);
//        } else {
//            path.lineTo(rect.topRight());
//        }

//        path.lineTo(rect.right(), rect.bottom() - yRadius);

//        if (corners.testFlag(Uml::Corner::BottomRight)) {
//            arcRect.moveBottomRight(rect.bottomRight());
//            path.arcTo(arcRect, 0, -90);
//        } else {
//            path.lineTo(rect.bottomRight());
//        }

//        path.lineTo(rect.left() + xRadius, rect.bottom());

//        if (corners.testFlag(Uml::Corner::BottomLeft)) {
//            arcRect.moveBottomLeft(rect.bottomLeft());
//            path.arcTo(arcRect, 270, 90);
//        } else {
//            path.lineTo(rect.bottomLeft());
//        }

//        path.closeSubpath();
//        painter->drawPath(path);
//    }

    /**
     * Converts a point to a comma separated string i.e "x,y"
     * @param point  The QPointF to convert.
     */
    QString pointToString(const QPointF& point)
    {
        return QString::fromLatin1("%1,%2").arg(point.x()).arg(point.y());
    }

    /**
     * Converts a comma separated string to point.
     */
    QPointF stringToPoint(const QString& str)
    {
        QPointF retVal;
        QStringList list = str.split(QLatin1Char(','));

        if(list.size() == 2) {
            retVal.setX(list.first().toDouble());
            retVal.setY(list.last().toDouble());
        }
        return retVal;
    }

    /**
     * Loads pixmap from xmi.
     *
     * @param pixEle  The dom element from which pixmap should be loaded.
     *
     * @param pixmap  The pixmap into which the image should be loaded.
     *
     * @return  True or false based on success or failure of this method.
     */
    bool loadPixmapFromXMI(QDomElement &pixEle, QPixmap &pixmap)
    {
        if (pixEle.isNull()) {
            return false;
        }
        QDomElement xpmElement = pixEle.firstChildElement(QStringLiteral("xpm"));

        QByteArray xpmData = xpmElement.text().toLatin1();
        QBuffer buffer(&xpmData);
        buffer.open(QIODevice::ReadOnly);

        QImageReader reader(&buffer, "xpm");
        QImage image;
        if (!reader.read(&image)) {
            return false;
        }

        pixmap = QPixmap::fromImage(image);
        return true;
    }

    /**
     * Saves pixmap information to XMI.
     *
     * @param stream The QXmlStreamWriter object to write to.
     * @param pixmap The pixmap to be saved.
     */
    void savePixmapToXMI(QXmlStreamWriter& stream, const QPixmap& pixmap)
    {
        stream.writeStartElement(QStringLiteral("pixmap"));

        stream.writeStartElement(QStringLiteral("xpm"));
        stream.writeEndElement();

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "xpm");
        buffer.close();

        stream.writeTextElement(QString(), QString::fromLatin1(buffer.data()));

        stream.writeEndElement();
    }

    /**
     * Loads gradient from xmi. The gradient pointer should be null
     * and the new gradient object will be created inside this method.
     * The gradient should later be deleted externally.
     *
     * @param gradientElement The DOM element from which gradient should be
     *                        loaded.
     *
     * @param gradient The pointer to gradient into which the gradient
     *                 should be loaded. (Allocated inside this
     *                 method)
     *
     * @return True or false based on success or failure of this method.
     */
    bool loadGradientFromXMI(QDomElement &gradientElement, QGradient *&gradient)
    {
        if(gradientElement.isNull()) {
            return false;
        }

        int type_as_int;
        QGradient::Type type;
        QGradientStops stops;
        QGradient::CoordinateMode cmode = QGradient::LogicalMode;
        QGradient::Spread spread = QGradient::PadSpread;

        type_as_int = gradientElement.attribute(QStringLiteral("type")).toInt();
        type = QGradient::Type(type_as_int);
        type_as_int = gradientElement.attribute(QStringLiteral("spread")).toInt();
        spread = QGradient::Spread(type_as_int);
        type_as_int = gradientElement.attribute(QStringLiteral("coordinatemode")).toInt();
        cmode = QGradient::CoordinateMode(type_as_int);

        QDomElement stopElement = gradientElement.firstChildElement(QStringLiteral("stops"));
        if(stopElement.isNull()) {
            return false;
        }
        for(QDomNode node = stopElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement ele = node.toElement();
            if(ele.tagName() != QStringLiteral("stop")) {
                continue;
            }

            qreal posn = ele.attribute(QStringLiteral("position")).toDouble();
            QColor color = QColor(ele.attribute(QStringLiteral("color")));
            stops << QGradientStop(posn, color);
        }

        if (type == QGradient::LinearGradient) {
            QPointF p1 = stringToPoint(gradientElement.attribute(QStringLiteral("start")));
            QPointF p2 = stringToPoint(gradientElement.attribute(QStringLiteral("finalstop")));
            gradient = new QLinearGradient(p1, p2);
        }
        else if (type == QGradient::RadialGradient) {
            QPointF center = stringToPoint(gradientElement.attribute(QStringLiteral("center")));
            QPointF focal = stringToPoint(gradientElement.attribute(QStringLiteral("focalpoint")));
            double radius = gradientElement.attribute(QStringLiteral("radius")).toDouble();
            gradient = new QRadialGradient(center, radius, focal);
        }
        else { // type == QGradient::ConicalGradient
            QPointF center = stringToPoint(gradientElement.attribute(QStringLiteral("center")));
            double angle = gradientElement.attribute(QStringLiteral("angle")).toDouble();
            gradient = new QConicalGradient(center, angle);
        }

        if(gradient) {
            gradient->setStops(stops);
            gradient->setSpread(spread);
            gradient->setCoordinateMode(cmode);
            return true;
        }

        return false;
    }

    /**
     * Saves gradient information to XMI.
     *
     * @param stream The QXmlStreamWriter object to write to.
     * @param gradient The gradient to be saved.
     */
    void saveGradientToXMI(QXmlStreamWriter& stream, const QGradient *gradient)
    {
        stream.writeStartElement(QStringLiteral("gradient"));

        stream.writeAttribute(QStringLiteral("type"), QString::number(gradient->type()));
        stream.writeAttribute(QStringLiteral("spread"), QString::number(gradient->spread()));
        stream.writeAttribute(QStringLiteral("coordinatemode"), QString::number(gradient->coordinateMode()));

        QGradient::Type type = gradient->type();

        if(type == QGradient::LinearGradient) {
            const QLinearGradient *lg = static_cast<const QLinearGradient*>(gradient);
            stream.writeAttribute(QStringLiteral("start"), pointToString(lg->start()));
            stream.writeAttribute(QStringLiteral("finalstop"), pointToString(lg->finalStop()));
        }
        else if(type == QGradient::RadialGradient) {
            const QRadialGradient *rg = static_cast<const QRadialGradient*>(gradient);
            stream.writeAttribute(QStringLiteral("center"), pointToString(rg->center()));
            stream.writeAttribute(QStringLiteral("focalpoint"), pointToString(rg->focalPoint()));
            stream.writeAttribute(QStringLiteral("radius"), QString::number(rg->radius()));
        }
        else { //type == QGradient::ConicalGradient
            const QConicalGradient *cg = static_cast<const QConicalGradient*>(gradient);
            stream.writeAttribute(QStringLiteral("center"), pointToString(cg->center()));
            stream.writeAttribute(QStringLiteral("angle"), QString::number(cg->angle()));
        }

        stream.writeStartElement(QStringLiteral("stops"));

        for(const QGradientStop& stop : gradient->stops()) {
            stream.writeStartElement(QStringLiteral("stop"));
            stream.writeAttribute(QStringLiteral("position"), QString::number(stop.first));
            stream.writeAttribute(QStringLiteral("color"), stop.second.name());
            stream.writeEndElement();
        }

        stream.writeEndElement();            // stops
        stream.writeEndElement();   // gradient
    }

    /**
     * Extracts the QBrush properties into brush from the XMI xml
     * element qElement.
     *
     * @param qElement The DOM element from which the xmi info should
     *                 be extracted.
     *
     * @param brush The QBrush object into which brush details should
     *              be read into.
     */
    bool loadBrushFromXMI(QDomElement &qElement, QBrush &brush)
    {
        if(qElement.isNull()) {
            return false;
        }

        quint8 style = qElement.attribute(QStringLiteral("style")).toShort();
        const QString colorString = qElement.attribute(QStringLiteral("color"));
        QColor color;
        color.setNamedColor(colorString);

        if(style == Qt::TexturePattern) {
            QPixmap pixmap;
            QDomElement pixElement = qElement.firstChildElement(QStringLiteral("pixmap"));
            if(!loadPixmapFromXMI(pixElement, pixmap)) {
                return false;
            }
            brush = QBrush(color, pixmap);
        }

        else if(style == Qt::LinearGradientPattern
                || style == Qt::RadialGradientPattern
                || style == Qt::ConicalGradientPattern) {
            QGradient  *gradient = nullptr;
            QDomElement gradElement = qElement.firstChildElement(QStringLiteral("gradient"));

            if(!loadGradientFromXMI(gradElement, gradient) || !gradient) {
                delete gradient;
                return false;
            }

            brush = QBrush(*gradient);
            delete gradient;
        }

        else {
            brush = QBrush(color, (Qt::BrushStyle)style);
        }

        //TODO: Checks if transform needs to be loaded.

        return true;
    }

    /**
     * Saves the brush info as XMI.
     *
     * @param stream The QXmlStreamWriter object to write to.
     * @param brush The QBrush whose details should be saved.
     */
    void saveBrushToXMI(QXmlStreamWriter& stream, const QBrush& brush)
    {
        stream.writeStartElement(QStringLiteral("brush"));

        stream.writeAttribute(QStringLiteral("style"), QString::number(brush.style()));
        stream.writeAttribute(QStringLiteral("color"), brush.color().name());

        if(brush.style() == Qt::TexturePattern) {
            savePixmapToXMI(stream, brush.texture());
        }
        else if(brush.style() == Qt::LinearGradientPattern
                || brush.style() == Qt::RadialGradientPattern
                || brush.style() == Qt::ConicalGradientPattern) {
            saveGradientToXMI(stream, brush.gradient());
        }

        //TODO: Check if transform of this brush needs to be saved.
        stream.writeEndElement();
    }

    /**
     * Returns true if the first widget's X is smaller than second's.
     * Used for sorting the UMLWidgetList.
     * @param widget1 The widget to compare.
     * @param widget2 The widget to compare with.
     */
    bool hasSmallerX(const UMLWidget* widget1, const UMLWidget* widget2)
    {
        return widget1->x() < widget2->x();
    }

    /**
     * Returns true if the first widget's Y is smaller than second's.
     * Used for sorting the UMLWidgetList.
     * @param widget1 The widget to compare.
     * @param widget2 The widget to compare with.
     */
    bool hasSmallerY(const UMLWidget* widget1, const UMLWidget* widget2)
    {
        return widget1->y() < widget2->y();
    }

    /**
     * Find the region in which the rectangle \a other lies with respect to
     * the rectangle \a self.
     * Beware that the Qt coordinate system has its origin point (0,0) in
     * the upper left corner with Y values growing downwards, thus the Y
     * related comparisons might look inverted if your are used to the
     * natural coordinate system with (0,0) in the lower left corner.
     */
    Uml::Region::Enum findRegion(const QRectF& self, const QRectF &other)
    {
        const qreal ownX      = self.x();
        const qreal ownY      = self.y();
        const qreal ownWidth  = self.width();
        const qreal ownHeight = self.height();
        const qreal otherX      = other.x();
        const qreal otherY      = other.y();
        const qreal otherWidth  = other.width();
        const qreal otherHeight = other.height();
        Uml::Region::Enum region = Uml::Region::Center;
        if (otherX + otherWidth < ownX) {
            if (otherY + otherHeight < ownY)
                region = Uml::Region::NorthWest;
            else if (otherY > ownY + ownHeight)
                region = Uml::Region::SouthWest;
            else
                region = Uml::Region::West;
        } else if (otherX > ownX + ownWidth) {
            if (otherY + otherHeight < ownY)
                region = Uml::Region::NorthEast;
            else if (otherY > ownY + ownHeight)
                region = Uml::Region::SouthEast;
            else
                region = Uml::Region::East;
        } else {
            if (otherY + otherHeight < ownY)
                region = Uml::Region::North;
            else if (otherY > ownY + ownHeight)
                region = Uml::Region::South;
            else
                region = Uml::Region::Center;
        }
        return region;
    }

    /**
     * Return the point in \a poly which precedes the point at index \a index.
     * If \a index is 0 then return the last (or, if \a poly.isClosed() is
     * true, the second to last) point.
     */
    QPointF prevPoint(int index, const QPolygonF& poly) {
        if (poly.size() < 3 || index >= poly.size())
            return QPoint();
        if (index == 0)
            return poly.at(poly.size() - 1 - (int)poly.isClosed());
        return poly.at(index - 1);
    }

    /**
     * Return the point in \a poly which follows the point at index \a index.
     * If \a index is the last index then return the first (or, if
     * \a poly.isClosed() is true, the second) point.
     */
    QPointF nextPoint(int index, const QPolygonF& poly) {
        if (poly.size() < 3 || index >= poly.size())
            return QPoint();
        if (index == poly.size() - 1)
            return poly.at((int)poly.isClosed());
        return poly.at(index + 1);
    }

    /**
     * Return the middle value between \a a and \a b.
     */
    qreal middle(qreal a, qreal b)
    {
        return (a + b) / 2.0;
    }

    /**
     * Auxiliary type for function findLine()
     */
    enum Axis_Type { X , Y };

    /**
     * Auxiliary type for function findLine()
     */
    enum Comparison_Type { Smallest, Largest };

    /**
     * Find the line of \a poly with the smallest or largest value (controlled by \a seek)
     * along the axis controlled by \a axis.
     * In case \a axis is X, do not consider lines whose Y values lie outside the Y values
     * defined by \a boundingRect.
     * In case \a axis is Y, do not consider lines whose X values lie outside the X values
     * defined by \a boundingRect.
     */
    QLineF findLine(const QPolygonF& poly, Axis_Type axis, Comparison_Type seek, const QRectF& boundingRect)
    {
        const int lastIndex = poly.size() - 1 - (int)poly.isClosed();
        QPointF prev = poly.at(lastIndex), curr;
        QPointF p1(seek == Smallest ? QPointF(1.0e6, 1.0e6) : QPointF(-1.0e6, -1.0e6));
        QPointF p2;
        for (int i = 0; i <= lastIndex; i++) {
            curr = poly.at(i);
            // uDebug() << "  poly[" << i << "] = " << curr;
            if (axis == X) {
                if (fmin(prev.y(), curr.y()) > boundingRect.y() + boundingRect.height() ||
                    fmax(prev.y(), curr.y()) < boundingRect.y()) {
                    // line is outside Y-axis range defined by boundingRect
                } else if ((seek == Smallest && curr.x() <= p1.x()) ||
                           (seek == Largest  && curr.x() >= p1.x())) {
                    p1 = curr;
                    p2 = prev;
                }
            } else {
                if (fmin(prev.x(), curr.x()) > boundingRect.x() + boundingRect.width() ||
                    fmax(prev.x(), curr.x()) < boundingRect.x()) {
                    // line is outside X-axis range defined by boundingRect
                } else if ((seek == Smallest && curr.y() <= p1.y()) ||
                           (seek == Largest  && curr.y() >= p1.y())) {
                    p1 = curr;
                    p2 = prev;
                }
            }
            prev = curr;
        }
        return QLineF(p1, p2);
    }

    /**
     * Determine the approximate closest points of two polygons.
     * @param self  First QPolygonF.
     * @param other Second QPolygonF.
     * @return  QLineF::p1() returns point of \a self;
     *          QLineF::p2() returns point of \a other.
     */
    QLineF closestPoints(const QPolygonF& self, const QPolygonF& other)
    {
        const QRectF& selfRect = self.boundingRect();
        const QRectF& otherRect = other.boundingRect();
        Uml::Region::Enum region = findRegion(selfRect, otherRect);
        if (region == Uml::Region::Center)
            return QLineF();
        if (self.size() < 3 || other.size() < 3)
            return QLineF();
        QLineF result;
        const int selfLastIndex  = self.size()  - 1 - (int)self.isClosed();
        const int otherLastIndex = other.size() - 1 - (int)other.isClosed();
        QPointF selfPoint(self.at(selfLastIndex));
        QPointF otherPoint(other.at(otherLastIndex));
        QLineF selfLine, otherLine;
        int i;

        switch (region) {

        case Uml::Region::North:
            // Find other's line with largest Y values
            otherLine = findLine(other, Y, Largest, selfRect);
            // Find own line with smallest Y values
            selfLine = findLine(self, Y, Smallest, otherRect);
            // Use the middle value of the X values
            result.setLine(middle(selfLine.p2().x(), selfLine.p1().x()), selfLine.p1().y(),
                           middle(otherLine.p2().x(), otherLine.p1().x()), otherLine.p1().y());
            break;

        case Uml::Region::South:
            // Find other's line with smallest Y values
            otherLine = findLine(other, Y, Smallest, selfRect);
            // Find own line with largest Y values
            selfLine = findLine(self, Y, Largest, otherRect);
            // Use the middle value of the X values
            result.setLine(middle(selfLine.p2().x(), selfLine.p1().x()), selfLine.p1().y(),
                           middle(otherLine.p2().x(), otherLine.p1().x()), otherLine.p1().y());
            break;

        case Uml::Region::West:
            // Find other's line with largest X values
            otherLine = findLine(other, X, Largest, selfRect);
            // Find own line with smallest X values
            selfLine = findLine(self, X, Smallest, otherRect);
            // Use the middle value of the Y values
            result.setLine(selfLine.p1().x(), middle(selfLine.p2().y(), selfLine.p1().y()),
                           otherLine.p1().x(), middle(otherLine.p2().y(), otherLine.p1().y()));
            break;

        case Uml::Region::East:
            // Find other's line with smallest X values
            otherLine = findLine(other, X, Smallest, selfRect);
            // Find own line with largest X values
            selfLine = findLine(self, X, Largest, otherRect);
            // Use the middle value of the Y values
            result.setLine(selfLine.p1().x(), middle(selfLine.p2().y(), selfLine.p1().y()),
                           otherLine.p1().x(), middle(otherLine.p2().y(), otherLine.p1().y()));
            break;

        case Uml::Region::NorthWest:
            // Find other's point with largest X and largest Y value
            for (i = 0; i < otherLastIndex; ++i) {
                QPointF current(other.at(i));
                if (current.x() + current.y() >= otherPoint.x() + otherPoint.y()) {
                    otherPoint = current;
                }
            }
            // Find own point with smallest X and smallest Y value
            for (i = 0; i < selfLastIndex; ++i) {
                QPointF current(self.at(i));
                if (current.x() + current.y() <= selfPoint.x() + selfPoint.y()) {
                    selfPoint = current;
                }
            }
            result.setPoints(selfPoint, otherPoint);
            break;

        case Uml::Region::SouthWest:
            // Find other's point with largest X and smallest Y value
            for (i = 0; i < otherLastIndex; ++i) {
                QPointF current(other.at(i));
                if (current.x() >= otherPoint.x() && current.y() <= otherPoint.y()) {
                    otherPoint = current;
                }
            }
            // Find own point with smallest X and largest Y value
            for (i = 0; i < selfLastIndex; ++i) {
                QPointF current(self.at(i));
                if (current.x() <= selfPoint.x() && current.y() >= selfPoint.y()) {
                    selfPoint = current;
                }
            }
            result.setPoints(selfPoint, otherPoint);
            break;

        case Uml::Region::NorthEast:
            // Find other's point with smallest X and largest Y value
            for (i = 0; i < otherLastIndex; ++i) {
                QPointF current(other.at(i));
                if (current.x() <= otherPoint.x() && current.y() >= otherPoint.y()) {
                    otherPoint = current;
                }
            }
            // Find own point with largest X and smallest Y value
            for (i = 0; i < selfLastIndex; ++i) {
                QPointF current(self.at(i));
                if (current.x() >= selfPoint.x() && current.y() <= selfPoint.y()) {
                    selfPoint = current;
                }
            }
            result.setPoints(selfPoint, otherPoint);
            break;

        case Uml::Region::SouthEast:
            // Find other's point with smallest X and smallest Y value
            for (i = 0; i < otherLastIndex; ++i) {
                QPointF current(other.at(i));
                if (current.x() + current.y() <= otherPoint.x() + otherPoint.y()) {
                    otherPoint = current;
                }
            }
            // Find own point with largest X and largest Y value
            for (i = 0; i < selfLastIndex; ++i) {
                QPointF current(self.at(i));
                if (current.x() + current.y() >= selfPoint.x() + selfPoint.y()) {
                    selfPoint = current;
                }
            }
            result.setPoints(selfPoint, otherPoint);
            break;

        default:
            // Error
            break;
        }

        return result;
    }

    /**
     * Returns a default name for the new widget
     * @param type the widget type
     * @return the default name
     */
    QString defaultWidgetName(WidgetBase::WidgetType type)
    {
        switch(type) {
        case WidgetBase::wt_Activity:         return i18n("new activity");
        case WidgetBase::wt_Actor:            return i18n("new actor");
        case WidgetBase::wt_Artifact:         return i18n("new artifact");
        case WidgetBase::wt_Association:      return i18n("new association");
        case WidgetBase::wt_Box:              return i18n("new box");
        case WidgetBase::wt_Category:         return i18n("new category");
        case WidgetBase::wt_Class:            return i18n("new class");
        case WidgetBase::wt_CombinedFragment: return i18n("new combined fragment");
        case WidgetBase::wt_Component:        return i18n("new component");
        case WidgetBase::wt_Datatype:         return i18n("new datatype");
        case WidgetBase::wt_Entity:           return i18n("new entity");
        case WidgetBase::wt_Enum:             return i18n("new enum");
        case WidgetBase::wt_FloatingDashLine: return i18n("new floating dash line");
        case WidgetBase::wt_ForkJoin:         return i18n("new fork/join");
        case WidgetBase::wt_Instance:         return i18n("new instance");
        case WidgetBase::wt_Interface:        return i18n("new interface");
        case WidgetBase::wt_Message:          return i18n("new message");
        case WidgetBase::wt_Node:             return i18n("new node");
        case WidgetBase::wt_Note:             return i18n("new note");
        case WidgetBase::wt_Object:           return i18n("new object");
        case WidgetBase::wt_ObjectNode:       return i18n("new object node");
        case WidgetBase::wt_Package:          return i18n("new package");
        case WidgetBase::wt_Pin:              return i18n("new pin");
        case WidgetBase::wt_Port:             return i18n("new port");
        case WidgetBase::wt_Precondition:     return i18n("new precondition");
        case WidgetBase::wt_Region:           return i18n("new region");
        case WidgetBase::wt_Signal:           return i18n("new signal");
        case WidgetBase::wt_State:            return i18n("new state");
        case WidgetBase::wt_Text:             return i18n("new text");
        case WidgetBase::wt_UMLWidget:        return i18n("new UML widget");
        case WidgetBase::wt_UseCase:          return i18n("new use case");
        default:
            logWarn1("Widget_Utils::defaultWidgetName unknown widget type: %1",
                     WidgetBase::toString(type));
            return i18n("new widget");
            break;
        }
    }

    /**
     * Returns translated title string used by widget related dialogs
     * @param type widget type
     * @return translated title string
     */
    QString newTitle(WidgetBase::WidgetType type)
    {
        switch(type) {
        case WidgetBase::wt_Activity:         return i18n("New activity");
        case WidgetBase::wt_Actor:            return i18n("New actor");
        case WidgetBase::wt_Artifact:         return i18n("New artifact");
        case WidgetBase::wt_Association:      return i18n("New association");
        case WidgetBase::wt_Box:              return i18n("New box");
        case WidgetBase::wt_Category:         return i18n("New category");
        case WidgetBase::wt_Class:            return i18n("New class");
        case WidgetBase::wt_CombinedFragment: return i18n("New combined fragment");
        case WidgetBase::wt_Component:        return i18n("New component");
        case WidgetBase::wt_Datatype:         return i18n("New datatype");
        case WidgetBase::wt_Entity:           return i18n("New entity");
        case WidgetBase::wt_Enum:             return i18n("New enum");
        case WidgetBase::wt_FloatingDashLine: return i18n("New floating dash line");
        case WidgetBase::wt_ForkJoin:         return i18n("New fork/join");
        case WidgetBase::wt_Instance:         return i18n("New instance");
        case WidgetBase::wt_Interface:        return i18n("New interface");
        case WidgetBase::wt_Message:          return i18n("New message");
        case WidgetBase::wt_Node:             return i18n("New node");
        case WidgetBase::wt_Note:             return i18n("New note");
        case WidgetBase::wt_Object:           return i18n("New object");
        case WidgetBase::wt_ObjectNode:       return i18n("New object node");
        case WidgetBase::wt_Package:          return i18n("New package");
        case WidgetBase::wt_Pin:              return i18n("New pin");
        case WidgetBase::wt_Port:             return i18n("New port");
        case WidgetBase::wt_Precondition:     return i18n("New precondition");
        case WidgetBase::wt_Region:           return i18n("New region");
        case WidgetBase::wt_Signal:           return i18n("New signal");
        case WidgetBase::wt_State:            return i18n("New state");
        case WidgetBase::wt_Text:             return i18n("New text");
        case WidgetBase::wt_UMLWidget:        return i18n("New UML widget");
        case WidgetBase::wt_UseCase:          return i18n("New use case");
        default:
            logWarn1("Widget_Utils::newTitle unknown widget type: %1",
                     WidgetBase::toString(type));
            return i18n("New widget");
        }
    }

    /**
     * Returns translated text string used by widget related dialogs
     * @param type widget type
     * @return translated text string
     */
    QString newText(WidgetBase::WidgetType type)
    {
        switch(type) {
        case WidgetBase::wt_Activity:         return i18n("Enter the name of the new activity:");
        case WidgetBase::wt_Actor:            return i18n("Enter the name of the new actor:");
        case WidgetBase::wt_Artifact:         return i18n("Enter the name of the new artifact:");
        case WidgetBase::wt_Association:      return i18n("Enter the name of the new association:");
        case WidgetBase::wt_Box:              return i18n("Enter the name of the new box:");
        case WidgetBase::wt_Category:         return i18n("Enter the name of the new category:");
        case WidgetBase::wt_Class:            return i18n("Enter the name of the new class:");
        case WidgetBase::wt_CombinedFragment: return i18n("Enter the name of the new combined fragment:");
        case WidgetBase::wt_Component:        return i18n("Enter the name of the new component:");
        case WidgetBase::wt_Datatype:         return i18n("Enter the name of the new datatype:");
        case WidgetBase::wt_Entity:           return i18n("Enter the name of the new entity:");
        case WidgetBase::wt_Enum:             return i18n("Enter the name of the new enum:");
        case WidgetBase::wt_FloatingDashLine: return i18n("Enter the name of the new floating dash line:");
        case WidgetBase::wt_ForkJoin:         return i18n("Enter the name of the new fork/join:");
        case WidgetBase::wt_Instance:         return i18n("Enter the name of the new instance:");
        case WidgetBase::wt_Interface:        return i18n("Enter the name of the new interface:");
        case WidgetBase::wt_Message:          return i18n("Enter the name of the new message:");
        case WidgetBase::wt_Node:             return i18n("Enter the name of the new node:");
        case WidgetBase::wt_Note:             return i18n("Enter the name of the new note:");
        case WidgetBase::wt_Object:           return i18n("Enter the name of the new object:");
        case WidgetBase::wt_ObjectNode:       return i18n("Enter the name of the new object node:");
        case WidgetBase::wt_Package:          return i18n("Enter the name of the new package:");
        case WidgetBase::wt_Pin:              return i18n("Enter the name of the new pin:");
        case WidgetBase::wt_Port:             return i18n("Enter the name of the new port:");
        case WidgetBase::wt_Precondition:     return i18n("Enter the name of the new precondition:");
        case WidgetBase::wt_Region:           return i18n("Enter the name of the new region:");
        case WidgetBase::wt_Signal:           return i18n("Enter the name of the new signal:");
        case WidgetBase::wt_State:            return i18n("Enter the name of the new state:");
        case WidgetBase::wt_Text:             return i18n("Enter the name of the new text:");
        case WidgetBase::wt_UMLWidget:        return i18n("Enter the name of the new uml widget:");
        case WidgetBase::wt_UseCase:          return i18n("Enter the name of the new use case:");
        default:
            logWarn1("Widget_Utils::newText unknown widget type: %1", WidgetBase::toString(type));
            return i18n("Enter the name of the new widget:");
        }
    }

    /**
     * Returns translated title string used by widget related dialogs
     * @param type widget type
     * @return translated title string
     */
    QString renameTitle(WidgetBase::WidgetType type)
    {
        switch(type) {
        case WidgetBase::wt_Activity:         return i18n("Rename activity");
        case WidgetBase::wt_Actor:            return i18n("Rename actor");
        case WidgetBase::wt_Artifact:         return i18n("Rename artifact");
        case WidgetBase::wt_Association:      return i18n("Rename association");
        case WidgetBase::wt_Box:              return i18n("Rename box");
        case WidgetBase::wt_Category:         return i18n("Rename category");
        case WidgetBase::wt_Class:            return i18n("Rename class");
        case WidgetBase::wt_CombinedFragment: return i18n("Rename combined fragment");
        case WidgetBase::wt_Component:        return i18n("Rename component");
        case WidgetBase::wt_Datatype:         return i18n("Rename datatype");
        case WidgetBase::wt_Entity:           return i18n("Rename entity");
        case WidgetBase::wt_Enum:             return i18n("Rename enum");
        case WidgetBase::wt_FloatingDashLine: return i18n("Rename floating dash line");
        case WidgetBase::wt_ForkJoin:         return i18n("Rename fork/join");
        case WidgetBase::wt_Instance:         return i18n("Rename instance");
        case WidgetBase::wt_Interface:        return i18n("Rename interface");
        case WidgetBase::wt_Message:          return i18n("Rename message");
        case WidgetBase::wt_Node:             return i18n("Rename node");
        case WidgetBase::wt_Note:             return i18n("Rename note");
        case WidgetBase::wt_Object:           return i18n("Rename object");
        case WidgetBase::wt_ObjectNode:       return i18n("Rename object node");
        case WidgetBase::wt_Package:          return i18n("Rename package");
        case WidgetBase::wt_Pin:              return i18n("Rename pin");
        case WidgetBase::wt_Port:             return i18n("Rename port");
        case WidgetBase::wt_Precondition:     return i18n("Rename precondition");
        case WidgetBase::wt_Region:           return i18n("Rename region");
        case WidgetBase::wt_Signal:           return i18n("Rename signal");
        case WidgetBase::wt_State:            return i18n("Rename state");
        case WidgetBase::wt_Text:             return i18n("Rename text");
        case WidgetBase::wt_UMLWidget:        return i18n("Rename UML widget");
        case WidgetBase::wt_UseCase:          return i18n("Rename use case");
        default:
            logWarn1("Widget_Utils::renameTitle unknown widget type: %1",
                     WidgetBase::toString(type));
            return i18n("Rename widget");
        }
    }

    /**
     * Returns translated text string used by widget related dialogs
     * @param type widget type
     * @return translated text string
     */
    QString renameText(WidgetBase::WidgetType type)
    {
        switch(type) {
        case WidgetBase::wt_Activity:         return i18n("Enter the new name of the activity:");
        case WidgetBase::wt_Actor:            return i18n("Enter the new name of the actor:");
        case WidgetBase::wt_Artifact:         return i18n("Enter the new name of the artifact:");
        case WidgetBase::wt_Association:      return i18n("Enter the new name of the association:");
        case WidgetBase::wt_Box:              return i18n("Enter the new name of the box:");
        case WidgetBase::wt_Category:         return i18n("Enter the new name of the category:");
        case WidgetBase::wt_Class:            return i18n("Enter the new name of the class:");
        case WidgetBase::wt_CombinedFragment: return i18n("Enter the new name of the combined fragment:");
        case WidgetBase::wt_Component:        return i18n("Enter the new name of the component:");
        case WidgetBase::wt_Datatype:         return i18n("Enter the new name of the datatype:");
        case WidgetBase::wt_Entity:           return i18n("Enter the new name of the entity:");
        case WidgetBase::wt_Enum:             return i18n("Enter the new name of the enum:");
        case WidgetBase::wt_FloatingDashLine: return i18n("Enter the new name of the floating dash line:");
        case WidgetBase::wt_ForkJoin:         return i18n("Enter the new name of the fork/join widget:");
        case WidgetBase::wt_Instance:         return i18n("Enter the new name of the instance:");
        case WidgetBase::wt_Interface:        return i18n("Enter the new name of the interface:");
        case WidgetBase::wt_Message:          return i18n("Enter the new name of the message:");
        case WidgetBase::wt_Node:             return i18n("Enter the new name of the node:");
        case WidgetBase::wt_Note:             return i18n("Enter the new name of the note:");
        case WidgetBase::wt_Object:           return i18n("Enter the new name of the object:");
        case WidgetBase::wt_ObjectNode:       return i18n("Enter the new name of the object node:");
        case WidgetBase::wt_Package:          return i18n("Enter the new name of the package:");
        case WidgetBase::wt_Pin:              return i18n("Enter the new name of the pin:");
        case WidgetBase::wt_Port:             return i18n("Enter the new name of the port:");
        case WidgetBase::wt_Precondition:     return i18n("Enter the new name of the precondition:");
        case WidgetBase::wt_Region:           return i18n("Enter the new name of the region:");
        case WidgetBase::wt_Signal:           return i18n("Enter the new name of the signal:");
        case WidgetBase::wt_State:            return i18n("Enter the new name of the state:");
        case WidgetBase::wt_Text:             return i18n("Enter the new name of the text:");
        case WidgetBase::wt_UMLWidget:        return i18n("Enter the new name of the uml widget:");
        case WidgetBase::wt_UseCase:          return i18n("Enter the new name of the use case:");
        default:
            logWarn1("Widget_Utils::renameText unknown widget type: %1", WidgetBase::toString(type));
            return i18n("Enter the new name of the widget:");
        }
    }

    /**
     * Prevent nested widget(s) located inside the area of a larger widget from disappearing.
     *
     * @param self        The widget against which to test the other widgets of the diagram
     * @param widgetList  The widgets of the diagram
     */
    void ensureNestedVisible(UMLWidget *self, UMLWidgetList widgetList)
    {
        for(UMLWidget *other : widgetList) {
            if (other == self)
                continue;
            if (other->isLocatedIn(self)) {
                if (other->zValue() <= self->zValue())
                    other->setZValue(other->zValue() + 1.0);
            } else if (self->isLocatedIn(other)) {
                if (self->zValue() <= other->zValue())
                    self->setZValue(self->zValue() + 1.0);
            }
        }
    }

    /**
     * Adorn stereotype name with guillemets.
     */
    QString adornStereo(QString name, bool appendSpace /* = true */)
    {
        QString s = QString::fromUtf8("«") + name + QString::fromUtf8("»");
        if (appendSpace)
            s.append(QLatin1Char(' '));
        return s;
    }
}
