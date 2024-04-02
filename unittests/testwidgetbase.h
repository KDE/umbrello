/*
    SPDX-FileCopyrightText: 2020 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTWIDGETBASE_H
#define TESTWIDGETBASE_H

#include "testbase.h"

class TestWidgetBase : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void test_setSelected();
    void test_clearSelected();
};

#endif // TESTWIDGETBASE_H
