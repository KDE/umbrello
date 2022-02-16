/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TEST_CLASSIFIER_H
#define TEST_CLASSIFIER_H

#include "testbase.h"

/**
 * Unit test for class UMLClassifier (classifier.h).
 */
class TEST_classifier: public TestBase
{
    Q_OBJECT
private slots:
    void test_equal();
    void test_copyInto();
    void test_clone();
    void test_addAttributeWithType();
    void test_addAttributeWithObject();
    void test_addAttributeWithAttribute();
    void test_removeAndCountAttribute();
    void test_getAttributeList();
    void test_addOperationWithPosition();
    void test_addOperationWithLog();
    void test_checkOperationSignature();
    void test_removeAndCountOperation();
    void test_getOperationList();
    void test_addTemplateWithType();
    void test_addTemplateWithLog();
    void test_addTemplateWithPosition();
    void test_removeAndCountTemplate();
    void test_findTemplate();
    void test_getTemplateList();
    void test_takeItem();
    void test_getFilteredList();
    void test_resolveRef();
    void test_findOperations();
    void test_findChildObjectById();
    void test_findOperation();
    void test_findSuperClassConcepts();
    void test_findSubClassConcepts();
    void test_setGetClassAssoc();
    void test_isInterface();
    void test_setGetOriginType();
    void test_setGetIsReference();
    void test_hasAbstractOps();
    void test_makeChildObject();
    void test_getUniAssociationToBeImplemented();
    void test_saveAndLoad();
};

#endif // TEST_CLASSIFIER_H
