/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "test_completion.h"

#include <QtTest/QtTest>

#include <language/duchain/parsingenvironment.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/codemodel.h>
#include <language/codecompletion/codecompletiontesthelper.h>
#include <language/duchain/types/alltypes.h>
#include "duchain/types/structuretype.h"

#include "completion/context.h"
#include "completion/item.h"
#include "completion/helpers.h"
#include "completion/model.h"
#include "duchain/declarations/functiondeclaration.h"


using namespace KTextEditor;
using namespace KDevelop;

QTEST_MAIN(Php::TestCompletion)

namespace Php
{

class TestCodeCompletionModel : public CodeCompletionModel
{
public:
    using CodeCompletionModel::foundDeclarations;
    //normally set by worker, but in test we don't have a worker
    void foundDeclarations(QList<KDevelop::CompletionTreeItemPointer> items, CodeCompletionContext* completionContext)
    {
        m_completionItems.clear();
        foreach(const CompletionTreeItemPointer &i, items) {
            m_completionItems << KSharedPtr<CompletionTreeElement>::staticCast(i);
        }
        m_completionContext = KDevelop::CodeCompletionContext::Ptr(completionContext);
        reset();
    }
};

/**
 * declaration of class A with a number of completion items
 *
 * also introduces an instance of class A named $instA;
 */
const QByteArray testClassA(
    "class A {"
    // start non-static
    // public
    " public function pubf() {}"             // at(0)
    " public $pub;"                          // at(1)
    // protected
    " protected function protf() {}"         // at(2)
    " protected $prot;"                      // at(3)
    // private
    " private function privf() {}"           // at(4)
    " private $priv;"                        // at(5)
    // start static
    // public
    " static public function spubf() {}"     // at(6)
    " static public $spub;"                  // at(7)
    // const == static public
    " const c = 0;"                          // at(8)
    // protected
    " static protected function sprotf() {}" // at(9)
    " static protected $sprot;"              // at(10)
    // private
    " static private function sprivf() {}"   // at(11)
    " static private $spriv;"                // at(12)
    "} $instA = new A; "
);

/**
 * declaration of class B which extends class A
 * B has one new public member function
 *
 * also introduces an instance of class B named $instB;
 */
const QByteArray testClassB(
    "class B extends A {"
    "public function __construct(){}" // at(0)
    "} $instB = new B; "
);

class TestCodeCompletionContext : public CodeCompletionContext
{
public:
    TestCodeCompletionContext(KDevelop::DUContextPointer context, const QString& text, const QString& followingText, const CursorInRevision &position, int depth = 0)
        : CodeCompletionContext(context, text, followingText, position, depth) { }
protected:
    QList<QSet<IndexedString> > completionFiles() {
        QList<QSet<IndexedString> > ret;
        QSet<IndexedString> set;
        set << IndexedString("file:///internal/projecttest0");
        set << IndexedString("file:///internal/projecttest1");
        ret << set;
        return ret;
    }
};

typedef CodeCompletionItemTester<TestCodeCompletionContext> BasePhpCompletionTester;

/**
 * Automatically prepent the test string with "<?php " when it does not start with "<?" already-
 * If we would not do that the Tokenizer in the code-completion would not work (always T_INLINE_HTML).
 */
class PhpCompletionTester : public BasePhpCompletionTester
{
public:
    PhpCompletionTester(DUContext* context, QString text = "; ", QString followingText = "", CursorInRevision position = CursorInRevision::invalid())
        : BasePhpCompletionTester(context, text.startsWith("<?") ? text : text.prepend("<?php "), followingText, position)
    {

    }
};

TestCompletion::TestCompletion()
{
}

void TestCompletion::dumpCompletionItems(QList<CompletionTreeItemPointer> items)
{
    kDebug() << items.count() << "completion items:";
    foreach(const CompletionTreeItemPointer &item, items) {
        kDebug() << item->declaration()->toString();
    }
}

void TestCompletion::publicObjectCompletion()
{
    TopDUContext* top = parse("<?php " + testClassA, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "$blah; $instA->");

    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::MemberAccess);

    QCOMPARE(tester.names, QStringList() << "pubf" << "pub");
}
void TestCompletion::publicStaticObjectCompletion()
{
    TopDUContext* top = parse("<?php " + testClassA, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "$blah; A::");

    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::StaticMemberAccess);

    QCOMPARE(tester.names, QStringList() << "spubf" << "$spub" << "c");
}
void TestCompletion::privateObjectCompletion()
{
    TopDUContext* top = parse("<?php " + testClassA, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());


    DUContext* funContext = top->childContexts().first()->localDeclarations().first()->internalContext();
    PhpCompletionTester tester(funContext, "$this->");

    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::MemberAccess);

    QCOMPARE(tester.names, QStringList() << "pubf" << "pub" << "protf" << "prot" << "privf" << "priv");
}
void TestCompletion::privateStaticObjectCompletion()
{
    TopDUContext* top = parse("<?php " + testClassA, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* funContext = top->childContexts().first()->localDeclarations().first()->internalContext();

    {
    PhpCompletionTester tester(funContext, "self::");

    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::StaticMemberAccess);

    QCOMPARE(tester.names, QStringList() << "spubf" << "$spub" << "c" << "sprotf" << "$sprot" << "sprivf" << "$spriv");
    }
    {
    PhpCompletionTester tester(funContext, "static::");

    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::StaticMemberAccess);

    QCOMPARE(tester.names, QStringList() << "spubf" << "$spub" << "c" << "sprotf" << "$sprot" << "sprivf" << "$spriv");
    }
}
void TestCompletion::protectedObjectCompletion()
{
    TopDUContext* top = parse("<?php " + testClassA + testClassB, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* funContext = top->childContexts().at(1)->localDeclarations().first()->internalContext();

    {
        PhpCompletionTester tester(funContext, "$this->");

        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::MemberAccess);

        QCOMPARE(tester.names, QStringList() << "__construct" << "pubf" << "pub" << "protf" << "prot");
    }

    {
        PhpCompletionTester tester(funContext, "");

        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);

        kDebug() << tester.names;

        QVERIFY(tester.names.contains("$this->__construct"));
        QVERIFY(tester.names.contains("$this->pubf"));
        QVERIFY(tester.names.contains("$this->pub"));
        QVERIFY(tester.names.contains("self::spubf"));
        QVERIFY(tester.names.contains("self::$spub"));
        QVERIFY(tester.names.contains("$this->protf"));
        QVERIFY(tester.names.contains("$this->prot"));
        QVERIFY(tester.names.contains("self::sprotf"));
        QVERIFY(tester.names.contains("self::$sprot"));
        QVERIFY(tester.names.contains("self::c"));
        QVERIFY(!tester.names.contains("self::sprivf"));
        QVERIFY(!tester.names.contains("self::$spriv"));
        QVERIFY(!tester.names.contains("$this->privf"));
        QVERIFY(!tester.names.contains("$this->$priv"));
    }
}
void TestCompletion::protectedStaticObjectCompletion()
{
    TopDUContext* top = parse("<?php " + testClassA + testClassB, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* funContext = top->childContexts().at(1)->localDeclarations().first()->internalContext();
    PhpCompletionTester tester(funContext, "self::");

    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::StaticMemberAccess);

    QCOMPARE(tester.names, QStringList() << "spubf" << "$spub" << "c" << "sprotf" << "$sprot");
}

void TestCompletion::methodCall()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public function foo(A $a, $b = null) {} } $i = new A();");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    {
        PhpCompletionTester tester(top, "$blah; $i->foo(");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);
        QVERIFY(tester.completionContext->parentContext());
        QCOMPARE(tester.completionContext->parentContext()->memberAccessOperation(),
                 CodeCompletionContext::FunctionCallAccess);

        CompletionTreeItemPointer item = searchDeclaration(tester.items, top->childContexts().at(0)->localDeclarations().at(0));
        QVERIFY(item);
        NormalDeclarationCompletionItem* item2 = dynamic_cast<NormalDeclarationCompletionItem*>(item.data());

        QString ret;
        createArgumentList(*item2, ret, 0);
        QCOMPARE(ret, QString("(A $a, null $b = null)"));
    }
    {
        PhpCompletionTester tester(top, "blah; $i->foo(new A(), ");
        QVERIFY(searchDeclaration(tester.items, top->childContexts().at(0)->localDeclarations().at(0)));
    }
}

void TestCompletion::functionCall()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $outside = 1; class A {} function foo(A $a, $b = null) {}");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    PhpCompletionTester tester(top, "blah; foo(");
    QVERIFY(tester.completionContext->parentContext());

    QVERIFY(tester.completionContext->parentContext());
    QVERIFY(!tester.completionContext->parentContext()->parentContext());
    QCOMPARE(tester.completionContext->parentContext()->memberAccessOperation(),
             CodeCompletionContext::FunctionCallAccess);

    QVERIFY(searchDeclaration(tester.items, top->localDeclarations().at(1)));
}

void TestCompletion::nestedFunctionCall_data()
{
    QTest::addColumn<QString>("text");

    QTest::newRow("nested") << QString("bar(foo(");
    QTest::newRow("nested prev arg") << QString("bar(1, foo(");
    QTest::newRow("nested prev func call") << QString("bar(foo(1), foo(");
    QTest::newRow("nested prev arg comma") << QString("bar(1, bar(1, ");
    QTest::newRow("nested prev func comma") << QString("bar(1, bar(foo(1), ");
}

void TestCompletion::nestedFunctionCall()
{
    QFETCH(QString, text);

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo($a) {return 1;} function bar($b, $c) {return 2;}");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    PhpCompletionTester tester(top, text);
    QVERIFY(tester.completionContext->parentContext());
    QVERIFY(tester.completionContext->parentContext()->parentContext());
    QVERIFY(!tester.completionContext->parentContext()->parentContext()->parentContext());
    QCOMPARE(tester.completionContext->parentContext()->memberAccessOperation(),
             CodeCompletionContext::FunctionCallAccess);
    QCOMPARE(tester.completionContext->parentContext()->parentContext()->memberAccessOperation(),
             CodeCompletionContext::FunctionCallAccess);

    QVERIFY(searchDeclaration(tester.items, top->localDeclarations().at(0)));
    QVERIFY(searchDeclaration(tester.items, top->localDeclarations().at(1)));
}

void TestCompletion::newObjectFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo.php"), "<?php class Foo { function bar() {} } ");
    DUChainReleaser releaseAddTop(addTop);

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = new Foo(); ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "blah; $a->");
    QCOMPARE(tester.items.count(), 1);
    QCOMPARE(tester.items.first()->declaration().data(), addTop->childContexts().first()->localDeclarations().first());
}

void TestCompletion::constantFromOtherFile()
{
    TopDUContext* addTop = parseAdditionalFile(
        IndexedString("file:///internal/projecttest0"),
        "<?php define('FIND_ME', 1); $dontFindMe = 1; "
    );
    DUChainReleaser releaseAddTop(addTop);

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(addTop->localDeclarations().size(), 2);
    Declaration* findMe = addTop->localDeclarations().first();
    Declaration* dontFindMe = addTop->localDeclarations().last();

    PhpCompletionTester tester(top, "");
    QVERIFY(searchDeclaration(tester.items, findMe));
    QVERIFY(!searchDeclaration(tester.items, dontFindMe));
}

void TestCompletion::baseClass()
{
    QByteArray method("<? class A { public $avar; } class B extends A { public $bvar; } $a = new A(); $b = new B(); ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    {
        PhpCompletionTester tester(top, "$a->");
        QCOMPARE(tester.names, QStringList() << "avar");
    }

    {
        PhpCompletionTester tester(top, "$b->");
        QCOMPARE(tester.names, QStringList() << "bvar" << "avar");
    }
}

void TestCompletion::extendsFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo.php"), "<?php class A { public $avar; } ");
    DUChainReleaser releaseAddTop(addTop);
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B extends A { public $bvar; } $b = new B();");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "$b->");
    QCOMPARE(tester.items.count(), 2);
    QCOMPARE(tester.items.at(1)->declaration().data(), addTop->childContexts().first()->localDeclarations().first());
    QCOMPARE(tester.items.at(0)->declaration().data(), top->childContexts().first()->localDeclarations().first());
}


void TestCompletion::globalClassFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo.php"), "<?php class A { } ");
    DUChainReleaser releaseAddTop(addTop);
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    /*
        PhpCompletionTester tester(top, "new ");
        QVERIFY(searchDeclaration(tester.items, addTop->localDeclarations().first()));
    */
}

void TestCompletion::codeModel()
{
    DUChainWriteLocker lock(DUChain::lock());
    uint count;
    const CodeModelItem* items;

    CodeModel::self().addItem(IndexedString("file:///foo"), QualifiedIdentifier("identifier"), CodeModelItem::Class);

    CodeModel::self().items(IndexedString("file:///foo"), count, items);
    bool found = false;
    for (uint i = 0;i < count;++i) {
        if (items[0].id.identifier() == QualifiedIdentifier("identifier")) {
            found = true;
            QCOMPARE(items[i].kind, CodeModelItem::Class);
        }
    }
    QVERIFY(found);
}

void TestCompletion::projectFileClass()
{
    TopDUContext* addTop = parseAdditionalFile(IndexedString("file:///internal/projecttest0"), "<? class B { function invisible() {} } ");
    DUChainReleaser releaseAddTop(addTop);

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php class foo { function bar() {} }", DumpNone, "file:///internal/projecttest1");
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    {
        // outside of class foo
        PhpCompletionTester tester(top, "<?php ");
        QVERIFY(searchDeclaration(tester.items, addTop->localDeclarations().first()));
    }
    {
        // inside of class foo, i.e. in its bar() method
        PhpCompletionTester tester(top->childContexts().first()->childContexts().first(), "<?php ");

        kDebug() << tester.names;
        // we want to see the class
        QVERIFY(searchDeclaration(tester.items, addTop->localDeclarations().first()));
        // but not its methods
        QVERIFY(!searchDeclaration(tester.items, addTop->childContexts().first()->localDeclarations().first()));
    }
}


void TestCompletion::variable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {  } $a = new A();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "");
    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);

    QVERIFY(searchDeclaration(tester.items, top->localDeclarations().at(1)));
}

void TestCompletion::nameNormalVariable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $abc = 0; $arr = array(); define('def', 0); class ghi {} ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "");
    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);

    foreach(const QString &id, QStringList() << "ghi" << "def" << "$abc" << "$arr") {
        QVERIFY(tester.names.contains(id, Qt::CaseSensitive));
    }
}

void TestCompletion::nameClassMember()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public $abc = 0; } $b = new A;  ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "$b->");

    TestCodeCompletionModel *model = new TestCodeCompletionModel;
    model->foundDeclarations(tester.items, tester.completionContext.data());

    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::MemberAccess);

    CompletionTreeItemPointer itm = searchDeclaration(tester.items, top->childContexts().first()->localDeclarations().first());
    QVERIFY(itm);
    QCOMPARE(itm->data(model->index(0, Php::CodeCompletionModel::Name), Qt::DisplayRole, model).toString(),
             QString("abc"));

    //don't delete model as its constructor does bad things (quit the current thread - we don't want that in test)
    //TODO find better solution that doesn't leak
}

void TestCompletion::exceptions()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class MyExcpt extends Exception {} $excpt = new MyExcpt(); ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    {
        PhpCompletionTester tester(top, "throw ");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::ExceptionInstanceChoose);
        QCOMPARE(tester.items.count(), 1);
        QVERIFY(searchDeclaration(tester.items, top->localDeclarations().at(1)));
    }

    {
        PhpCompletionTester tester(top, "throw new ");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::ExceptionChoose);
        QCOMPARE(tester.items.count(), 2);
        QVERIFY(searchDeclaration(tester.items, top->localDeclarations().at(0)));
    }

    {
        PhpCompletionTester tester(top, "try { } catch(");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::ExceptionChoose);
        QCOMPARE(tester.items.count(), 2);
        QVERIFY(searchDeclaration(tester.items, top->localDeclarations().at(0)));
    }
}

void TestCompletion::exceptionOtherFile()
{
    TopDUContext* addTop = parseAdditionalFile(IndexedString("file:///internal/projecttest0"),
        "<?php class MyExcptOtherFile extends Exception {} class MyClass {}");
    DUChainReleaser releaseAddTop(addTop);
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    {
        PhpCompletionTester tester(top, "throw new ");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::ExceptionChoose);
        QCOMPARE(tester.items.count(), 2);
        QVERIFY(searchDeclaration(tester.items, addTop->localDeclarations().at(0)));
    }

}

void TestCompletion::abstractMethods()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? abstract class A {  abstract function foo(); function bar(){} }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* funContext = top->childContexts().first()->localDeclarations().last()->internalContext();
    PhpCompletionTester tester(funContext, "$this->");
    QCOMPARE(tester.names, QStringList() << "foo" << "bar");
}

void TestCompletion::interfaceMethods()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface A {  function foo(); } class B implements A { function bar(){} }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* funContext = top->childContexts().last()->localDeclarations().first()->internalContext();
    PhpCompletionTester tester(funContext, "$this->");
    QCOMPARE(tester.names, QStringList() << "bar" << "foo");
}

void TestCompletion::interfaceMethods2()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface A {  function foo(); } /** @var A **/ $a = x(); ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* funContext = top;
    PhpCompletionTester tester(funContext, "$a->");
    QCOMPARE(tester.names, QStringList() << "foo");
}

void TestCompletion::implementMethods()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface A { function foo(); } class B implements A {  }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    // context of class B
    DUContext* classContext = top->childContexts().last();
    {
        PhpCompletionTester tester(classContext, "{");
        QStringList compItems;
        compItems << "foo";
        compItems << "const";
        compItems << "final";
        compItems << "function";
        compItems << "public";
        compItems << "private";
        compItems << "protected";
        compItems << "static";
        compItems << "var";
        compItems.sort();
        tester.names.sort();
        QCOMPARE(tester.names, compItems);
    }

    //TODO: verify actual completion text
}

void TestCompletion::overrideMethods()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function a(){} final function b(){}  } class B extends A {  }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    // context of class B
    DUContext* classContext = top->childContexts().last();
    {
        PhpCompletionTester tester(classContext, "{");
        QStringList compItems;
        compItems << "a";
        compItems << "const";
        compItems << "final";
        compItems << "function";
        compItems << "public";
        compItems << "private";
        compItems << "protected";
        compItems << "static";
        compItems << "var";
        compItems.sort();
        tester.names.sort();
        QCOMPARE(tester.names, compItems);
    }
    {
        PhpCompletionTester tester(classContext, "public static");
        QStringList compItems;
        compItems << "final";
        compItems << "function";
        compItems.sort();
        tester.names.sort();
        QCOMPARE(tester.names, compItems);
    }
    {
        PhpCompletionTester tester(classContext, "private function");
        QVERIFY(tester.items.isEmpty());
    }
    {
        PhpCompletionTester tester(classContext, "final public ");
        QStringList compItems;
        compItems << "a";
        compItems << "function";
        compItems << "static";
        compItems.sort();
        tester.names.sort();
        QCOMPARE(tester.names, compItems);
    }

    //TODO: verify actual completion text
}

void TestCompletion::overrideVars()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { protected $x;  } class B extends A {  }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    // context of class B
    DUContext* classContext = top->childContexts().last();
    {
        PhpCompletionTester tester(classContext, "{");
        QStringList compItems;
        compItems << "x";
        compItems << "const";
        compItems << "final";
        compItems << "function";
        compItems << "public";
        compItems << "private";
        compItems << "protected";
        compItems << "static";
        compItems << "var";
        compItems.sort();
        tester.names.sort();
        QCOMPARE(tester.names, compItems);
    }
}

void TestCompletion::inArray()
{
    TopDUContext* top = parse("", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "<?php a = array(1, ");
    QVERIFY(tester.items.count() > 0);

    // TODO: compare to global completion list
}

void TestCompletion::verifyExtendsOrImplements(const QString &codeStr, const QString &completionStr,
        ClassDeclarationData::ClassType type,
        const CursorInRevision& cursor,
        QStringList forbiddenIdentifiers)
{
    if (cursor.isValid()) {
        kDebug() << codeStr.mid(0, cursor.column) + completionStr + '|' + codeStr.mid(cursor.column);
    } else {
        kDebug() << codeStr + completionStr + '|';
    }
    TopDUContext *top = parse(codeStr.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);

    DUContext *ctx;
    if (cursor.isValid()) {
        DUChainWriteLocker lock(DUChain::lock());
        ctx = top->findContextAt(cursor);
        QVERIFY(ctx);
        QVERIFY(ctx->owner());
        QVERIFY(dynamic_cast<ClassDeclaration*>(ctx->owner()));
    } else {
        ctx = top;
    }

    PhpCompletionTester tester(ctx, completionStr);

    QVERIFY(!tester.items.isEmpty());
    // make sure the items are unique
    QCOMPARE(tester.names.size(), tester.names.toSet().size());
    foreach(const CompletionTreeItemPointer &item, tester.items) {
        ClassDeclaration* klass = dynamic_cast<ClassDeclaration*>(item->declaration().data());
        QVERIFY(klass);
        QVERIFY(klass->classModifier() != ClassDeclarationData::Final);
        QCOMPARE(klass->classType(), type);

        if (!forbiddenIdentifiers.isEmpty()) {
            QVERIFY(! forbiddenIdentifiers.contains(item->declaration()->identifier().toString()));
        }
    }
}

void TestCompletion::newExtends()
{
    verifyExtendsOrImplements("<?php ", "class test extends ",
                              ClassDeclarationData::Class,
                              CursorInRevision::invalid(),
                              QStringList() << "test");

    verifyExtendsOrImplements("<?php ", "interface test extends ",
                              ClassDeclarationData::Interface,
                              CursorInRevision::invalid(),
                              QStringList() << "test");

    verifyExtendsOrImplements("<?php interface blub{} ", "interface test extends blub, ",
                              ClassDeclarationData::Interface,
                              CursorInRevision::invalid(),
                              QStringList() << "test" << "blub");
}

void TestCompletion::updateExtends()
{
    //                         0         1         2         3         4         5
    //                         012345678901234567890123456789012345678901234567890123456789
    verifyExtendsOrImplements("<?php class test {}", "class test extends ",
                              ClassDeclarationData::Class,
                              CursorInRevision(0, 16),
                              QStringList() << "test");

    //                         0         1         2         3         4         5
    //                         012345678901234567890123456789012345678901234567890123456789
    verifyExtendsOrImplements("<?php interface test {}", "interface test extends ",
                              ClassDeclarationData::Interface,
                              CursorInRevision(0, 20),
                              QStringList() << "test");

    //                         0         1         2         3         4         5
    //                         012345678901234567890123456789012345678901234567890123456789
    verifyExtendsOrImplements("<?php interface blub{} interface test extends blub {}",
                              "interface test extends blub,bar, ",
                              ClassDeclarationData::Interface,
                              CursorInRevision(0, 50),
                              QStringList() << "test" << "blub");
}

void TestCompletion::newImplements()
{
    verifyExtendsOrImplements("<?php ", "class test implements ",
                              ClassDeclarationData::Interface,
                              CursorInRevision::invalid(),
                              QStringList() << "test");
    verifyExtendsOrImplements("<?php interface blub{}", " class test implements blub, ",
                              ClassDeclarationData::Interface,
                              CursorInRevision::invalid(),
                              QStringList() << "test" << "blub");
}

void TestCompletion::updateImplements()
{
    //                         0         1         2         3         4         5
    //                         012345678901234567890123456789012345678901234567890123456789
    verifyExtendsOrImplements("<?php class test {}", "class test implements ",
                              ClassDeclarationData::Interface,
                              CursorInRevision(0, 16),
                              QStringList() << "test");

    //                         0         1         2         3         4         5
    //                         012345678901234567890123456789012345678901234567890123456789
    verifyExtendsOrImplements("<?php interface blub{} class test implements blub {}",
                              "class test implements blub, ",
                              ClassDeclarationData::Interface,
                              CursorInRevision(0, 49),
                              QStringList() << "test" << "blub");
}

void TestCompletion::avoidCircularInheritance()
{
    verifyExtendsOrImplements("<?php interface blub{} interface bar extends blub{}",
                              "interface test extends bar, ",
                              ClassDeclarationData::Interface,
                              CursorInRevision::invalid(),
                              QStringList() << "test" << "blub" << "bar");

    verifyExtendsOrImplements("<?php interface blub{} interface bar extends blub{}",
                              "class test implements bar, ",
                              ClassDeclarationData::Interface,
                              CursorInRevision::invalid(),
                              QStringList() << "blub" << "bar");
}



void TestCompletion::unsureType()
{
    QByteArray method("<? class A { public $vA; } class B { public $vB; } function foo() { return new A; return new B; } $f = foo(); ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "$f->");
    QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::MemberAccess);

    kDebug() << tester.names;
    foreach(const QString &id, QStringList() << "vA" << "vB") {
        QVERIFY(tester.names.contains(id, Qt::CaseSensitive));
    }
}

void TestCompletion::completionAfterComments()
{
    TopDUContext* top = parse("<?php\n", DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    foreach ( const QString &code, QStringList() << "# asdf\n"
                                    << "// asdf\n"
                                    << "/* */" )
    {
        PhpCompletionTester tester(top, code);

        kDebug() << tester.names;
        QVERIFY(tester.completionContext->isValid());
        QVERIFY(tester.items.count() > 0);
    }

    // TODO: compare to global completion list
}

void TestCompletion::completionInComments()
{
    TopDUContext* top = parse("<?php\n", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    foreach ( const QString &code, QStringList() << "# "
                                    << "// " << "/* " )
    {
        PhpCompletionTester tester(top, code);
        QVERIFY(!tester.completionContext->isValid());
    }
}

void TestCompletion::phpStartTag()
{
    // some context with a function (or anything else for that matter) starting with "php" substring
    TopDUContext* top = parse("<?php function php_test() {} \n", DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    foreach ( const QString &code, QStringList() << "p" << "ph" << "php" ) {
        PhpCompletionTester tester(top, "<?", code);

        QVERIFY(tester.items.isEmpty());
    }

    PhpCompletionTester tester(top, "<?php ");

    QVERIFY(!tester.items.isEmpty());
}

void TestCompletion::outsidePhpContext()
{
    TopDUContext* top = parse("<?php $var = 1; ?>=", DumpDUChain);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "<?php $var = 1; ?>=");

    QVERIFY(tester.items.isEmpty());
}

void TestCompletion::nonGlobalInFunction()
{
    TopDUContext* top = parse("<?php $outside = 1; function test() {}", DumpDUChain);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top->childContexts().first(), "");

    QList<Declaration*> decs = top->findLocalDeclarations(Identifier("outside"));
    QCOMPARE(decs.count(), 1);
    QVERIFY(!searchDeclaration(tester.items, decs.first()));
}

void TestCompletion::fileCompletion()
{
    TopDUContext* top = parse("<?php ", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ///TODO: somehow offer files and check whether they work with relative sub-paths
    ///TODO: make sure items after dirname(__FILE__) or similar start with a /
    foreach ( const QString& code, QStringList() << "include \"" << "include_once \"" << "require_once \""
                                                 << "require \"" << "include ( \""
                                                 << "include dirname(__FILE__) . \"/"
                                                 << "include ( dirname(__FILE__) . \"/"
                                                 << "include '" << "include ( '"
                                                 << "include ( dirname(__FILE__) . '/"
                                                 /** TODO:  << "include __DIR__ . \"/" */ )
    {
        kDebug() << code;
        PhpCompletionTester tester(top, code);
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::FileChoose);
    }
}

void TestCompletion::instanceof()
{
    TopDUContext* top = parse("<?php interface A{} class B{} abstract class C{} final class D{}", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "$a instanceof ");

    foreach ( const QString& name, QStringList() << "a" << "b" << "c" << "d" ) {
        kDebug() << name;
        QList<Declaration*> decs = top->findLocalDeclarations(Identifier(name));
        QCOMPARE(decs.size(), 1);
        ClassDeclaration* cdec = dynamic_cast<ClassDeclaration*>(decs.first());
        QVERIFY(cdec);
        QVERIFY(searchDeclaration(tester.items, cdec));
    }

    foreach ( const CompletionTreeItemPointer &item, tester.items ) {
        QVERIFY(dynamic_cast<ClassDeclaration*>(item->declaration().data()));
    }
}

void TestCompletion::afterFunctionArg()
{
    TopDUContext* top = parse("<?php class A{ var $b; } $a = new A;", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    foreach ( const QString &code, QStringList() << "if ($a->" << "while ($a->" << "foobar($a->" ) {
        kDebug() << code;
        PhpCompletionTester tester(top, code);
        QCOMPARE(tester.names.size(), 1);
        QCOMPARE(tester.names.first(), QString("b"));
    }
}

void TestCompletion::functionBeforeDeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php  function test() {}", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().size(), 1);
    PhpCompletionTester tester(top, "", "", CursorInRevision(0, 3));
    // function _should_ be found
    QVERIFY(searchDeclaration(tester.items, top->localDeclarations().first()));
}

void TestCompletion::classBeforeDeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php  function test() {}", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().size(), 1);
    PhpCompletionTester tester(top, "", "", CursorInRevision(0, 3));
    // class _should_ be found
    QVERIFY(searchDeclaration(tester.items, top->localDeclarations().first()));
}

void TestCompletion::constantBeforeDeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php  define('TEST', 1);", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().size(), 1);
    PhpCompletionTester tester(top, "", "", CursorInRevision(0, 3));
    // constant should _not_ be found
    QVERIFY(!searchDeclaration(tester.items, top->localDeclarations().first()));
}

void TestCompletion::variableBeforeDeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php  $test = 1;", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().size(), 1);
    PhpCompletionTester tester(top, "", "", CursorInRevision(0, 3));
    // variable should _not_ be found
    QVERIFY(!searchDeclaration(tester.items, top->localDeclarations().first()));
}

void TestCompletion::functionArguments()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php function foo($asdf, $bar) {}", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* fDec = top->localDeclarations().first();
    QVERIFY(fDec);
    FunctionType::Ptr fType = fDec->type<FunctionType>();
    QVERIFY(fType);

    // params
    QVector< Declaration* > args = top->childContexts().first()->localDeclarations();
    QCOMPARE(args.size(), 2);

    PhpCompletionTester tester(top->childContexts().last(), "");
    // should get two local and the func itself
    QVERIFY(searchDeclaration(tester.items, fDec));
    foreach( Declaration* dec, args ) {
        kDebug() << dec->toString();
        QVERIFY(searchDeclaration(tester.items, dec));
    }
}

void TestCompletion::referencedClass()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php " + testClassA + " function foo(A &$arg) {}", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->findDeclarations(Identifier("a"));
    QCOMPARE(decs.size(), 1);

    ClassDeclaration* aDec = dynamic_cast<ClassDeclaration*>(decs.first());
    QVERIFY(aDec);

    decs = top->findDeclarations(Identifier("foo"));
    QCOMPARE(decs.size(), 1);

    FunctionDeclaration* funcDec = dynamic_cast<FunctionDeclaration*>(decs.first());
    QVERIFY(funcDec);
    QVERIFY(funcDec->internalContext());
    QVERIFY(funcDec->internalFunctionContext());
    QVERIFY(funcDec->internalContext()->imports(funcDec->internalFunctionContext()));

    PhpCompletionTester tester(funcDec->internalContext(), "$arg->");
    QVERIFY(tester.completionContext->memberAccessOperation() == CodeCompletionContext::MemberAccess);
    QCOMPARE(tester.names, QStringList() << "pubf" << "pub");
}

void TestCompletion::ctorCall()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { /** @param string $bar **/ public function __construct($bar) {} }\n"
                      "class B { /** @param bool $asdf **/ public function B($asdf) {} }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* aCtor = top->childContexts().first()->localDeclarations().first();
    Declaration* bCtor = top->childContexts().last()->localDeclarations().first();

    {
        PhpCompletionTester tester(top, "new A(");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);
        QVERIFY(tester.completionContext->parentContext());
        QCOMPARE(tester.completionContext->parentContext()->memberAccessOperation(),
                 CodeCompletionContext::FunctionCallAccess);

        CompletionTreeItemPointer item = searchDeclaration(tester.items, aCtor);
        QVERIFY(item);
        NormalDeclarationCompletionItem* item2 = dynamic_cast<NormalDeclarationCompletionItem*>(item.data());

        QString ret;
        createArgumentList(*item2, ret, 0);
        QCOMPARE(ret, QString("(string $bar)"));
    }
    {
        PhpCompletionTester tester(top, "new B(");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);
        QVERIFY(tester.completionContext->parentContext());
        QCOMPARE(tester.completionContext->parentContext()->memberAccessOperation(),
                 CodeCompletionContext::FunctionCallAccess);

        CompletionTreeItemPointer item = searchDeclaration(tester.items, bCtor);
        QVERIFY(item);
        NormalDeclarationCompletionItem* item2 = dynamic_cast<NormalDeclarationCompletionItem*>(item.data());

        QString ret;
        createArgumentList(*item2, ret, 0);
        QCOMPARE(ret, QString("(bool $asdf)"));
    }
}

void TestCompletion::chainedCalling()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php class a { function b() { return new a; } } $a = new a;", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    PhpCompletionTester tester(top, "$a->b()->");
    QVERIFY(tester.completionContext->memberAccessOperation() == CodeCompletionContext::MemberAccess);
    QCOMPARE(tester.names, QStringList() << "b");
}

void TestCompletion::funcCallInConditional()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function asdf($a, $b = 1) {}");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    {
        PhpCompletionTester tester(top, "if ( !empty($_POST['answer']) && asdf(");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);
        QVERIFY(tester.completionContext->parentContext());
        QCOMPARE(tester.completionContext->parentContext()->memberAccessOperation(),
                 CodeCompletionContext::FunctionCallAccess);

        CompletionTreeItemPointer item = searchDeclaration(tester.items, top->localDeclarations().at(0));
        QVERIFY(item);
        NormalDeclarationCompletionItem* item2 = dynamic_cast<NormalDeclarationCompletionItem*>(item.data());

        QString ret;
        createArgumentList(*item2, ret, 0);
        QCOMPARE(ret, QString("(mixed $a, int $b = 1)"));
    }
}

void TestCompletion::namespaces()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? namespace foo\\bar {}\n");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    {
        PhpCompletionTester tester(top, "namespace ");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NamespaceChoose);
        QVERIFY(!tester.completionContext->parentContext());

        QCOMPARE(tester.names, QStringList() << "foo");
    }
}

void TestCompletion::inNamespace()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? namespace foo { function bar() {} }\n"
                      "   namespace yxc { function qwe() {} }\n" );

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    {
        PhpCompletionTester tester(top->childContexts().at(0), "");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::NoMemberAccess);
        QVERIFY(!tester.completionContext->parentContext());

        QVERIFY(searchDeclaration(tester.items, top->localDeclarations().first()));
        QVERIFY(searchDeclaration(tester.items, top->childContexts().first()->localDeclarations().first()));
        QVERIFY(searchDeclaration(tester.items, top->localDeclarations().last()));
        QVERIFY(!searchDeclaration(tester.items, top->childContexts().last()->localDeclarations().first()));
    }
    {
        PhpCompletionTester tester(top->childContexts().at(0), "\\");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::BackslashAccess);
        QVERIFY(!tester.completionContext->parentContext());

        QCOMPARE(tester.items.count(), 2);
        QVERIFY(searchDeclaration(tester.items, top->localDeclarations().first()));
        QVERIFY(!searchDeclaration(tester.items, top->childContexts().first()->localDeclarations().first()));
        QVERIFY(searchDeclaration(tester.items, top->localDeclarations().last()));
        QVERIFY(!searchDeclaration(tester.items, top->childContexts().last()->localDeclarations().first()));
    }
    {
        PhpCompletionTester tester(top->childContexts().at(0), "\\foo\\");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::BackslashAccess);
        QVERIFY(!tester.completionContext->parentContext());

        QCOMPARE(tester.items.count(), 1);
        QVERIFY(!searchDeclaration(tester.items, top->localDeclarations().first()));
        QVERIFY(searchDeclaration(tester.items, top->childContexts().first()->localDeclarations().first()));
        QVERIFY(!searchDeclaration(tester.items, top->localDeclarations().last()));
        QVERIFY(!searchDeclaration(tester.items, top->childContexts().last()->localDeclarations().first()));
    }
    {
        PhpCompletionTester tester(top->childContexts().at(0), "\\yxc\\");
        QCOMPARE(tester.completionContext->memberAccessOperation(), CodeCompletionContext::BackslashAccess);
        QVERIFY(!tester.completionContext->parentContext());

        QCOMPARE(tester.items.count(), 1);
        QVERIFY(!searchDeclaration(tester.items, top->localDeclarations().first()));
        QVERIFY(!searchDeclaration(tester.items, top->childContexts().first()->localDeclarations().first()));
        QVERIFY(!searchDeclaration(tester.items, top->localDeclarations().last()));
        QVERIFY(searchDeclaration(tester.items, top->childContexts().last()->localDeclarations().first()));
    }
}

void TestCompletion::closures()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $l = function($a) {};\n" );

    TopDUContext* top = parse(method, DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    Declaration* l = top->localDeclarations().first();
    Declaration* c = top->localDeclarations().last();
    {
        PhpCompletionTester tester(top, "");
        QVERIFY(tester.containsDeclaration(l));
        QVERIFY(!tester.containsDeclaration(c));
    }
    {
        PhpCompletionTester tester(top, "$l(");
        QVERIFY(tester.containsDeclaration(l));
        QVERIFY(!tester.containsDeclaration(c));

        QVERIFY(tester.completionContext->parentContext());
        QVERIFY(!tester.completionContext->parentContext()->parentContext());
        QCOMPARE(tester.completionContext->parentContext()->memberAccessOperation(),
                 CodeCompletionContext::FunctionCallAccess);
    }
}

}

#include "test_completion.moc"

