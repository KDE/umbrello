/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTPRECONDITIONWIDGET_H
#define TESTPRECONDITIONWIDGET_H

#include "testbase.h"

class TestPreconditionWidget : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void test_saveAndLoad();
};

#endif // TESTPRECONDITIONWIDGET_H
