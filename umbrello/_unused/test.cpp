/***************************************************************************
 * Copyright (C) 2008 by Gopala Krishna A <krishna.ggk@gmail.com>          *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "test.h"

#include "boxwidget.h"
#include "classifier.h"
#include "enum.h"
#include "floatingdashlinewidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "textitem.h"
#include "umlscene.h"
#include "umlview.h"
#include "widget_utils.h"

#include <QBuffer>
#include <QMetaObject>
#include <QMetaProperty>
#include <QTime>
#include <QTimerEvent>
#include <QApplication>
#include <QLabel>

#include <kdebug.h>

Test* Test::m_self = 0;

QBrush randHoverBrush()
{
    QLinearGradient grad(QPointF(0,0), QPointF(0, 1));
    int r, g, b, a;
    r = qrand() % 255;
    g = qrand() % 255;
    b = qrand() % 255;
    a = 10 + qrand() % 245; // set minimum to atleast 10
    grad.setColorAt(0, QColor(r, g, b, a));

    r = qrand() % 255;
    g = qrand() % 255;
    b = qrand() % 255;
    a = 10 + qrand() % 245; // set minimum to atleast 10
    grad.setColorAt(1, QColor(r, g, b, a));

    return QBrush(grad);
}

struct TestPrivate
{
    TestPrivate()
    {
    }
};

Test::Test() :
    d(new TestPrivate)
{
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    startTimer(100);
}

Test* Test::self()
{
    if(!m_self) {
        m_self = new Test();
    }
    return m_self;
}

void Test::testScene(UMLScene *scene)
{
    Q_UNUSED(scene);
//X     New::AssociationWidget *wid = new New::AssociationWidget(0, Uml::at_Association, 0, 0);
//X     New::AssociationLine *path1 = wid->associationLine();
//X     scene->addItem(wid);
//X     path1->insertPoint(0, QPointF(100, 100));
//X     path1->insertPoint(1, QPointF(200, 50));
//X     path1->insertPoint(2, QPointF(700, 300));
//X     wid->setLineColor(QColor(qrand() % 256, qrand() % 256, qrand() % 256));
//X     wid->setLineWidth(0);
//X 
//X     path1->setStartHeadSymbol(New::HeadSymbol::Arrow);
    // path1->setEndHeadSymbol(New::HeadSymbol::Circle);

    // New::AssociationLine *path2 = new New::AssociationLine(0);
    // scene->addItem(path2);
    // path2->insertPoint(0, QPointF(80, 400));
    // path2->insertPoint(1, QPointF(120, 450));
    // path2->insertPoint(2, QPointF(300, 350));
    // path2->setPen(QPen(Qt::darkGreen));

    // path2->setStartHeadSymbol(New::HeadSymbol::Arrow);
}

void Test::updateWidgetGeometry(WidgetBase *wid)
{
    widgetsForUpdation.append(wid);
}

QBrush Test::randomGradientBrush()
{
    QLinearGradient *gradient = new QLinearGradient();
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);

    int h, s, v;
    h = qrand() % 360;
    s = qrand() % 255;
    v = qrand() % 255;

    QColor colorBottom = QColor::fromHsv(h, s, v);

    colorBottom.toHsv().getHsv(&h, &s, &v);
    int diff = 50;
    if(v + diff <= 255) {
        v += diff;
    }
    else {
        v -= diff;
    }

    diff = 75;
    if(s + diff <= 255) {
        s += diff;
    }
    else {
        s -= diff;
    }
    QColor colorTop = QColor::fromHsv(h, s, v);
    gradient->setColorAt(0, colorTop);
    gradient->setColorAt(1, colorBottom);

    gradient->setStart(0, 0);
    gradient->setFinalStop(0, 1);

    QBrush retval(*gradient);
    delete gradient;
    return retval;
}

void Test::timerEvent(QTimerEvent *event)
{
    if (!widgetsForUpdation.isEmpty()) {
        WidgetBase *wid = widgetsForUpdation.takeFirst();
        wid->setFont(qApp->font());
    }
    return QObject::timerEvent(event);
}

#include "test.moc"
