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

#include <QtCore/QBuffer>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QTime>
#include <QtCore/QTimerEvent>
#include <QtGui/QApplication>
#include <QtGui/QLabel>

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
    if (scene->getType() != Uml::dt_Sequence)
        return;

    UMLClassifier *obj = new UMLClassifier("hello");
    ObjectWidget *wid = new ObjectWidget(obj);
    wid->setPos(100, 100);

    UMLClassifier *obj1 = new UMLClassifier("world");
    ObjectWidget *wid1 = new ObjectWidget(obj1);
    wid1->setPos(200, 100);

    MessageWidget *msg = new MessageWidget(wid, wid1, Uml::sequence_message_asynchronous);

    scene->addItem(wid);
    scene->addItem(wid1);
    scene->addItem(msg);
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
