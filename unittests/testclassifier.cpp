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

#include "testclassifier.h"

// app include
#include "uml.h"
#include "classifier.h"

#define IS_NOT_IMPL() QSKIP("not implemented yet", SkipSingle)
//-----------------------------------------------------------------------------

//#define RUN_ALL

#ifdef RUN_ALL
#undef QCOMPARE
#define QCOMPARE(actual, expected) \
    QTest::qCompare(actual, expected, #actual, #expected, __FILE__, __LINE__)
#endif

void TEST_classifier::test_equal()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* b = a;
    UMLClassifier* c = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* d = new UMLClassifier("Test B", Uml::ID::None);
    QCOMPARE(*a == *b, true);
    QCOMPARE(*a == *c, true);
    QCOMPARE(*b == *c, true);
    QCOMPARE(*c == *d, false);
}

void TEST_classifier::test_copyInto()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* b = new UMLClassifier("Test B", Uml::ID::None);
    b->copyInto(a);
    QCOMPARE(*a == *b, true);
}

void TEST_classifier::test_clone()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLClassifier* b = a->clone()->asUMLClassifier();
    QCOMPARE(*a == *b, true);
}

void TEST_classifier::test_addAttributeWithType()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    UMLAttribute* attrA = a->addAttribute("attributeA_", Uml::ID::None);
    /* UMLAttribute* attrB = */ a->addAttribute("attributeB_", Uml::ID::None);
    int num1 = a->getAttributeList().count();
    QCOMPARE(num1, 2);
    int num2 = a->removeAttribute(attrA);
    QCOMPARE(num2, 1);  // one deleted
    int num3 = a->getAttributeList().count();
    QCOMPARE(num3, num1 - 1);
}

void TEST_classifier::test_addAttributeWithObject()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_addAttributeWithAttribute()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_removeAndCountAttribute()
{
    UMLClassifier* a = new UMLClassifier("Test A", Uml::ID::None);
    int num0 = a->getAttributeList().count();
    QCOMPARE(num0, 0);  // no attributes present yet
    /*UMLAttribute* attrA = */ a->addAttribute("attributeA_", Uml::ID::None);
    UMLAttribute* attrB = a->addAttribute("attributeB_", Uml::ID::None);
    UMLAttribute* attrC = a->addAttribute("attributeC_", Uml::ID::None);
    /* UMLAttribute* attrD = */ a->addAttribute("attributeD_", Uml::ID::None);
    int num1 = a->getAttributeList().count();
    QCOMPARE(num1, 4);
    int num2 = a->removeAttribute(attrB);
    QCOMPARE(num2, 3);  // one deleted
    num2 = a->removeAttribute(attrC);
    QCOMPARE(num2, 2);  // one deleted
    int num3 = a->getAttributeList().count();
    QCOMPARE(num3, 2); 
}

void TEST_classifier::test_getAttributeList()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_addOperationWithPosition()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_addOperationWithLog()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_checkOperationSignature()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_removeAndCountOperation()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_getOperationList()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_addTemplateWithType()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_addTemplateWithLog()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_addTemplateWithPosition()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_removeAndCountTemplate()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_findTemplate()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_getTemplateList()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_takeItem()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_getFilteredList()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_resolveRef()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_findOperations()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_findChildObjectById()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_findOperation()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_findSuperClassConcepts()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_findSubClassConcepts()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_setGetClassAssoc()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_setBaseType()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_isInterface()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_isDatatype()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_setGetOriginType()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_setGetIsReference()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_hasAbstractOps()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_makeChildObject()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_getUniAssociationToBeImplemented()
{
    IS_NOT_IMPL();
}

QTEST_MAIN(TEST_classifier)

class A {
public:
    A(int a) :_a(a) {}
    bool operator ==(const A & rhs) const
    {
        fprintf(stderr, "%s\n", __PRETTY_FUNCTION__);
        return _a == rhs._a;
    }
private:
    int _a;
};


#if 0
int main(int argc, char **argv)
{

//    A a(5);
//    A b(2);
//    A c(a);
//    printf("%d\n", a == b);
//    printf("%d\n", a == c);

    QApplication app(argc, argv);
    TEST_classifier z;
    z.initTestCase();
    z.test_equal();
}
#endif
