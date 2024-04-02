/* This file is part of KDevelop

    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchain_multiplefiles.h"

#include <QtTest>

#include <tests/testcore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>
#include <language/backgroundparser/backgroundparser.h>
#include <tests/testproject.h>
#include <tests/testfile.h>
#include <language/duchain/declaration.h>
#include <language/duchain/problem.h>
#include <language/duchain/types/integraltype.h>

QTEST_MAIN(Php::TestDUChainMultipleFiles)

using namespace KDevelop;
using namespace Php;

void TestDUChainMultipleFiles::initTestCase()
{
    DUChainTestBase::initTestCase();
    TestCore* core = dynamic_cast<TestCore*>(ICore::self());
    Q_ASSERT(core);
    m_projectController = new TestProjectController(core);
    core->setProjectController(m_projectController);
}

void TestDUChainMultipleFiles::testImportsGlobalFunction()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f1(QStringLiteral("<? function foo() {}"), QStringLiteral("php"), project);
    f1.parse(features);
    QVERIFY(f1.waitForParsed());

    TestFile f2(QStringLiteral("<? foo();"), QStringLiteral("php"), project);
    f2.parse(features);
    QVERIFY(f2.waitForParsed());

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f1.topContext());
    QVERIFY(f2.topContext());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));
}

void TestDUChainMultipleFiles::testImportsBaseClassNotYetParsed()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f2(QStringLiteral("<? class B extends A {}"), QStringLiteral("php"), project);
    f2.parse(features);

    TestFile f1(QStringLiteral("<? class A {}"), QStringLiteral("php"), project);
    f1.parse(features, 100); //low priority, to make sure f2 is parsed first

    QVERIFY(f1.waitForParsed());
    QTest::qWait(100);

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testNonExistingBaseClass()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f1(QStringLiteral("<? class B extends A {}"), QStringLiteral("php"), project);
    f1.parse(features);
    QVERIFY(f1.waitForParsed());

    //there must not be a re-enqueued parsejob
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testImportsGlobalFunctionNotYetParsed()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f2(QStringLiteral("<? foo2();"), QStringLiteral("php"), project);
    f2.parse(features);

    TestFile f1(QStringLiteral("<? function foo2() {}"), QStringLiteral("php"), project);
    f1.parse(features, 100); //low priority, to make sure f2 is parsed first

    QVERIFY(f2.waitForParsed());
    QTest::qWait(100);

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));

}

void TestDUChainMultipleFiles::testNonExistingGlobalFunction()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f2(QStringLiteral("<? foo3();"), QStringLiteral("php"), project);
    f2.parse(features);

    QVERIFY(f2.waitForParsed());
     //there must not be a re-enqueued parsejob
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testImportsStaticFunctionNotYetParsed()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f2(QStringLiteral("<? C::foo();"), QStringLiteral("php"), project);
    f2.parse(features);

    TestFile f1(QStringLiteral("<? class C { public static function foo() {} }"), QStringLiteral("php"), project);
    f1.parse(features, 100); //low priority, to make sure f2 is parsed first

    QVERIFY(f2.waitForParsed());
    QTest::qWait(100);

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));
}

void TestDUChainMultipleFiles::testNonExistingStaticFunction()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f2(QStringLiteral("<? D::foo();"), QStringLiteral("php"), project);
    f2.parse(features);

    QVERIFY(f2.waitForParsed());
     //there must not be a re-enqueued parsejob
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testForeachImportedIdentifier()
{
    // see https://bugs.kde.org/show_bug.cgi?id=269369

    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    // build dependency
    TestFile f1(QStringLiteral("<? class SomeIterator implements Countable, Iterator { }"), QStringLiteral("php"), project);
    f1.parse(features);
    QVERIFY(f1.waitForParsed());

    TestFile f2(QStringLiteral("<?\n"
                "class A {\n"
                "  public function foo() { $i = $this->bar(); Q_FOREACH($i as $a => $b) {} } \n"
                "  public function bar() { $a = new SomeIterator(); return $a; }\n"
                " }\n"), QStringLiteral("php"), project);

    for(int i = 0; i < 2; ++i) {
        if (i > 0) {
            features = static_cast<TopDUContext::Features>(features | TopDUContext::ForceUpdate);
        }
        f2.parse(features);
        QVERIFY(f2.waitForParsed());
        QTest::qWait(100);

        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(f2.topContext()->childContexts().size(), 1);
        DUContext* ACtx = f2.topContext()->childContexts().first();
        QVERIFY(ACtx);
        QCOMPARE(ACtx->childContexts().size(), 4);
        Declaration* iDec = ACtx->childContexts().at(1)->localDeclarations().first();
        QVERIFY(iDec);
        Declaration* SomeIteratorDec = f1.topContext()->localDeclarations().first();
        QVERIFY(SomeIteratorDec);
        if (i == 0) {
            QEXPECT_FAIL("", "needs a full two-pass (i.e. get rid of PreDeclarationBuilder)", Continue);
        }
        QVERIFY(iDec->abstractType()->equals(SomeIteratorDec->abstractType().constData()));
        QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));
    }
}

void TestDUChainMultipleFiles::testUpdateForeach()
{
    TopDUContext::Features features = TopDUContext::AllDeclarationsContextsAndUses;

    TestProject* project = new TestProject;
    m_projectController->closeAllProjects();
    m_projectController->addProject(project);

    TestFile f(QStringLiteral("<?\n$k = null;\nQ_FOREACH(array() as $i => $k) {}\n"), QStringLiteral("php"), project);

    f.parse(features);
    QVERIFY(f.waitForParsed());
    QVERIFY(f.topContext());

    {
        DUChainWriteLocker lock;
        QVERIFY(f.topContext()->problems().isEmpty());
        QCOMPARE(f.topContext()->findDeclarations(Identifier("k")).count(), 1);
        Declaration* kDec = f.topContext()->findDeclarations(Identifier(QStringLiteral("k"))).first();
        QCOMPARE(kDec->rangeInCurrentRevision().start().line(), 1);
        QCOMPARE(kDec->rangeInCurrentRevision().start().column(), 0);
        QCOMPARE(kDec->uses().count(), 1);
        QCOMPARE(kDec->uses().begin()->count(), 1);
        QCOMPARE(kDec->uses().begin()->begin()->start.line, 2);
    }

    // delete $k = null; line
    f.setFileContents(QStringLiteral("<?\nQ_FOREACH(array() as $i => $k) {}\n"));
    f.parse(static_cast<TopDUContext::Features>(features | TopDUContext::ForceUpdate));
    QVERIFY(f.waitForParsed());
    QVERIFY(f.topContext());

    {
        DUChainWriteLocker lock;
        QVERIFY(f.topContext()->problems().isEmpty());
        QCOMPARE(f.topContext()->findDeclarations(Identifier("k")).count(), 1);
        Declaration* kDec = f.topContext()->findDeclarations(Identifier(QStringLiteral("k"))).first();
        QCOMPARE(kDec->rangeInCurrentRevision().start().line(), 1);
        QCOMPARE(kDec->rangeInCurrentRevision().start().column(), 25);
        QCOMPARE(kDec->uses().count(), 0);
    }
}

void TestDUChainMultipleFiles::testTodoExtractorReparse()
{
    TestFile file(QStringLiteral("<?php\n$foo = new bar();\n// TODO\n$foo->baz();"), QStringLiteral("php"));

    QVERIFY(KDevelop::ICore::self()->languageController()->completionSettings()->todoMarkerWords().contains("TODO"));

    auto features = TopDUContext::AllDeclarationsContextsAndUses;

    for (int i = 0; i < 2; ++i) {
        if (i == 1) {
            file.setFileContents(QStringLiteral("<?php\n$foo = new bar();\n// TODO\n$foo->asdf();"));
            features = static_cast<TopDUContext::Features>(features | TopDUContext::ForceUpdate);
        }

        file.parse(features);
        QVERIFY(file.waitForParsed());

        DUChainReadLocker lock;
        auto top = file.topContext();
        QVERIFY(top);
        QCOMPARE(top->problems().size(), 1);
        QCOMPARE(top->problems().at(0)->description(), QString("TODO"));
        QCOMPARE(top->problems().at(0)->range(), RangeInRevision(2, 3, 2, 7));
    }
}

void TestDUChainMultipleFiles::testIteratorForeachReparse() {
    TestFile file(QStringLiteral("<?php\n/*\n\n/*\n*/\nQ_FOREACH(new A() as $a) {}\nclass A implements Iterator {\npublic function current() { return 0; }\n}"), QStringLiteral("php"));

    auto features = TopDUContext::AllDeclarationsAndContexts;

    for (int i = 0; i < 2; ++i) {
        if (i == 1) {
            file.setFileContents(QStringLiteral("<?php\n/*\n*/\n\n/*\n*/\nQ_FOREACH(new A() as $a) {}\nclass A implements Iterator {\npublic function current() { return 0; }\n}"));
            features = static_cast<TopDUContext::Features>(features | TopDUContext::ForceUpdate);
        }

        file.parse(features);
        QVERIFY(file.waitForParsed());

        DUChainReadLocker lock;
        auto top = file.topContext();
        QVERIFY(top);
        QVERIFY(top->localDeclarations().size() == 2);
        QCOMPARE(top->localDeclarations().at(0)->qualifiedIdentifier(), QualifiedIdentifier("a"));

        IntegralType::Ptr type = top->localDeclarations().at(0)->type<IntegralType>();
        QVERIFY(type);
        //Should actually parse as an TypeInt, but this does not work.
        QVERIFY(type->dataType() == IntegralType::TypeMixed);
    }
}
