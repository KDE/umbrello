/*
    Copyright 2020  Ralf Habacker  <ralf.habacker@freenet.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testwidgetbase.h"

#include "folder.h"
#include "widgetbase.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

void TestWidgetBase::test_setSelected()
{
    UMLFolder folder("folder");
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    WidgetBase widget1(&scene, WidgetBase::wt_UMLWidget);
    scene.addItem(&widget1);

    // simple widget
    widget1.setSelected(true);
    QVERIFY(widget1.isSelected());
    widget1.setSelected(false);
    QVERIFY(!widget1.isSelected());
    widget1.setSelected(true);
    scene.clearSelected();
    QVERIFY(!widget1.isSelected());
}

void TestWidgetBase::test_clearSelected()
{
    UMLFolder folder("folder");
    UMLView view(&folder);
    UMLScene scene(&folder, &view);
    UMLWidget widget1(&scene, WidgetBase::wt_Text, nullptr);
    scene.addItem(&widget1);
    UMLWidget widget2(&scene, WidgetBase::wt_Box, nullptr);
    scene.addItem(&widget2);
    widget2.setSelected(true);
    widget1.setSelected(true);
    QVERIFY(widget1.isSelected());
    QVERIFY(widget2.isSelected());
    scene.clearSelected();
    QVERIFY(!widget1.isSelected());
    QVERIFY(!widget2.isSelected());
    widget2.setSelected(true);
    widget1.setSelected(true);
    QVERIFY(widget1.isSelected());
    QVERIFY(widget2.isSelected());
    scene.clearSelection();
}

QTEST_MAIN(TestWidgetBase)
