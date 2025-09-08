/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testclassifier.h"

// app include
#include "umlapp.h"
#include "umlassociation.h"
#include "umlclassifier.h"
#include "umldatatype.h"
#include "umloperation.h"
#include "model_utils.h"

//-----------------------------------------------------------------------------

void TEST_classifier::test_equal()
{
    UMLClassifier* a = new UMLClassifier(QStringLiteral("Test A"), Uml::ID::None);
    UMLClassifier* b = a;
    UMLClassifier* c = new UMLClassifier(QStringLiteral("Test A"), Uml::ID::None);
    UMLClassifier* d = new UMLClassifier(QStringLiteral("Test B"), Uml::ID::None);
    QCOMPARE(*a == *b, true);
    QCOMPARE(*a == *c, true);
    QCOMPARE(*b == *c, true);
    QCOMPARE(*c == *d, false);
}

void TEST_classifier::test_copyInto()
{
    UMLClassifier a(QStringLiteral("Test A"), Uml::ID::None);
    UMLClassifier b(QStringLiteral("Test B"), Uml::ID::None);
    b.copyInto(&a);
    QCOMPARE(a == b, true);
}

void TEST_classifier::test_clone()
{
    UMLClassifier* a = new UMLClassifier(QStringLiteral("Test A"), Uml::ID::None);
    UMLClassifier* b = a->clone()->asUMLClassifier();
    QCOMPARE(*a == *b, true);
}

void TEST_classifier::test_addAttributeWithType()
{
    UMLClassifier a(QStringLiteral("Test A"), Uml::ID::None);
    a.addAttribute(QStringLiteral("attributeA_"), Uml::ID::None);
    UMLAttribute *attrA = a.addAttribute(QStringLiteral("attributeA_"), Uml::ID::None);
    /* UMLAttribute* attrB = */ a.addAttribute(QStringLiteral("attributeB_"), Uml::ID::None);
    int num1 = a.getAttributeList().count();
    QCOMPARE(num1, 2);
    int num2 = a.removeAttribute(attrA);
    QCOMPARE(num2, 1);  // one deleted
    int num3 = a.getAttributeList().count();
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
    UMLClassifier* a = new UMLClassifier(QStringLiteral("Test A"), Uml::ID::None);
    int num0 = a->getAttributeList().count();
    QCOMPARE(num0, 0);  // no attributes present yet
    /*UMLAttribute* attrA = */ a->addAttribute(QStringLiteral("attributeA_"), Uml::ID::None);
    UMLAttribute* attrB = a->addAttribute(QStringLiteral("attributeB_"), Uml::ID::None);
    UMLAttribute* attrC = a->addAttribute(QStringLiteral("attributeC_"), Uml::ID::None);
    /* UMLAttribute* attrD = */ a->addAttribute(QStringLiteral("attributeD_"), Uml::ID::None);
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
    qDebug() << "already tested by testumlobject";
}

void TEST_classifier::test_findOperations()
{
    UMLClassifier c(QStringLiteral("Test A"), Uml::ID::None);
    UMLOperation o1(nullptr, QStringLiteral("testop1"));
    c.addOperation(&o1);
    int num1 = c.getOpList().count();
    QCOMPARE(num1, 1);
    UMLOperation o2(nullptr, QStringLiteral("testop2"));
    c.addOperation(&o2);
    int num2 = c.getOpList().count();
    QCOMPARE(num2, 2);
    QCOMPARE(c.findOperations(QStringLiteral("testop1")).count(), 1);
    QCOMPARE(c.findOperations(QStringLiteral("testop2")).count(), 1);
    QCOMPARE(c.findOperations(QStringLiteral("testOp1")).count(), 0);
    QCOMPARE(c.findOperations(QStringLiteral("testOp2")).count(), 0);
    // case insensitive language
    Uml::ProgrammingLanguage::Enum lang = UMLApp::app()->activeLanguage();
    UMLApp::app()->setActiveLanguage(Uml::ProgrammingLanguage::PostgreSQL);
    QCOMPARE(c.findOperations(QStringLiteral("testOp1")).count(), 1);
    QCOMPARE(c.findOperations(QStringLiteral("testOp2")).count(), 1);
    UMLApp::app()->setActiveLanguage(lang);
}

void TEST_classifier::test_findChildObjectById()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_findOperation()
{
    UMLClassifier c(QStringLiteral("Test A"), Uml::ID::None);
    UMLOperation o1(nullptr, QStringLiteral("testop1"));
    UMLAttribute a1(nullptr, QStringLiteral("aParam"));
    a1.setTypeName(QStringLiteral("int"));
    o1.addParm(&a1);
    c.addOperation(&o1);
    UMLOperation o2(nullptr, QStringLiteral("testop1"));
    UMLAttribute a2(nullptr, QStringLiteral("aParam"));
    a2.setTypeName(QStringLiteral("double"));
    o2.addParm(&a2);
    c.addOperation(&o2);
    Model_Utils::NameAndType_List searchTypes;
    // first function
    searchTypes << Model_Utils::NameAndType(QStringLiteral("aParam"), a1.getType());
    UMLOperation *o = c.findOperation(QStringLiteral("testop1"), searchTypes);
    QVERIFY(o);
    // second function
    searchTypes.clear();
    searchTypes << Model_Utils::NameAndType(QStringLiteral("aParam"), a2.getType());
    o = c.findOperation(QStringLiteral("testop1"), searchTypes);
    QVERIFY(o);

    // unknown type
    UMLDatatype d1(QStringLiteral("someType"));
    searchTypes.clear();
    searchTypes << Model_Utils::NameAndType(QStringLiteral("aParam"), &d1);
    o = c.findOperation(QStringLiteral("testop1"), searchTypes);
    QVERIFY(!o);

#if 0
    // different param name
    searchTypes.clear();
    searchTypes << Model_Utils::NameAndType("otherParam", a1.getType());
    o = c.findOperation("testop1", searchTypes);
    QVERIFY(!o);

    // different param name
    searchTypes.clear();
    searchTypes << Model_Utils::NameAndType("otherParam", a2.getType());
    o = c.findOperation("testop1", searchTypes);
    QVERIFY(!o);
#else
    qDebug() <<"finding param names is not supported";
#endif
}

void TEST_classifier::test_findSuperClassConcepts()
{
    UMLClassifier c1(QStringLiteral("Test A"));
    UMLClassifier c2(QStringLiteral("Test B"));
    UMLAssociation a1(Uml::AssociationType::Generalization, &c1, &c2);
    QCOMPARE(c1.findSuperClassConcepts(UMLClassifier::ALL).size(), 0);
    c1.addAssociationEnd(&a1);
    QCOMPARE(c1.findSuperClassConcepts(UMLClassifier::ALL).size(), 1);
    UMLAssociation a2(Uml::AssociationType::Realization, &c1, &c2);
    c1.addAssociationEnd(&a2);
    QCOMPARE(c1.findSuperClassConcepts(UMLClassifier::ALL).size(), 2);
}

void TEST_classifier::test_findSubClassConcepts()
{
    UMLClassifier c1(QStringLiteral("Test A"));
    UMLClassifier c2(QStringLiteral("Test B"));
    UMLAssociation a1(Uml::AssociationType::Generalization, &c1, &c2);
    QCOMPARE(c2.findSubClassConcepts(UMLClassifier::ALL).size(), 0);
    c2.addAssociationEnd(&a1);
    QCOMPARE(c2.findSubClassConcepts(UMLClassifier::ALL).size(), 1);
    UMLAssociation a2(Uml::AssociationType::Realization, &c1, &c2);
    c2.addAssociationEnd(&a2);
    QCOMPARE(c2.findSubClassConcepts(UMLClassifier::ALL).size(), 2);
}

void TEST_classifier::test_setGetClassAssoc()
{
    IS_NOT_IMPL();
}

void TEST_classifier::test_isInterface()
{
    UMLClassifier c1(QStringLiteral("Test A"));
    QCOMPARE(c1.isInterface(), false);
    c1.setBaseType(UMLObject::ObjectType::ot_Interface);
    QCOMPARE(c1.isInterface(), true);
    c1.setBaseType(UMLObject::ObjectType::ot_Class);
    QCOMPARE(c1.isInterface(), false);
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

typedef TestUML<UMLClassifier, const QString&> TestUMLClassifier;

void TEST_classifier::test_saveAndLoad()
{
    UMLPackage parent(QStringLiteral("test package"));
    TestUMLClassifier c1(QStringLiteral("Test A"));
    c1.setUMLPackage(&parent);
    UMLOperation o1(nullptr, QStringLiteral("testop1"));
    c1.addOperation(&o1);
    UMLOperation o2(nullptr, QStringLiteral("testop2"));
    c1.addOperation(&o2);
    QString save = c1.testSave1();
    //c1.testDump("save");
    TestUMLClassifier c2;
    c2.setUMLPackage(&parent);
    QCOMPARE(c2.testLoad1(save), true);
    //c2.testDump("after load");
    QCOMPARE(c2.testSave1(), save);
}

QTEST_MAIN(TEST_classifier)
