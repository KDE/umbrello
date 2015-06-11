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

#include "TEST_classifier.h"

// app include
#include "uml.h"
#include "classifier.h"

// qt includes
#include <QApplication>
#include <QtTest>


const bool IS_NOT_IMPL = false;

QApplication *app;
UMLApp *umlApp;

//-----------------------------------------------------------------------------
void TEST_classifier::initTestCase()
{
    // FIXME  we need to force QTEST_MAIN to use the GUI variant
    char **argv = { 0 };
    int argc = 0;
    app = new QApplication(argc, argv);
    QWidget *w = new QWidget;
    umlApp = new UMLApp(w);
}

void TEST_classifier::cleanupTestCase()
{
    delete umlApp;
    delete app;
}

//-----------------------------------------------------------------------------

void TEST_classifier::test_equal()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* b = a;
    UMLClassifier* c = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* d = new UMLClassifier("Test B", Uml::ID::None);
    QCOMPARE(a == b, true);
    QCOMPARE(a == c, true);
    QCOMPARE(b == c, true);
    QCOMPARE(c == d, false);
}

void TEST_classifier::test_copyInto()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* b = new UMLClassifier("Test B", Uml::ID::None);
    b->copyInto(a);
    QCOMPARE(a == b, true);
}

void TEST_classifier::test_clone()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* b;
    b = static_cast<UMLClassifier*>(a->clone());
    QCOMPARE(a == b, true);
}

void TEST_classifier::test_addAttributeWithType()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLAttribute* attrA = a->addAttribute("attributeA_", Uml::ID::None);
    /* UMLAttribute* attrB = */ a->addAttribute("attributeB_", Uml::ID::None);
    int num1 = a->attributes();
    QCOMPARE(num1, 2);
    int num2 = a->removeAttribute(attrA);
    QCOMPARE(num2, 1);  // one deleted
    int num3 = a->attributes();
    QCOMPARE(num3, num1 - 1);
}

void TEST_classifier::test_addAttributeWithObject()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_addAttributeWithAttribute()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_removeAndCountAttribute()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    int num0 = a->attributes();
    QCOMPARE(num0, 0);  // no attributes present yet
    /*UMLAttribute* attrA = */ a->addAttribute("attributeA_", Uml::ID::None);
    UMLAttribute* attrB = a->addAttribute("attributeB_", Uml::ID::None);
    UMLAttribute* attrC = a->addAttribute("attributeC_", Uml::ID::None);
    /* UMLAttribute* attrD = */ a->addAttribute("attributeD_", Uml::ID::None);
    int num1 = a->attributes();
    QCOMPARE(num1, 4);
    int num2 = a->removeAttribute(attrB);
    QCOMPARE(num2, 1);  // one deleted
    num2 = a->removeAttribute(attrC);
    QCOMPARE(num2, 1);  // one deleted
    int num3 = a->attributes();
    QCOMPARE(num3, 2); 
}

void TEST_classifier::test_getAttributeList()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_addOperationWithPosition()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_addOperationWithLog()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_checkOperationSignature()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_removeAndCountOperation()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_getOperationList()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_addTemplateWithType()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_addTemplateWithLog()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_addTemplateWithPosition()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_removeAndCountTemplate()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_findTemplate()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_getTemplateList()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_takeItem()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_getFilteredList()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_resolveRef()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_findOperations()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_findChildObjectById()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_findOperation()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_findSuperClassConcepts()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_findSubClassConcepts()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_setGetClassAssoc()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_setBaseType()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_isInterface()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_isDatatype()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_setGetOriginType()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_setGetIsReference()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_hasAbstractOps()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_makeChildObject()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

void TEST_classifier::test_getUniAssociationToBeImplemented()
{
    QCOMPARE(IS_NOT_IMPL, true);
}

QTEST_MAIN(TEST_classifier)
