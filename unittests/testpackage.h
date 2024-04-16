/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTPACKAGE_H
#define TESTPACKAGE_H

#include "testbase.h"

class TestPackage : public TestBase
{
    Q_OBJECT
protected:
    SetLoading *_sl;
protected Q_SLOTS:
    void initTestCase()
    {
        TestBase::initTestCase();
        _sl = new SetLoading;
    }

    void cleanupTestCase()
    {
        delete _sl;
    }

private Q_SLOTS:
    void test_appendClassesAndInterfaces();
    void test_saveAndLoad();
};

#endif // TESTASSOCIATION_H
