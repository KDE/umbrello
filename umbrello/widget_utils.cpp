/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "widget_utils.h"

// app includes
#include "objectwidget.h"
#include "uml.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

// qt/kde includes
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtGui/QPolygonF>

// c++ include
#include <cmath>

namespace Widget_Utils
{

    NewUMLRectWidget* findWidget(Uml::IDType id,
                                 const UMLWidgetList& widgets,
                                 const MessageWidgetList* pMessages /* = NULL */)
    {
        UMLWidgetListIt it( widgets );
        foreach ( NewUMLRectWidget* obj , widgets ) {
            if (obj->getBaseType() == Uml::wt_Object) {
                if (static_cast<ObjectWidget *>(obj)->localID() == id)
                    return obj;
            } else if (obj->getID() == id) {
                return obj;
            }
        }

        if (pMessages == NULL)
            return NULL;

        foreach ( NewUMLRectWidget* obj , *pMessages ) {
            if( obj->getID() == id )
                return obj;
        }
        return NULL;
    }

    QGraphicsRectItem *decoratePoint(const QPointF& p)
    {
        const int SIZE = 4;
        UMLView *currentView = UMLApp::app()->getCurrentView();
        QGraphicsRectItem *rect = new QGraphicsRectItem(0, 0, SIZE, SIZE);
        currentView->umlScene()->addItem(rect);
        rect->setPos(p.x() - SIZE / 2, p.y() - SIZE / 2);
        rect->setBrush( QBrush(Qt::blue) );
        rect->setPen( QPen(Qt::blue) );
        return rect;
    }

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

	void drawTriangledRect(QPainter *painter,
										 const QRectF& rect, const QSizeF& triSize)
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

    QString pointToString(const QPointF& point)
    {
        return QString("%1,%2").arg(point.x()).arg(point.y());
    }

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

    bool loadPixmapFromXMI(const QDomElement &qElement, QPixmap &pixmap)
    {
        return true;
    }

    void savePixmapToXMI(QDomDocument &qDoc, QDomElement &qElement, const QPixmap& pixmap)
    {

    }

    bool loadGradientFromXMI(const QDomElement &qElement, QGradient *&gradient)
    {
        QDomElement gradientElement = qElement.firstChildElement("gradient");
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

    bool loadBrushFromXMI(const QDomElement &qElement, QBrush &brush)
    {
        QDomElement brushElement = qElement.firstChildElement(QLatin1String("brush"));
        if(brushElement.isNull()) {
            return false;
        }

        quint8 style = brushElement.attribute("style").toShort();
        QColor color = brushElement.attribute("color");

        if(style == Qt::TexturePattern) {
            QPixmap pixmap;
            if(!loadPixmapFromXMI(brushElement, pixmap)) {
                return false;
            }
            brush = QBrush(color, pixmap);
        }

        else if(style == Qt::LinearGradientPattern
                || style == Qt::RadialGradientPattern
                || style == Qt::ConicalGradientPattern) {
            QGradient *gradient = 0;

            if(!loadGradientFromXMI(brushElement, gradient) || !gradient) {
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
}  // namespace Widget_Utils
