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

private:
    Q_SLOT void test_createWidgetsSingleSelect();
    Q_SLOT void test_createWidgetsMultiSelect();
    Q_SLOT void test_createAssociationWidget();
    Q_SLOT void test_createUMLScene();
    Q_SLOT void test_createUMLListview();
    Q_SLOT void test_createMiscMenu();
};

#endif // TESTLISTPOPUPMENU_H
