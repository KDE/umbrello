/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TEST_UMLCANVASOBJECT_H
#define TEST_UMLCANVASOBJECT_H

#include "testbase.h"

/**
 * Unit test for class UMLCanvasObject
 */
class TestUMLCanvasObject: public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void test_addAssociationEnd();
    void test_getAssociations();
    void test_removeAllAssociationEnds();
    void test_getSuperClasses();
    void test_getRealizations();
    void test_getAggregations();
    void test_getCompositions();
    void test_getRelationships();
};

#endif // TEST_UMLCANVASOBJECT_H
