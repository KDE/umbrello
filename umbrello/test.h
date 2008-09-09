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

#ifndef TEST_H
#define TEST_H

#include <QtCore/QObject>

// THIS IS USED TO ONLY TEST FEATURES OF THE PORT
class UMLScene;
class TestPrivate;
class QBrush;
class WidgetBase;

class Test : public QObject
{
    Q_OBJECT;
public:
    Test();
    void testScene(UMLScene *scene);

    static Test* self();
    static Test *m_self;

    void updateWidgetGeometry(WidgetBase *);

    QBrush randomGradientBrush();

protected:
    void timerEvent(QTimerEvent *event);

private:
    QList<WidgetBase*> widgetsForUpdation;
    TestPrivate *d;
};

#endif //TEST_H
