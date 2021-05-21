/*
    SPDX-FileCopyrightText: 2018 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTLISTPOPUPMENU_H
#define TESTLISTPOPUPMENU_H

#include "testbase.h"

class TestListPopupMenu : public TestBase
{
    Q_OBJECT

private slots:
    void test_createWidgetsSingleSelect();
    void test_createWidgetsMultiSelect();
    void test_createAssociationWidget();
    void test_createUMLScene();
    void test_createUMLListview();
    void test_createMiscMenu();
};

#endif // TESTLISTPOPUPMENU_H
