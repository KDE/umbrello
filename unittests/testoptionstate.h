/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTOPTIONSTATE_H
#define TESTOPTIONSTATE_H

#include "testbase.h"

class TestOptionState : public TestBase
{
    Q_OBJECT

private slots:
    void test_create();
    void test_saveAndLoad();
};

#endif // TESTOPTIONSTATE_H
