/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTUMLOBJECT_H
#define TESTUMLOBJECT_H

#include "testbase.h"

class TestUMLObject : public TestBase
{
    Q_OBJECT
private slots:
    void test_copyInto();
    void test_clone();
    void test_doc();
    void test_equal();
    void test_fullyQualifiedName();
    void test_isAbstract();
    void test_isStatic();
    void test_resolveRef();
    void test_saveAndLoad();
    void test_setBaseType();
    void test_setStereotype();
    void test_setUMLPackage();
    void test_setVisibility();
    void test_toString();
    void test_dynamic_cast();
    void test_isUMLXXX();
};

#endif // TESTUMLOBJECT_H
