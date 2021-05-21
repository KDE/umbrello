/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testpackage.h"

#include "classifier.h"
#include "package.h"
#include "stereotype.h"

void TestPackage::test_appendClassesAndInterfaces()
{
    UMLPackage p("package");
    UMLClassifier c1("class A");
    UMLClassifier c2("class B");
    UMLClassifier c3("class C");
    p.addObject(&c1);
    p.addObject(&c2);
    p.addObject(&c3);
    QCOMPARE(p.containedObjects().size(), 3);
    UMLClassifierList items;
    p.appendClassesAndInterfaces(items);
    QCOMPARE(items.size(), 3);
}

typedef TestUML<UMLPackage, const QString &> TestUMLPackage;

void TestPackage::test_saveAndLoad()
{
    UMLPackage parent("parent");
    TestUMLPackage p1("Package");
    p1.setUMLPackage(&parent);
    UMLClassifier c1("Test A");
    c1.setUMLPackage(&p1);
    UMLClassifier c2("Test B");
    c2.setUMLPackage(&p1);
    p1.addObject(&c1);
    p1.addObject(&c2);
    p1.setStereotypeCmd("test");
    QString save = p1.testSave1();
    //p1.testDump("save");
    TestUMLPackage p2;
    p2.setUMLPackage(&parent);
    QCOMPARE(p2.testLoad1(save), true);
    //p2.testDump("load");
    QCOMPARE(p2.testSave1(), save);
}

QTEST_MAIN(TestPackage)
