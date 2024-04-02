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

private:
    Q_SLOT void test_equal();
    Q_SLOT void test_copyInto();
    Q_SLOT void test_clone();
    Q_SLOT void test_addAttributeWithType();
    Q_SLOT void test_addAttributeWithObject();
    Q_SLOT void test_addAttributeWithAttribute();
    Q_SLOT void test_removeAndCountAttribute();
    Q_SLOT void test_getAttributeList();
    Q_SLOT void test_addOperationWithPosition();
    Q_SLOT void test_addOperationWithLog();
    Q_SLOT void test_checkOperationSignature();
    Q_SLOT void test_removeAndCountOperation();
    Q_SLOT void test_getOperationList();
    Q_SLOT void test_addTemplateWithType();
    Q_SLOT void test_addTemplateWithLog();
    Q_SLOT void test_addTemplateWithPosition();
    Q_SLOT void test_removeAndCountTemplate();
    Q_SLOT void test_findTemplate();
    Q_SLOT void test_getTemplateList();
    Q_SLOT void test_takeItem();
    Q_SLOT void test_getFilteredList();
    Q_SLOT void test_resolveRef();
    Q_SLOT void test_findOperations();
    Q_SLOT void test_findChildObjectById();
    Q_SLOT void test_findOperation();
    Q_SLOT void test_findSuperClassConcepts();
    Q_SLOT void test_findSubClassConcepts();
    Q_SLOT void test_setGetClassAssoc();
    Q_SLOT void test_isInterface();
    Q_SLOT void test_setGetOriginType();
    Q_SLOT void test_setGetIsReference();
    Q_SLOT void test_hasAbstractOps();
    Q_SLOT void test_makeChildObject();
    Q_SLOT void test_getUniAssociationToBeImplemented();
    Q_SLOT void test_saveAndLoad();
};

#endif // TEST_CLASSIFIER_H
