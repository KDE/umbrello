/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TEST_CLASSIFIER_H
#define TEST_CLASSIFIER_H

#include <QObject>

/**
 * Unit test for class UMLClassifier (classifier.h).
 */
class TEST_classifier: public QObject
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
    void test_setBaseType();
    void test_isInterface();
    void test_isDatatype();
    void test_setGetOriginType();
    void test_setGetIsReference();
    void test_hasAbstractOps();
    void test_makeChildObject();
    void test_getUniAssociationToBeImplemented();
};

#endif // TEST_CLASSIFIER_H
