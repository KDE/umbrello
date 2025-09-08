/*
    SPDX-FileCopyrightText: 2020 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testwidgetbase.h"

#include "umlfolder.h"
#include "widgetbase.h"
#include "umlscene.h"
#include "umlview.h"
#include "umlwidget.h"

void TestWidgetBase::test_setSelected()
{
    UMLFolder folder(QStringLiteral("folder"));
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
    UMLFolder folder(QStringLiteral("folder"));
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
