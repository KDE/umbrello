/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_utils.h"

// app includes
#include "debug_utils.h"
#include "objectwidget.h"
#include "messagewidget.h"
#include "uml.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

// qt includes
#include <QBuffer>
#include <QImageReader>
#include <QPolygonF>

// c++ include
#include <cmath>

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
    UMLWidget* findWidget(Uml::IDType id,
                          const UMLWidgetList& widgets,
                          const MessageWidgetList* messages /* = 0 */)
    {
        foreach (UMLWidget* obj, widgets) {
            if (obj->baseType() == WidgetBase::wt_Object) {
                if (static_cast<ObjectWidget *>(obj)->localID() == id)
                    return obj;
            } else if (obj->id() == id) {
                return obj;
            }
        }

        if (messages) {
            foreach (UMLWidget* obj, *messages) {
                if (obj->id() == id)
                    return obj;
            }
        }
        return NULL;
    }

    /**
     * Creates the decoration point.
     * @param p   the base point
     * @return    the decoration point
     */
    UMLSceneRectItem* decoratePoint(const UMLScenePoint& p)
    {
        const int SIZE = 4;
        UMLView *currentView = UMLApp::app()->currentView();
        UMLSceneRectItem *rect = new UMLSceneRectItem(p.x() - SIZE / 2,
                                                      p.y() - SIZE / 2,
                                                      SIZE, SIZE);
        currentView->umlScene()->addItem(rect);
        rect->setBrush( QBrush(Qt::blue) );
        rect->setPen( QPen(Qt::blue) );
        return rect;
    }

    /**
     * Calculates and draws a cross inside an ellipse
     * @param p Pointer to a QPainter object.
     * @param ellipse The rectangle describing the ellipse.
     */
    void drawCrossInEllipse(QPainter *p, const QRectF& r)
    {
        QRectF ellipse = r;
        ellipse.moveCenter(QPointF(0, 0));
        qreal a = ellipse.width() * 0.5;
        qreal b = ellipse.height() * .5;
        qreal xc = ellipse.center().x();
        qreal yc = ellipse.center().y();

        // The first point's x value is chose to be center.x() + 70% of x radius.
        qreal x1 = ellipse.center().x() + .7 * .5 * ellipse.width();
        // Calculate y1 correspoding to x1 using formula.
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
     * Draws a polygon which is almost rectanguar except for the top
     * right corner. A triangle is drawn in top right corner of the
     * rectangle.
     *
     * @param painter The painter with which this shape is to be drawn.
     * @param rect    The rectangle dimensions.
     * @param triSize The size of the triange in the top-right corner.
     */
    void drawTriangledRect(QPainter *painter,
                           const UMLSceneRect& rect, const UMLSceneSize& triSize)
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
        UMLSceneLine heightEdge(poly[1], poly[1] + QPointF(0, triSize.height()));
        painter->drawLine(heightEdge);
        UMLSceneLine baseEdge(heightEdge.p2(), poly[2]);
        painter->drawLine(baseEdge);
    }

    /**
     * Draws an arrow head with the given painter, with the arrow
     * sharp point at \a headPos.
     *
     * @param painter    The painter with which this arrow should be drawn.
     * @param headPos    The position where the head of the arrow should lie.
     * @param arrowSize  This indicates the size of the arrow head.
     * @param arrowType  This indicates direction of arrow as in LeftArrow, RightArrow..
     * @param solid      If true, a solid head is drawn. Otherwise 2 lines are drawn.
     */
    void drawArrowHead(QPainter *painter, const QPointF &arrowPos,
                       const QSizeF& arrowSize, Qt::ArrowType arrowType,
                       bool  solid)
    {
        QPolygonF poly;
        if (arrowType == Qt::LeftArrow) {
            poly << QPointF(arrowPos.x() + arrowSize.width(), arrowPos.y() - .5 * arrowSize.height())
                 << arrowPos
                 << QPointF(arrowPos.x() + arrowSize.width(), arrowPos.y() + .5 * arrowSize.height());
        }
        else if (arrowType == Qt::RightArrow) {
            poly << QPointF(arrowPos.x() - arrowSize.width(), arrowPos.y() - .5 * arrowSize.height())
                 << arrowPos
                 << QPointF(arrowPos.x() - arrowSize.width(), arrowPos.y() + .5 * arrowSize.height());
        }

        if (solid) {
            painter->drawPolygon(poly);
        }
        else {
            painter->drawPolyline(poly);
        }
    }

    /**
     * Draws a rounded rect rounded at specified corners.
     *
     * @param painter The painter with which this round rect should be drawn.
     * @param rect    The rectangle to be drawn.
     * @param xRadius The x radius of rounded corner.
     * @param yRadius The y radius of rounded corner.
     * @param corners The corners to be rounded.
     */
    void drawRoundedRect(QPainter *painter, const QRectF& rect, qreal xRadius,
            qreal yRadius, Uml::Corners corners)
    {
        if (xRadius < 0 || yRadius < 0) {
            painter->drawRect(rect);
            return;
        }
        QRectF arcRect(0, 0, 2 * xRadius, 2 * yRadius);

        QPainterPath path;
        path.moveTo(rect.left(), rect.top() + yRadius);
        if (corners.testFlag(Uml::corner_TopLeft)) {
            arcRect.moveTopLeft(rect.topLeft());
            path.arcTo(arcRect, 180, -90);
        } else {
            path.lineTo(rect.topLeft());
        }

        path.lineTo(rect.right() - xRadius, rect.top());

        if (corners.testFlag(Uml::corner_TopRight)) {
            arcRect.moveTopRight(rect.topRight());
            path.arcTo(arcRect, 90, -90);
        } else {
            path.lineTo(rect.topRight());
        }

        path.lineTo(rect.right(), rect.bottom() - yRadius);

        if (corners.testFlag(Uml::corner_BottomRight)) {
            arcRect.moveBottomRight(rect.bottomRight());
            path.arcTo(arcRect, 0, -90);
        } else {
            path.lineTo(rect.bottomRight());
        }

        path.lineTo(rect.left() + xRadius, rect.bottom());

        if (corners.testFlag(Uml::corner_BottomLeft)) {
            arcRect.moveBottomLeft(rect.bottomLeft());
            path.arcTo(arcRect, 270, 90);
        } else {
            path.lineTo(rect.bottomLeft());
        }

        path.closeSubpath();
        painter->drawPath(path);
    }

    /**
     * Converts a point to a comma separated string i.e "x,y"
     */
    QString pointToString(const QPointF& point)
    {
        return QString("%1,%2").arg(point.x()).arg(point.y());
    }

    /**
     * Converts a comma separated string to point.
     */
    QPointF stringToPoint(const QString& str)
    {
        QPointF retVal;
        QStringList list = str.split(',');

        if(list.size() == 2) {
            retVal.setX(list.first().toDouble());
            retVal.setY(list.last().toDouble());
        }
        return retVal;
    }

    /**
     * Loads pixmap from xmi.
     *
     * @param qElement The dom element from which pixmap should be
     *                 loaded.
     *
     * @param pixmap The pixmap into which the image should be loaded.
     *
     * @return True or false based on success or failure of this method.
     */
    bool loadPixmapFromXMI(QDomElement &pixEle, QPixmap &pixmap)
    {
        if (pixEle.isNull()) {
            return false;
        }
        QDomElement xpmElement = pixEle.firstChildElement("xpm");

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
     * Saves pixmap informatin into dom element \a qElement.
     *
     * @param qDoc The dom document object.
     *
     * @param qElement The dom element into which the pixmap should be
     *                 saved.
     *
     * @param pixmap The pixmap to be saved.
     */
    void savePixmapToXMI(QDomDocument &qDoc, QDomElement &qElement, const QPixmap& pixmap)
    {
        QDomElement pixmapElement = qDoc.createElement("pixmap");

        QDomElement xpmElement = qDoc.createElement("xpm");
        pixmapElement.appendChild(xpmElement);

        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "xpm");
        buffer.close();

        xpmElement.appendChild(qDoc.createTextNode(QString(buffer.data())));

        qElement.appendChild(pixmapElement);
    }

    /**
     * Loads gradient from xmi. The gradient pointer should be null
     * and the new gradient object will be created inside this method.
     * The gradient should later be deleted externally.
     *
     * @param qElement The dom element from which gradient should be
     *                 loaded.
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

        type_as_int = gradientElement.attribute("type").toInt();
        type = QGradient::Type(type_as_int);
        type_as_int = gradientElement.attribute("spread").toInt();
        spread = QGradient::Spread(type_as_int);
        type_as_int = gradientElement.attribute("coordinatemode").toInt();
        cmode = QGradient::CoordinateMode(type_as_int);

        QDomElement stopElement = gradientElement.firstChildElement("stops");
        if(stopElement.isNull()) {
            return false;
        }
        for(QDomNode node = stopElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement ele = node.toElement();
            if(ele.tagName() != QLatin1String("stop")) {
                continue;
            }

            qreal posn = ele.attribute("position").toDouble();
            QColor color = QColor(ele.attribute("color"));
            stops << QGradientStop(posn, color);
        }

        if (type == QGradient::LinearGradient) {
            QPointF p1 = stringToPoint(gradientElement.attribute("start"));
            QPointF p2 = stringToPoint(gradientElement.attribute("finalstop"));
            gradient = new QLinearGradient(p1, p2);
        }
        else if (type == QGradient::RadialGradient) {
            QPointF center = stringToPoint(gradientElement.attribute("center"));
            QPointF focal = stringToPoint(gradientElement.attribute("focalpoint"));
            double radius = gradientElement.attribute("radius").toDouble();
            gradient = new QRadialGradient(center, radius, focal);
        }
        else { // type == QGradient::ConicalGradient
            QPointF center = stringToPoint(gradientElement.attribute("center"));
            double angle = gradientElement.attribute("angle").toDouble();
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
     * Saves gradient information into dom element \a qElement.
     *
     * @param qDoc The dom document object.
     *
     * @param qElement The dom element into which the gradient should be
     *                 saved.
     *
     * @param gradient The gradient to be saved.
     */
    void saveGradientToXMI(QDomDocument &qDoc, QDomElement &qElement, const QGradient *gradient)
    {
        QDomElement gradientElement = qDoc.createElement("gradient");

        gradientElement.setAttribute("type", int(gradient->type()));
        gradientElement.setAttribute("spread", int(gradient->spread()));
        gradientElement.setAttribute("coordinatemode", int(gradient->coordinateMode()));

        QDomElement stopsElement = qDoc.createElement("stops");
        gradientElement.appendChild(stopsElement);

        foreach(QGradientStop stop, gradient->stops()) {
            QDomElement ele = qDoc.createElement("stop");
            ele.setAttribute("position", stop.first);
            ele.setAttribute("color", stop.second.name());
            stopsElement.appendChild(ele);
        }

        QGradient::Type type = gradient->type();

        if(type == QGradient::LinearGradient) {
            const QLinearGradient *lg = static_cast<const QLinearGradient*>(gradient);
            gradientElement.setAttribute("start", pointToString(lg->start()));
            gradientElement.setAttribute("finalstop", pointToString(lg->finalStop()));
        }
        else if(type == QGradient::RadialGradient) {
            const QRadialGradient *rg = static_cast<const QRadialGradient*>(gradient);
            gradientElement.setAttribute("center", pointToString(rg->center()));
            gradientElement.setAttribute("focalpoint", pointToString(rg->focalPoint()));
            gradientElement.setAttribute("radius", rg->radius());
        }
        else { //type == QGradient::ConicalGradient
            const QConicalGradient *cg = static_cast<const QConicalGradient*>(gradient);
            gradientElement.setAttribute("center", pointToString(cg->center()));
            gradientElement.setAttribute("angle", cg->angle());
        }

        qElement.appendChild(gradientElement);
    }

    /**
     * Extracts the QBrush properties into brush from the XMI xml
     * element qElement.
     *
     * @param qElement The dom element from which the xmi info should
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

        quint8 style = qElement.attribute("style").toShort();
        const QString colorString = qElement.attribute("color");
        QColor color;
        color.setNamedColor(colorString);

        if(style == Qt::TexturePattern) {
            QPixmap pixmap;
            QDomElement pixElement = qElement.firstChildElement("pixmap");
            if(!loadPixmapFromXMI(pixElement, pixmap)) {
                return false;
            }
            brush = QBrush(color, pixmap);
        }

        else if(style == Qt::LinearGradientPattern
                || style == Qt::RadialGradientPattern
                || style == Qt::ConicalGradientPattern) {
            QGradient *gradient = 0;
            QDomElement gradElement = qElement.firstChildElement("gradient");

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
     * Saves the brush info as xmi into the dom element \a qElement.
     *
     * @param qDoc The QDomDocument object pointing to the xmi document.
     *
     * @param qElement The element into which the pen, brush and font
     *                 info should be saved.
     *
     * @param brush The QBrush whose details should be saved.
     */
    void saveBrushToXMI(QDomDocument &qDoc, QDomElement &qElement,
                        const QBrush& brush)
    {
        QDomElement brushElement = qDoc.createElement("brush");

        brushElement.setAttribute("style", (quint8)brush.style());
        brushElement.setAttribute("color", brush.color().name());

        if(brush.style() == Qt::TexturePattern) {
            savePixmapToXMI(qDoc, brushElement, brush.texture());
        }
        else if(brush.style() == Qt::LinearGradientPattern
                || brush.style() == Qt::RadialGradientPattern
                || brush.style() == Qt::ConicalGradientPattern) {
            saveGradientToXMI(qDoc, brushElement, brush.gradient());
        }

        //TODO: Check if transform of this brush needs to be saved.
        qElement.appendChild(brushElement);
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

}  // namespace Widget_Utils
