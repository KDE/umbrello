/*
    SPDX-FileCopyrightText: 2019 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testpreconditionwidget.h"
#include "preconditionwidget.h"

#include "umlfolder.h"
#include "objectwidget.h"
#include "umlscene.h"

#include <QXmlStreamWriter>

typedef TestWidget<PreconditionWidget, ObjectWidget*> TestPreconditionWidgetClass;

void TestPreconditionWidget::test_saveAndLoad()
{
    UMLFolder folder(QStringLiteral("testfolder"));
    UMLScene scene(&folder);
    UMLObject o(nullptr);
    ObjectWidget ow(&scene, &o);
    scene.addWidgetCmd(&ow);
    TestPreconditionWidgetClass pw1(&scene, &ow);
    scene.addWidgetCmd(&pw1);
    QString save = pw1.testSave1();
    //pw1.testDump("save");
    TestPreconditionWidgetClass pw2(&scene, nullptr);
    QCOMPARE(pw2.testLoad1(save), true);
    QCOMPARE(pw2.testSave1(), save);
    //pw2.testDump("load");
    QCOMPARE(pw2.objectWidget(), &ow);
}

QTEST_MAIN(TestPreconditionWidget)

