/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTASSOCIATION_H
#define TESTASSOCIATION_H

#include "testbase.h"

class TestAssociation : public TestBase
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
    void test_equal();
    void test_toString();
    void test_UMLRole();
    void test_associationType();
    void test_objectID();
    void test_visibility();
    void test_changeability();
    void test_multiplicity();
    void test_roleName();
    void test_roleDoc();
    void resolveRef();
    void test_saveAndLoad();
};

#endif // TESTASSOCIATION_H
