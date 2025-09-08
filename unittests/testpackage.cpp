/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testpackage.h"

#include "umlclassifier.h"
#include "umlpackage.h"
#include "stereotype.h"

void TestPackage::test_appendClassesAndInterfaces()
{
    UMLPackage p(QStringLiteral("package"));
    UMLClassifier c1(QStringLiteral("class A"));
    UMLClassifier c2(QStringLiteral("class B"));
    UMLClassifier c3(QStringLiteral("class C"));
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
    UMLPackage parent(QStringLiteral("parent"));
    TestUMLPackage p1(QStringLiteral("Package"));
    p1.setUMLPackage(&parent);
    UMLClassifier c1(QStringLiteral("Test A"));
    c1.setUMLPackage(&p1);
    UMLClassifier c2(QStringLiteral("Test B"));
    c2.setUMLPackage(&p1);
    p1.addObject(&c1);
    p1.addObject(&c2);
    p1.setStereotypeCmd(QStringLiteral("test"));
    QString save = p1.testSave1();
    //p1.testDump("save");
    TestUMLPackage p2;
    p2.setUMLPackage(&parent);
    QCOMPARE(p2.testLoad1(save), true);
    //p2.testDump("load");
    QCOMPARE(p2.testSave1(), save);
}

QTEST_MAIN(TestPackage)
