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

#include "umlscene.h"
#include "umlview.h"
#include "newenumwidget.h"
#include "enum.h"
#include "textitem.h"

#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QTimerEvent>
#include <kdebug.h>

Test* Test::m_self = 0;

struct TestPrivate
{
    TestPrivate() : scene(0), enumWidget(0)
    {}

    UMLScene *scene;
    QString xml;
    NewEnumWidget *enumWidget;
};

Test::Test() :
    d(new TestPrivate)
{
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
    QLinearGradient ling(QPointF(0, 0), QPointF(0, 1));
    ling.setCoordinateMode(QGradient::ObjectBoundingMode);

    QColor col1(Qt::darkGray);
    QColor col2(Qt::lightGray);
    ling.setColorAt(0, col1);
    ling.setColorAt(1, col2);

    wid->setBrush(QBrush(ling));
    wid->setSize(220, 120);

    QDomDocument doc("TEST");
    //doc
    QDomElement ele = doc.createElement("TopLevel");
    doc.appendChild(ele);

    wid->saveToXMI(doc, ele);

    d->xml = doc.toString();
    d->enumWidget = wid;
    wid->setBrush(Qt::darkYellow);
    uDebug() << "-------------------";
    uDebug() << d->xml;
    uDebug() << "-------------------";
    wid->setShowPackage(true);
    en->createEnumLiteral("MaximumSizeHintTester");
    uDebug() << "leaving test";

    startTimer(3 * 1000);
}


void Test::timerEvent(QTimerEvent *event)
{
    QDomDocument doc("TEST");
    doc.setContent(d->xml);

    QDomElement ele = doc.documentElement().firstChild().toElement();
    d->enumWidget->loadFromXMI(ele);
    killTimer(event->timerId());
}

#include "test.moc"
