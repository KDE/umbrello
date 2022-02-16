/* This file is part of KDevelop

    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>
    SPDX-FileCopyrightText: 2011 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchain_multiplefiles.h"

#include <QtTest/QtTest>

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <interfaces/ilanguagecontroller.h>
#include <language/backgroundparser/backgroundparser.h>
#include <tests/testproject.h>
#include <tests/testfile.h>
#include <language/duchain/declaration.h>
#include <language/duchain/problem.h>

QTEST_KDEMAIN(Php::TestDUChainMultipleFiles, GUI)

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
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    TestFile f1("<? function foo() {}", "php", project);
    f1.parse(features);
    QVERIFY(f1.waitForParsed());

    TestFile f2("<? foo();", "php", project);
    f2.parse(features);
    f2.waitForParsed();

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f1.topContext());
    QVERIFY(f2.topContext());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));
}

void TestDUChainMultipleFiles::testImportsBaseClassNotYetParsed()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->clearProjects();
    m_projectController->addProject(project);


    TestFile f2("<? class B extends A {}", "php", project);
    f2.parse(features);

    TestFile f1("<? class A {}", "php", project);
    f1.parse(features, 100); //low priority, to make sure f2 is parsed first

    f1.waitForParsed();
    QTest::qWait(100);

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testNonExistingBaseClass()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    TestFile f1("<? class B extends A {}", "php", project);
    f1.parse(features);
    f1.waitForParsed();

    //there must not be a re-enqueued parsejob
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testImportsGlobalFunctionNotYetParsed()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    TestFile f2("<? foo2();", "php", project);
    f2.parse(features);

    TestFile f1("<? function foo2() {}", "php", project);
    f1.parse(features, 100); //low priority, to make sure f2 is parsed first

    f2.waitForParsed();
    QTest::qWait(100);

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));

}

void TestDUChainMultipleFiles::testNonExistingGlobalFunction()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    TestFile f2("<? foo3();", "php", project);
    f2.parse(features);

    f2.waitForParsed();
     //there must not be a re-enqueued parsejob
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testImportsStaticFunctionNotYetParsed()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    TestFile f2("<? C::foo();", "php", project);
    f2.parse(features);

    TestFile f1("<? class C { public static function foo() {} }", "php", project);
    f1.parse(features, 100); //low priority, to make sure f2 is parsed first

    f2.waitForParsed();
    QTest::qWait(100);

    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(f2.topContext()->imports(f1.topContext(), CursorInRevision(0, 0)));
}

void TestDUChainMultipleFiles::testNonExistingStaticFunction()
{
    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    TestFile f2("<? D::foo();", "php", project);
    f2.parse(features);

    f2.waitForParsed();
     //there must not be a re-enqueued parsejob
    QVERIFY(ICore::self()->languageController()->backgroundParser()->queuedCount() == 0);
}

void TestDUChainMultipleFiles::testForeachImportedIdentifier()
{
    // see https://bugs.kde.org/show_bug.cgi?id=269369

    TopDUContext::Features features = TopDUContext::VisibleDeclarationsAndContexts;

    TestProject* project = new TestProject;
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    // build dependency
    TestFile f1("<? class SomeIterator implements Countable, Iterator { }", "php", project);
    f1.parse(features);
    f1.waitForParsed();

    TestFile f2("<?\n"
                "class A {\n"
                "  public function foo() { $i = $this->bar(); foreach($i as $a => $b) {} } \n"
                "  public function bar() { $a = new SomeIterator(); return $a; }\n"
                " }\n", "php", project);

    for(int i = 0; i < 2; ++i) {
        if (i > 0) {
            features = static_cast<TopDUContext::Features>(features | TopDUContext::ForceUpdate);
        }
        f2.parse(features);
        f2.waitForParsed();
        QTest::qWait(100);

        DUChainWriteLocker lock(DUChain::lock());
        DUContext* ACtx = f2.topContext()->childContexts().first();
        QVERIFY(ACtx);
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
    m_projectController->clearProjects();
    m_projectController->addProject(project);

    TestFile f("<?\n$k = null;\nforeach(array() as $i => $k) {}\n", "php", project);

    f.parse(features);
    f.waitForParsed();
    QVERIFY(f.topContext());

    {
        DUChainWriteLocker lock;
        QVERIFY(f.topContext()->problems().isEmpty());
        QCOMPARE(f.topContext()->findDeclarations(Identifier("k")).count(), 1);
        Declaration* kDec = f.topContext()->findDeclarations(Identifier("k")).first();
        QCOMPARE(kDec->rangeInCurrentRevision().start.line, 1);
        QCOMPARE(kDec->rangeInCurrentRevision().start.column, 0);
        QCOMPARE(kDec->uses().count(), 1);
        QCOMPARE(kDec->uses().begin()->count(), 1);
        QCOMPARE(kDec->uses().begin()->begin()->start.line, 2);
    }

    // delete $k = null; line
    f.setFileContents("<?\nforeach(array() as $i => $k) {}\n");
    f.parse(static_cast<TopDUContext::Features>(features | TopDUContext::ForceUpdate));
    f.waitForParsed();
    QVERIFY(f.topContext());

    {
        DUChainWriteLocker lock;
        QVERIFY(f.topContext()->problems().isEmpty());
        QCOMPARE(f.topContext()->findDeclarations(Identifier("k")).count(), 1);
        Declaration* kDec = f.topContext()->findDeclarations(Identifier("k")).first();
        QCOMPARE(kDec->rangeInCurrentRevision().start.line, 1);
        QCOMPARE(kDec->rangeInCurrentRevision().start.column, 25);
        QCOMPARE(kDec->uses().count(), 0);
    }
}

#include "duchain_multiplefiles.moc"
