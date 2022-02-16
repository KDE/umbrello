/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testumlcanvasobject.h"

// app include
#include "association.h"
#include "classifier.h"
#include "umlcanvasobject.h"

//-----------------------------------------------------------------------------
void TestUMLCanvasObject::test_addAssociationEnd()
{
    UMLCanvasObject c1("Test A");
    UMLCanvasObject c2("Test B");
    UMLAssociation a(Uml::AssociationType::Generalization, &c1, &c2);
    c1.addAssociationEnd(&a);
    c2.addAssociationEnd(&a);
    QCOMPARE(c1.associations(), 1);
    QCOMPARE(c2.associations(), 1);
    c1.addAssociationEnd(&a);
    c2.addAssociationEnd(&a);
    QCOMPARE(c1.associations(), 1);
    QCOMPARE(c2.associations(), 1);
}

void TestUMLCanvasObject::test_getAssociations()
{
    UMLCanvasObject c1("Test A");
    UMLCanvasObject c2("Test B");
    UMLAssociation a(Uml::AssociationType::Generalization, &c1, &c2);
    c1.addAssociationEnd(&a);
    c2.addAssociationEnd(&a);
    QCOMPARE(c1.hasAssociation(&a), true);
    QCOMPARE(c2.hasAssociation(&a), true);
    c1.removeAssociationEnd(&a);
    c2.removeAssociationEnd(&a);
    QCOMPARE(c1.hasAssociation(&a), false);
    QCOMPARE(c2.hasAssociation(&a), false);
}

void TestUMLCanvasObject::test_removeAllAssociationEnds()
{
    UMLCanvasObject c1("Test A");
    UMLCanvasObject c2("Test B");
    UMLAssociation a(Uml::AssociationType::Generalization, &c1, &c2);
    c1.addAssociationEnd(&a);
    UMLAssociation b(Uml::AssociationType::Association, &c1, &c2);
    c1.addAssociationEnd(&b);
    QCOMPARE(c1.associations(), 2);
    c1.removeAllAssociationEnds();
    QCOMPARE(c1.associations(), 0);
}

void TestUMLCanvasObject::test_getSuperClasses()
{
    UMLCanvasObject o1("Test Sup o");
    UMLCanvasObject o2("Test Super o");
    UMLAssociation a1(Uml::AssociationType::Generalization, &o1, &o2);
    o1.addAssociationEnd(&a1);
    o2.addAssociationEnd(&a1);
    QCOMPARE(o1.getSuperClasses(false).size(), 0);
    QCOMPARE(o1.getSubClasses().size(), 0);
    QCOMPARE(o2.getSuperClasses().size(), 0);
    QCOMPARE(o2.getSubClasses().size(), 0);

    UMLClassifier c1("Test Sup c");
    UMLClassifier c2("Test Super c");
    UMLAssociation a2(Uml::AssociationType::Generalization, &c1, &c2);
    c1.addAssociationEnd(&a2);
    c2.addAssociationEnd(&a2);
    QCOMPARE(c1.getSuperClasses(false).size(), 1);
    QCOMPARE(c1.getSubClasses().size(), 0);
    QCOMPARE(c2.getSuperClasses().size(), 0);
    QCOMPARE(c2.getSubClasses().size(), 1);

    UMLAssociation a3(Uml::AssociationType::Realization, &c1, &c2);
    c1.addAssociationEnd(&a3);
    c2.addAssociationEnd(&a3);
    QCOMPARE(c1.getSuperClasses(false).size(), 1);
    QCOMPARE(c1.getSuperClasses(true).size(), 2);
    QCOMPARE(c1.getSubClasses().size(), 0);
    QCOMPARE(c2.getSuperClasses().size(), 0);
    QCOMPARE(c2.getSubClasses().size(), 2);
}

void TestUMLCanvasObject::test_getRealizations()
{
    UMLClassifier c1("Test A");
    UMLClassifier c2("Test B");
    UMLAssociation a1(Uml::AssociationType::Realization, &c1, &c2);
    UMLAssociation a2(Uml::AssociationType::Association, &c1, &c2);
    c1.addAssociationEnd(&a1);
    c2.addAssociationEnd(&a1);
    QCOMPARE(c1.getRealizations().size(), 1);
    c1.addAssociationEnd(&a2);
    c2.addAssociationEnd(&a2);
    QCOMPARE(c1.getRealizations().size(), 1);
    c1.removeAssociationEnd(&a2);
    c2.removeAssociationEnd(&a2);
    QCOMPARE(c1.getRealizations().size(), 1);
}

void TestUMLCanvasObject::test_getAggregations()
{
    UMLClassifier c1("Test A");
    UMLClassifier c2("Test B");
    UMLAssociation a1(Uml::AssociationType::Aggregation, &c1, &c2);
    c1.addAssociationEnd(&a1);
    c2.addAssociationEnd(&a1);
    QCOMPARE(c1.getAggregations().size(), 1);
    UMLAssociation a2(Uml::AssociationType::Association, &c1, &c2);
    c1.addAssociationEnd(&a2);
    c2.addAssociationEnd(&a2);
    QCOMPARE(c1.getAggregations().size(), 1);
    c1.removeAssociationEnd(&a1);
    c2.removeAssociationEnd(&a1);
    QCOMPARE(c1.getAggregations().size(), 0);
}

void TestUMLCanvasObject::test_getCompositions()
{
    UMLClassifier c1("Test A");
    UMLClassifier c2("Test B");
    UMLAssociation a1(Uml::AssociationType::Composition, &c1, &c2);
    c1.addAssociationEnd(&a1);
    c2.addAssociationEnd(&a1);
    QCOMPARE(c1.getCompositions().size(), 1);
    UMLAssociation a2(Uml::AssociationType::Association, &c1, &c2);
    c1.addAssociationEnd(&a2);
    c2.addAssociationEnd(&a2);
    QCOMPARE(c1.getCompositions().size(), 1);
    c1.removeAssociationEnd(&a1);
    c2.removeAssociationEnd(&a1);
    QCOMPARE(c1.getCompositions().size(), 0);
}

void TestUMLCanvasObject::test_getRelationships()
{
    UMLClassifier c1("Test A");
    UMLClassifier c2("Test B");
    UMLAssociation a1(Uml::AssociationType::Relationship, &c1, &c2);
    c1.addAssociationEnd(&a1);
    c2.addAssociationEnd(&a1);
    QCOMPARE(c1.getRelationships().size(), 1);
    UMLAssociation a2(Uml::AssociationType::Association, &c1, &c2);
    c1.addAssociationEnd(&a2);
    c2.addAssociationEnd(&a2);
    QCOMPARE(c1.getRelationships().size(), 1);
    c1.removeAssociationEnd(&a1);
    c2.removeAssociationEnd(&a1);
    QCOMPARE(c1.getRelationships().size(), 0);
}

QTEST_MAIN(TestUMLCanvasObject)
