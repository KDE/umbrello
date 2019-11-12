/*
    Copyright 2019  Ralf Habacker  <ralf.habacker@freenet.de>

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

#include "testassociation.h"

// app include
#include "association.h"
#include "package.h"
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlrole.h"

//-----------------------------------------------------------------------------

void TestAssociation::test_equal()
{
    UMLPackage parent("Test Parent");
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
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
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
    UMLAssociation a(Uml::AssociationType::Association, &o1, nullptr);
    QString ar = QString(QLatin1String("objectA: %1 null")).arg(Uml::AssociationType::toStringI18n(Uml::AssociationType::Association));
    QCOMPARE(a.toString(), ar);
    UMLAssociation b(Uml::AssociationType::Association, nullptr, &o2);
    QString br = QString(QLatin1String("null %1 objectB:")).arg(Uml::AssociationType::toStringI18n(Uml::AssociationType::Association));
    QCOMPARE(b.toString(), br);
    UMLAssociation c(Uml::AssociationType::Association, &o1, &o2);
    QString cr = QString(QLatin1String("objectA: %1 objectB:")).arg(Uml::AssociationType::toStringI18n(Uml::AssociationType::Association));
    QCOMPARE(c.toString(), cr);
}

void TestAssociation::test_UMLRole()
{
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getUMLRole(Uml::RoleType::A)->object(), &o1);
    QCOMPARE(a.getUMLRole(Uml::RoleType::B)->object(), &o2);
}

void TestAssociation::test_associationType()
{
    UMLAssociation a(Uml::AssociationType::Association);
    QCOMPARE(a.getAssocType(), Uml::AssociationType::Association);
    a.setAssociationType(Uml::AssociationType::Aggregation);
    QCOMPARE(a.getAssocType(), Uml::AssociationType::Aggregation);
}

void TestAssociation::test_objectID()
{
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getObjectId(Uml::RoleType::A), o1.id());
    QCOMPARE(a.getObjectId(Uml::RoleType::B), o2.id());
}

void TestAssociation::test_visibility()
{
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
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
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
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
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getMultiplicity(Uml::RoleType::A), QString());
    QCOMPARE(a.getMultiplicity(Uml::RoleType::B), QString());
    a.setMultiplicity("1", Uml::RoleType::A);
    a.setMultiplicity("2", Uml::RoleType::B);
    QCOMPARE(a.getMultiplicity(Uml::RoleType::A), QLatin1String("1"));
    QCOMPARE(a.getMultiplicity(Uml::RoleType::B), QLatin1String("2"));
}

void TestAssociation::test_roleName()
{
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getRoleName(Uml::RoleType::A), QString());
    QCOMPARE(a.getRoleName(Uml::RoleType::B), QString());
    a.setRoleName("test1", Uml::RoleType::A);
    a.setRoleName("test2", Uml::RoleType::B);
    QCOMPARE(a.getRoleName(Uml::RoleType::A), QLatin1String("test1"));
    QCOMPARE(a.getRoleName(Uml::RoleType::B), QLatin1String("test2"));
}

void TestAssociation::test_roleDoc()
{
    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    QCOMPARE(a.getRoleDoc(Uml::RoleType::A), QString());
    QCOMPARE(a.getRoleDoc(Uml::RoleType::B), QString());
    a.setRoleDoc("test1", Uml::RoleType::A);
    a.setRoleDoc("test2", Uml::RoleType::B);
    QCOMPARE(a.getRoleDoc(Uml::RoleType::A), QLatin1String("test1"));
    QCOMPARE(a.getRoleDoc(Uml::RoleType::B), QLatin1String("test2"));
}

void TestAssociation::resolveRef()
{
    UMLPackage parent("Test Parent");
    UMLStereotype *stereotype1 = UMLApp::app()->document()->createStereotype("test1");
    UMLStereotype *stereotype2 = UMLApp::app()->document()->createStereotype("test2");

    UMLObject o1(nullptr, "objectA");
    UMLObject o2(nullptr, "objectB");
    UMLAssociation a(Uml::AssociationType::Association, &o1, &o2);
    // no resolve
    a.setUMLPackage(&parent);
    QCOMPARE(a.resolveRef(), true);

    // secondary
    a.getUMLRole(Uml::RoleType::A)->setSecondaryId(Uml::ID::toString(stereotype1->id()));
    a.getUMLRole(Uml::RoleType::B)->setSecondaryId(Uml::ID::toString(stereotype2->id()));
    QCOMPARE(a.resolveRef(), true);

    // secondary fallback
    a.getUMLRole(Uml::RoleType::A)->setSecondaryId(QLatin1String(""));
    a.getUMLRole(Uml::RoleType::A)->setSecondaryFallback(Uml::ID::toString(stereotype1->id()));
    a.getUMLRole(Uml::RoleType::B)->setSecondaryId(QLatin1String(""));
    a.getUMLRole(Uml::RoleType::B)->setSecondaryFallback(Uml::ID::toString(stereotype2->id()));
    QCOMPARE(a.resolveRef(), true);
}

QTEST_MAIN(TestAssociation)
