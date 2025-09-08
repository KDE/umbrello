/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testassociation.h"

// app include
#include "umlassociation.h"
#include "classifier.h"
#include "folder.h"
#include "package.h"
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlrole.h"

#include <QXmlStreamWriter>

//-----------------------------------------------------------------------------

void TestAssociation::test_equal()
{
    const SignalBlocker sb(UMLApp::app()->document());
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    a.setUMLPackage(&parent);
    UMLAssociation b(Uml::AssociationType::Association, &o1, &o2);
    b.setUMLPackage(&parent);
    UMLAssociation c(Uml::AssociationType::Association, &o1, nullptr);
    c.setUMLPackage(&parent);
    UMLAssociation d(Uml::AssociationType::Association, nullptr, &o2);
    d.setUMLPackage(&parent);
    //QCOMPARE(a, b);
    QCOMPARE(a == c, false);
    QCOMPARE(b == c, false);
    QCOMPARE(c == d, false);
}

void TestAssociation::test_toString()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, nullptr);
    QString ar = QString(QStringLiteral("objectA: %1 null")).arg(Uml::AssociationType::toStringI18n(Uml::AssociationType::Association));
    QCOMPARE(a.toString(), ar);
    UMLAssociation b(Uml::AssociationType::Association, nullptr, &o2);
    QString br = QString(QStringLiteral("null %1 objectB:")).arg(Uml::AssociationType::toStringI18n(Uml::AssociationType::Association));
    QCOMPARE(b.toString(), br);
    UMLAssociation c(Uml::AssociationType::Association, &o1, &o2);
    QString cr = QString(QStringLiteral("objectA: %1 objectB:")).arg(Uml::AssociationType::toStringI18n(Uml::AssociationType::Association));
    QCOMPARE(c.toString(), cr);
}

void TestAssociation::test_UMLRole()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getUMLRole(Uml::RoleType::A)->object(), &o1);
    QCOMPARE(a.getUMLRole(Uml::RoleType::B)->object(), &o2);
}

void TestAssociation::test_associationType()
{
    UMLAssociation a(Uml::AssociationType::Association);
    QVERIFY(a.getAssocType() == Uml::AssociationType::Association);
    a.setAssociationType(Uml::AssociationType::Aggregation);
    QVERIFY(a.getAssocType() == Uml::AssociationType::Aggregation);
}

void TestAssociation::test_objectID()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getObjectId(Uml::RoleType::A), o1.id());
    QCOMPARE(a.getObjectId(Uml::RoleType::B), o2.id());
}

void TestAssociation::test_visibility()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Public);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Public);
    a.setVisibility(Uml::Visibility::Protected, Uml::RoleType::A);
    a.setVisibility(Uml::Visibility::Protected, Uml::RoleType::B);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Protected);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Protected);
    a.setVisibility(Uml::Visibility::Private, Uml::RoleType::A);
    a.setVisibility(Uml::Visibility::Private, Uml::RoleType::B);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Private);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Private);
    a.setVisibility(Uml::Visibility::Implementation, Uml::RoleType::A);
    a.setVisibility(Uml::Visibility::Implementation, Uml::RoleType::B);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Implementation);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Implementation);
    UMLAssociation b(Uml::AssociationType::Association);
}

void TestAssociation::test_changeability()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Public);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Public);
    a.setVisibility(Uml::Visibility::Protected, Uml::RoleType::A);
    a.setVisibility(Uml::Visibility::Protected, Uml::RoleType::B);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Protected);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Protected);
    a.setVisibility(Uml::Visibility::Private, Uml::RoleType::A);
    a.setVisibility(Uml::Visibility::Private, Uml::RoleType::B);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Private);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Private);
    a.setVisibility(Uml::Visibility::Implementation, Uml::RoleType::A);
    a.setVisibility(Uml::Visibility::Implementation, Uml::RoleType::B);
    QVERIFY(a.visibility(Uml::RoleType::A) == Uml::Visibility::Implementation);
    QVERIFY(a.visibility(Uml::RoleType::B) == Uml::Visibility::Implementation);
    UMLAssociation b(Uml::AssociationType::Association);
    QVERIFY(b.visibility(Uml::RoleType::A) == Uml::Visibility::Public);
    QVERIFY(b.visibility(Uml::RoleType::B) == Uml::Visibility::Public);
}

void TestAssociation::test_multiplicity()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getMultiplicity(Uml::RoleType::A), QString());
    QCOMPARE(a.getMultiplicity(Uml::RoleType::B), QString());
    a.setMultiplicity(QStringLiteral("1"), Uml::RoleType::A);
    a.setMultiplicity(QStringLiteral("2"), Uml::RoleType::B);
    QCOMPARE(a.getMultiplicity(Uml::RoleType::A), QStringLiteral("1"));
    QCOMPARE(a.getMultiplicity(Uml::RoleType::B), QStringLiteral("2"));
}

void TestAssociation::test_roleName()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getRoleName(Uml::RoleType::A), QString());
    QCOMPARE(a.getRoleName(Uml::RoleType::B), QString());
    a.setRoleName(QStringLiteral("test1"), Uml::RoleType::A);
    a.setRoleName(QStringLiteral("test2"), Uml::RoleType::B);
    QCOMPARE(a.getRoleName(Uml::RoleType::A), QStringLiteral("test1"));
    QCOMPARE(a.getRoleName(Uml::RoleType::B), QStringLiteral("test2"));
}

void TestAssociation::test_roleDoc()
{
    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getRoleDoc(Uml::RoleType::A), QString());
    QCOMPARE(a.getRoleDoc(Uml::RoleType::B), QString());
    a.setRoleDoc(QStringLiteral("test1"), Uml::RoleType::A);
    a.setRoleDoc(QStringLiteral("test2"), Uml::RoleType::B);
    QCOMPARE(a.getRoleDoc(Uml::RoleType::A), QStringLiteral("test1"));
    QCOMPARE(a.getRoleDoc(Uml::RoleType::B), QStringLiteral("test2"));
}

void TestAssociation::resolveRef()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLStereotype *stereotype1 = UMLApp::app()->document()->createStereotype(QStringLiteral("test1"));
    UMLStereotype *stereotype2 = UMLApp::app()->document()->createStereotype(QStringLiteral("test2"));

    UMLObject o1(nullptr, QStringLiteral("objectA"));
    UMLObject o2(nullptr, QStringLiteral("objectB"));
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    // no resolve
    a.setUMLPackage(&parent);
    QCOMPARE(a.resolveRef(), true);

    // secondary
    a.getUMLRole(Uml::RoleType::A)->setSecondaryId(Uml::ID::toString(stereotype1->id()));
    a.getUMLRole(Uml::RoleType::B)->setSecondaryId(Uml::ID::toString(stereotype2->id()));
    QCOMPARE(a.resolveRef(), true);

    // secondary fallback
    a.getUMLRole(Uml::RoleType::A)->setSecondaryId(QStringLiteral(""));
    a.getUMLRole(Uml::RoleType::A)->setSecondaryFallback(Uml::ID::toString(stereotype1->id()));
    a.getUMLRole(Uml::RoleType::B)->setSecondaryId(QStringLiteral(""));
    a.getUMLRole(Uml::RoleType::B)->setSecondaryFallback(Uml::ID::toString(stereotype2->id()));
    QCOMPARE(a.resolveRef(), true);
}

typedef TestUML<UMLAssociation, Uml::AssociationType::Enum> TestUMLAssociation;

void TestAssociation::test_saveAndLoad()
{
    UMLPackage parent(QStringLiteral("Test Parent"));
    UMLClassifier c1(QStringLiteral("Test A"));
    UMLClassifier c2(QStringLiteral("Test B"));
    c1.setUMLPackage(&parent);
    c2.setUMLPackage(&parent);
    c1.setStereotypeCmd(QStringLiteral("test"));
    UMLFolder *root = UMLApp::app()->document()->rootFolder(Uml::ModelType::Logical);
    root->addObject(&c1);
    root->addObject(&c2);
    TestUMLAssociation a1(Uml::AssociationType::Association, &c1, &c2);
    a1.setNameCmd(QStringLiteral("Test assoc"));
    a1.setUMLPackage(&parent);
    QString save = a1.testSave1();
    //a1.testDump("save");
    TestUMLAssociation a2;
    a2.setUMLPackage(&parent);
    QCOMPARE(a2.testLoad1(save), true);
    QCOMPARE(a2.testSave1(), save);
    //a2.testDump("load");
}

QTEST_MAIN(TestAssociation)
