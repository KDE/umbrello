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
#include "enum.h"
#include "newenumwidget.h"
#include "textitem.h"
#include "umlscene.h"
#include "umlview.h"

#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QTime>
#include <QtCore/QTimerEvent>

#include <kdebug.h>

Test* Test::m_self = 0;

struct TestPrivate
{
    TestPrivate() : scene(0), enumWidget(0), enumObject(0), count(0)
    {
    }

    UMLScene *scene;
    QString xml;
    NewEnumWidget *enumWidget;
    UMLEnum *enumObject;
    int count;
};

Test::Test() :
    d(new TestPrivate)
{
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
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
    d->scene = scene;

    BoxWidget *w = new BoxWidget("131313");
    scene->addItem(w);

    TextItem *item = new TextItem("Notice loading of gradient brush from xmi in 3 secs");

    item->setFlag(QGraphicsItem::ItemIsMovable, true);
    item->setAcceptHoverEvents(true);
    item->setHoverBrush(Qt::cyan);
    item->setBackgroundBrush(Qt::darkCyan);
    item->setDefaultTextColor(Qt::darkBlue);
    item->setTextWidth(100);

    scene->addItem(item);

    uDebug() << "entered test";

    UMLEnum *en = new UMLEnum("Qt::SizeHint");
    d->enumObject = en;
    en->createEnumLiteral("MinimumSize");
    en->createEnumLiteral("MaximumSize");
    en->createEnumLiteral("PreferredSize");
    en->createEnumLiteral("MaximumSize");
    NewEnumWidget *wid = new NewEnumWidget(en);

    wid->setFontColor(Qt::darkBlue);
    wid->setLineColor(Qt::darkGreen);
    wid->init();
    scene->addItem(wid);
    wid->setPos(40, 40);

    wid->setBrush(randomGradientBrush());
    wid->setSize(220, 120);

    QDomDocument doc("TEST");
    //doc
    QDomElement ele = doc.createElement("TopLevel");
    doc.appendChild(ele);

    wid->saveToXMI(doc, ele);

    d->xml = doc.toString();
    d->enumWidget = wid;
    wid->setBrush(randomGradientBrush());
    uDebug() << "-------------------";
    uDebug() << d->xml;
    uDebug() << "-------------------";
    wid->setShowPackage(true);
    en->createEnumLiteral("MaximumSizeHintTester");
    uDebug() << "leaving test";

    startTimer(3 * 1000);
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
    if(d->count == 0) {
        QDomDocument doc("TEST");
        doc.setContent(d->xml);

        QDomElement ele = doc.documentElement().firstChild().toElement();
        d->enumWidget->loadFromXMI(ele);
    }
    else if(d->count < 8) {
        d->enumObject->createEnumLiteral("MaximumSize" + QString::number(qrand() % 100));
    }
    else {
        killTimer(event->timerId());
    }
    ++d->count;
}

#include "test.moc"
