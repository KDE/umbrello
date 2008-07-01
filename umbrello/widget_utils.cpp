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

// qt/kde includes
#include <q3canvas.h>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <kdebug.h>
// app includes
#include "uml.h"
#include "umlview.h"
#include "umlwidget.h"
#include "objectwidget.h"
#include "umlscene.h"


namespace Widget_Utils
{

    NewUMLRectWidget* findWidget(Uml::IDType id,
                          const UMLWidgetList& widgets,
                          const MessageWidgetList* pMessages /* = NULL */)
    {
        UMLWidgetListIt it( widgets );
        foreach ( NewUMLRectWidget* obj , widgets ) {
            if (obj->getBaseType() == Uml::wt_Object) {
                if (static_cast<ObjectWidget *>(obj)->getLocalID() == id)
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
