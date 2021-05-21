/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "expressionparser.h"

#include <QtTest/QtTest>

#include <language/duchain/parsingenvironment.h>
#include <language/duchain/problem.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/declaration.h>

#include "../types/structuretype.h"
#include "../expressionparser.h"

using namespace KDevelop;

QTEST_MAIN(Php::TestExpressionParser)

namespace Php
{

TestExpressionParser::TestExpressionParser()
{
}


void TestExpressionParser::newClass()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function foo() {} } $i = new A();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType(QByteArray("$i"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(res.type());
    QCOMPARE(StructureType::Ptr::staticCast(res.type())->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestExpressionParser::newSelf()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function self() {  } }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType( QByteArray("new self()"),
                                        DUContextPointer(top->childContexts().first()->childContexts().last()),
                                        CursorInRevision(0, 30));
    QVERIFY(res.type());
    QCOMPARE(StructureType::Ptr::staticCast(res.type())->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestExpressionParser::newStatic()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function self() {  } }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType( QByteArray("new static()"),
                                        DUContextPointer(top->childContexts().first()->childContexts().last()),
                                        CursorInRevision(0, 30));
    QVERIFY(res.type().cast<StructureType>());
    QCOMPARE(res.type().cast<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestExpressionParser::memberVariable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { /** @var A **/ public $foo; } $i = new A();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType(QByteArray("$i->foo"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(res.type());
    QCOMPARE(res.allDeclarations().count(), 1);
    QCOMPARE(res.allDeclarations().first().data(), top->childContexts().first()->localDeclarations().first());
    QCOMPARE(StructureType::Ptr::staticCast(res.type())->qualifiedIdentifier(), QualifiedIdentifier("a"));
}
void TestExpressionParser::memberFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public function foo() {} } $i = new A();");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType(QByteArray("$i->foo()"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(res.type());
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type())->dataType() == IntegralType::TypeVoid);
    QCOMPARE(res.allDeclarations().size(), 1);
    QCOMPARE(res.allDeclarations().first().data(), top->childContexts().first()->localDeclarations().first());
}

void TestExpressionParser::newTrait()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? trait A { function foo() {} }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->problems().isEmpty());

    QCOMPARE(top->childContexts().size(), 1);
    QVERIFY(top->childContexts().at(0)->type() == DUContext::Class);
}

void TestExpressionParser::newTraitWithAbstractMethod()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? trait A { public abstract function foo(); }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->problems().isEmpty());

    QCOMPARE(top->childContexts().size(), 1);
    QVERIFY(top->childContexts().at(0)->type() == DUContext::Class);
}

void TestExpressionParser::invalidTrait_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("constant") << "<? trait A { const FOO = ''; }\n";
}

void TestExpressionParser::invalidTrait()
{
    QFETCH(QString, code);

    TopDUContext* top = parse(code.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(!top->problems().isEmpty());
}

void TestExpressionParser::invalidTraitUse_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("staticModifier") << "<? trait A { public function foo(){} } class Foo { use A { A::foo as public static bla; } }\n";

    QTest::newRow("finalModifier") << "<? trait A { public function foo(){} } class Foo { use A { A::foo as public final bla; } }\n";

    QTest::newRow("traitMethodCollision") << "<? trait A { public function foo(){} } trait B { public function foo(){} } class Foo { use A,B; }\n";

    QTest::newRow("propertyCollision") << "<? trait A { public $arg; } class Foo { use A; public $arg; }\n";
}

void TestExpressionParser::invalidTraitUse()
{
    QFETCH(QString, code);

    TopDUContext* top = parse(code.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(!top->problems().isEmpty());
}

void TestExpressionParser::namespaceUseNameConflict()
{
    QByteArray alias("<?php namespace Bar { class Foo {} use Foo; }\n");

    TopDUContext* top = parse(alias, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(!top->problems().isEmpty());
}

void TestExpressionParser::globalFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo() {}");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType(QByteArray("foo"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(res.type());
    QVERIFY(FunctionType::Ptr::dynamicCast(res.type()));
    QCOMPARE(res.allDeclarations().count(), 1);
    QCOMPARE(res.allDeclarations().first().data(), top->localDeclarations().first());
}

void TestExpressionParser::chainCall()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function foo() { return $this; } } $a = new A();");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fn = top->childContexts().first()->localDeclarations().first()->type<FunctionType>();
    QVERIFY(fn);
    QVERIFY(fn->returnType()->equals(top->localDeclarations().first()->abstractType().unsafeData()));

    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType(QByteArray("$a->foo()"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(res.type());
    QVERIFY(res.type()->equals(top->localDeclarations().first()->abstractType().unsafeData()));

    res = p.evaluateType(QByteArray("$a->foo()->foo()->foo()"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(res.type());
    QVERIFY(res.type()->equals(top->localDeclarations().first()->abstractType().unsafeData()));
}
void TestExpressionParser::thisObject()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public function foo() {} }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* funContext = top->childContexts().first()->localDeclarations().first()->internalContext();
    ExpressionParser p(true);
    ExpressionEvaluationResult res = p.evaluateType(QByteArray("$this"), DUContextPointer(funContext), CursorInRevision(1, 0));
    QCOMPARE(res.allDeclarations().count(), 1);
    QCOMPARE(res.allDeclarations().first().data(), top->localDeclarations().first());
    QVERIFY(res.type());
    QVERIFY(StructureType::Ptr::dynamicCast(res.type()));
    QCOMPARE(StructureType::Ptr::dynamicCast(res.type())->declaration(top), top->localDeclarations().first());
}

void TestExpressionParser::integralTypes()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $foo=1;");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);

    ExpressionEvaluationResult res = p.evaluateType(QByteArray("123"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeInt));

    res = p.evaluateType(QByteArray("123.1"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeFloat));

    res = p.evaluateType(QByteArray("\"asdf\""), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeString));

    res = p.evaluateType(QByteArray("\"as $foo df\""), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeString));

    res = p.evaluateType(QByteArray("'asdf'"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeString));

    res = p.evaluateType(QByteArray("true"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeBoolean));

    res = p.evaluateType(QByteArray("TRUE"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeBoolean));

    res = p.evaluateType(QByteArray("null"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeNull));

    res = p.evaluateType(QByteArray("NULL"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeNull));
}

void TestExpressionParser::newObject()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);

    ExpressionEvaluationResult res = p.evaluateType(QByteArray("new A();"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(StructureType::Ptr::dynamicCast(res.type()));
    QCOMPARE(StructureType::Ptr::staticCast(res.type())->declaration(top), top->localDeclarations().first());
}

void TestExpressionParser::cast()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $foo = 1; ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);

    ExpressionEvaluationResult res = p.evaluateType(QByteArray("(string)$foo"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeString);

    res = p.evaluateType(QByteArray("(int)$foo"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeInt);

    res = p.evaluateType(QByteArray("(double)$foo"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeFloat);

    res = p.evaluateType(QByteArray("(bool)$foo"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeBoolean);

    res = p.evaluateType(QByteArray("(array)$foo"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeArray);

    res = p.evaluateType(QByteArray("(object)$foo"), DUContextPointer(top), CursorInRevision(1, 0));
    kDebug() << res.type();
    kDebug() << res.type()->toString();
    QVERIFY(StructureType::Ptr::dynamicCast(res.type()));
    QVERIFY(StructureType::Ptr::staticCast(res.type())->qualifiedIdentifier() == QualifiedIdentifier("stdclass"));
}

void TestExpressionParser::operations()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $foo = 1; ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);

    ExpressionEvaluationResult res = p.evaluateType(QByteArray("'1' . '1'"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeString);

    res = p.evaluateType(QByteArray("1 . 1"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeString);

    res = p.evaluateType(QByteArray("1 + 1"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeInt);

    res = p.evaluateType(QByteArray("'1' + '1'"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeInt);

    res = p.evaluateType(QByteArray("$foo .= '1'"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeString);

    res = p.evaluateType(QByteArray("$foo .= 1"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeString);

    res = p.evaluateType(QByteArray("$foo += 1"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeInt);

    res = p.evaluateType(QByteArray("$foo += '1'"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeInt);

    res = p.evaluateType(QByteArray("$foo *= 1"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeInt);

    res = p.evaluateType(QByteArray("$foo *= '1'"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QVERIFY(IntegralType::Ptr::staticCast(res.type())->dataType() == IntegralType::TypeInt);
}

void TestExpressionParser::findArg()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A{} function foo($arg, &$bar, A &$a) {  } ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ExpressionParser p(true);

    QCOMPARE(top->childContexts().size(), 3);
    QVERIFY(top->childContexts().at(0)->type() == DUContext::Class);
    QVERIFY(top->childContexts().at(1)->type() == DUContext::Function);
    QVERIFY(top->childContexts().at(2)->type() != DUContext::Function);

    ExpressionEvaluationResult res = p.evaluateType(QByteArray("$arg"), DUContextPointer(top->childContexts().last()),
                                                    CursorInRevision(0, 47));
    QVERIFY(IntegralType::Ptr::dynamicCast(res.type()));
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeMixed));

    res = p.evaluateType(QByteArray("$bar"), DUContextPointer(top->childContexts().last()),
                         CursorInRevision(0, 47));
    ReferenceType::Ptr type = ReferenceType::Ptr::dynamicCast(res.type());
    QVERIFY(type);
    QVERIFY(IntegralType::Ptr::dynamicCast(type->baseType()));
    QCOMPARE(IntegralType::Ptr::staticCast(type->baseType())->dataType(), static_cast<uint>(IntegralType::TypeMixed));

    res = p.evaluateType(QByteArray("$a"), DUContextPointer(top->childContexts().last()),
                         CursorInRevision(0, 47));
    type = ReferenceType::Ptr::dynamicCast(res.type());
    QVERIFY(type);
    QVERIFY(StructureType::Ptr::dynamicCast(type->baseType()));
    QCOMPARE(StructureType::Ptr::staticCast(type->baseType())->declaration(top), top->localDeclarations().first());
}

void TestExpressionParser::array_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("normalSyntax") << "<? $a = array(1,2,3);\n";

    QTest::newRow("shortSyntax") << "<? $a = [1,2,3];\n";

    QTest::newRow("staticNormalSyntax") << "<? static $a = array(1,2,3);\n";

    QTest::newRow("staticShortSyntax") << "<? static $a = [1,2,3];\n";
}

void TestExpressionParser::array()
{
    // see bug https://bugs.kde.org/show_bug.cgi?id=237110

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QFETCH(QString, code);

    TopDUContext* top = parse(code.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());

    ExpressionParser p(true);
    QCOMPARE(top->localDeclarations().first()->abstractType().cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeArray));

    ExpressionEvaluationResult res = p.evaluateType("$b = $a[0]", DUContextPointer(top), CursorInRevision(0, 22));
    QVERIFY(res.type().cast<IntegralType>());
    QEXPECT_FAIL("", "we'd need advanced array support to know that [0] returns a string...", Continue);
    QCOMPARE(res.type().cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeString));
    // fallback
    QCOMPARE(res.type().cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeMixed));
}

void TestExpressionParser::arrayFunctionDereferencing_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("globalFunction") << "<? function foo() { return [1, 2]; }\n"
                                       "$a = foo()[0];\n";

    QTest::newRow("classMethod") << "<? class foo{ function bar() { return [1, 2]; } }\n"
                                       "$obj = new foo(); $a = $obj->bar()[0];\n";

    QTest::newRow("staticClassMethod") << "<? class foo{ static function bar() { return [1, 2]; } }\n"
                                       "$a = foo::bar()[0];\n";
}

void TestExpressionParser::arrayFunctionDereferencing()
{
    // see bug https://bugs.kde.org/show_bug.cgi?id=305779
    QFETCH(QString, code);
    /*
    QByteArray method("<? function foo() { return [1, 2]; }\n"
                      "class obj { static function bar() { return [1, 2]; } }\n"
                      "$myObj = new obj();\n"
                      // actual test stuff:
                      "$a = foo()[0];\n"
                      "$b = $myObj->bar()[0];\n"
                      "$c = obj::bar()[0];\n");*/

    TopDUContext* top = parse(code.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());

    Declaration* decl = top->localDeclarations().last();
    IntegralType::Ptr type = decl->abstractType().cast<IntegralType>();
    QVERIFY(type);
    QEXPECT_FAIL("", "we'd need advanced array support to know that [0] returns an int...", Continue);
    QCOMPARE(type->dataType(), static_cast<uint>(IntegralType::TypeInt));
    // fallback
    QCOMPARE(type->dataType(), static_cast<uint>(IntegralType::TypeMixed));
}

void TestExpressionParser::arrayLiteralDereferencing_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("normalSyntax") << "<? $a = array(1,2,3)[1];\n";

    QTest::newRow("shortSyntax") << "<? $a = [1,2,3][1];\n";
}

void TestExpressionParser::arrayLiteralDereferencing()
{
    QFETCH(QString, code);

    TopDUContext* top = parse(code.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());

    Declaration* decl = top->localDeclarations().last();
    IntegralType::Ptr type = decl->abstractType().cast<IntegralType>();
    QVERIFY(type);
    QEXPECT_FAIL("", "we'd need advanced array support to know that [0] returns an int...", Continue);
    QCOMPARE(type->dataType(), static_cast<uint>(IntegralType::TypeInt));
    // fallback
    QCOMPARE(type->dataType(), static_cast<uint>(IntegralType::TypeMixed));
}

void TestExpressionParser::stringAsArray_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("constantEncapsedString") << "<? $a = 'string'[1];\n";

    QTest::newRow("dynamicString") << "<? $string = 'Hello';\n"
                                      "$a = \"$string World\"[1];\n";
}

void TestExpressionParser::stringAsArray()
{
    QFETCH(QString, code);

    TopDUContext* top = parse(code.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());

    Declaration* decl = top->localDeclarations().last();
    IntegralType::Ptr type = decl->abstractType().cast<IntegralType>();
    QVERIFY(type);
    QCOMPARE(type->dataType(), static_cast<uint>(IntegralType::TypeString));
}

void TestExpressionParser::classMemberOnInstantiation()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A{ function foo(){ return 'a'; } } $a = (new A())->foo();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());

    ExpressionParser p(true);

    ExpressionEvaluationResult res = p.evaluateType(QByteArray("$a"), DUContextPointer(top), CursorInRevision(1, 0));
    QVERIFY(res.type());
    QCOMPARE(IntegralType::Ptr::staticCast(res.type())->dataType(), static_cast<uint>(IntegralType::TypeString));
}

}

#include "expressionparser.moc"
