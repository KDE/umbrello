/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "uses.h"

#include <QtTest/QtTest>

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

#include "../declarations/classdeclaration.h"
#include "../declarations/variabledeclaration.h"
#include "../declarations/traitmethodaliasdeclaration.h"
#include "../declarations/traitmemberaliasdeclaration.h"

using namespace KDevelop;

QTEST_MAIN(Php::TestUses)

namespace Php
{

void compareUses(Declaration* dec, QList<RangeInRevision> ranges)
{
    qDebug() << "comparing uses for" << dec->toString();
    QCOMPARE(dec->uses().keys().count(), 1);
    QCOMPARE(dec->uses().values().count(), 1);
    QCOMPARE(dec->uses().values().first().count(), ranges.count());
    for (int i = 0; i < ranges.count(); ++i) {
        qDebug() << dec->uses().values().first().at(i) << ranges.at(i);
        QCOMPARE(dec->uses().values().first().at(i), ranges.at(i));
    }
}

void compareUses(Declaration* dec, RangeInRevision range)
{
    QList<RangeInRevision> r;
    r << range;
    compareUses(dec, r);
}

TestUses::TestUses()
{
}

void TestUses::newObject()
{

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class Foo {} $a = new Foo(); ");
    TopDUContext* top = parse(method, DumpNone, QUrl(QStringLiteral("file:///internal/usestest/newObject.php")));
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    compareUses(top->localDeclarations().first(), RangeInRevision(0, 25, 0, 28));
    QCOMPARE(top->localDeclarations().first()->uses().keys().first(), IndexedString(QUrl("file:///internal/usestest/newObject.php")));
}

void TestUses::functionCall()
{

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo() {} foo(); ");
    TopDUContext* top = parse(method, DumpNone, QUrl(QStringLiteral("file:///internal/usestest/functionCall.php")));
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    Declaration* fun = top->localDeclarations().first();
    compareUses(fun, RangeInRevision(0, 21, 0, 24));
    QCOMPARE(fun->uses().keys().first(), IndexedString(QUrl("file:///internal/usestest/functionCall.php")));
}

void TestUses::memberFunctionCall()
{

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function foo() {} } $a = new A(); $a->foo(); ");
    TopDUContext* top = parse(method, DumpNone, QUrl(QStringLiteral("file:///internal/usestest/memberFunctionCall.php")));
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    Declaration* fun = top->childContexts().first()->localDeclarations().first();
    compareUses(fun, RangeInRevision(0, 51, 0, 54));
    QCOMPARE(fun->uses().keys().first(), IndexedString(QUrl("file:///internal/usestest/memberFunctionCall.php")));
}

void TestUses::memberVariable()
{

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public $foo; } $a = new A(); $a->foo; ");
    TopDUContext* top = parse(method, DumpNone, QUrl(QStringLiteral("file:///internal/usestest/memberVariable.php")));
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    Declaration* var = top->childContexts().first()->localDeclarations().first();
    compareUses(var, RangeInRevision(0, 46, 0, 49));
    QCOMPARE(var->uses().keys().first(), IndexedString(QUrl("file:///internal/usestest/memberVariable.php")));
}

void TestUses::variable()
{
    //                        0         1         2         3         4         5         6         7
    //                        01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?php\nclass A { public $foo; } $a = new A(); $a; $a->foo; foo($a); ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(1, 42 - 3, 1, 44 - 3) << RangeInRevision(1, 46 - 3, 1, 48 - 3) << RangeInRevision(1, 59 - 3, 1, 61 - 3);
    compareUses(top->localDeclarations().at(1), ranges);
}

void TestUses::varInString()
{

    //                  0         1         2         3         4         5         6         7
    //                  01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?php $a=0; \"$a {$a}\"; ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(0, 13, 0, 15) << RangeInRevision(0, 17, 0, 19);
    compareUses(top->localDeclarations().at(0), ranges);
}

void TestUses::variableInNamespace()
{

    //                        0         1         2         3         4         5         6         7
    //                        01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?php\nclass A { public $foo; } namespace Foo { $a = new A(); $a; $a->foo; foo($a); };");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(1, 55, 1, 57) << RangeInRevision(1, 59, 1, 61) << RangeInRevision(1, 72, 1, 74);
    compareUses(top->localDeclarations().at(2), ranges);
}

void TestUses::globalVariableInNamespace()
{

    //                        0         1         2         3         4         5         6         7
    //                        01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?php\nclass A { public $foo; } $a = new A(); namespace Foo { $a; $a->foo; foo($a); };");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(1, 55, 1, 57) << RangeInRevision(1, 59, 1, 61) << RangeInRevision(1, 72, 1, 74);
    compareUses(top->localDeclarations().at(1), ranges);
}

void TestUses::variableInOtherNamespace()
{
    //                        0         1         2         3         4         5         6         7
    //                        01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?php\nclass A { public $foo; } namespace Foo { $a = new A(); } namespace Bar { $a; $a->foo; foo($a); };");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(1, 73, 1, 75) << RangeInRevision(1, 77, 1, 79) << RangeInRevision(1, 90, 1, 92);
    compareUses(top->localDeclarations().at(2), ranges);
}

void TestUses::memberVarInString()
{

    //                 0         1         2         3         4          5         6          7
    //                 01234567890123456789012345678901234567890123456789 01234567890123 4567890123456789
    QByteArray method("<?php class A { public $v=0; } $a=new A(); $a->v; \"$a->v {$a->v}\"; ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(0, 43, 0, 45) << RangeInRevision(0, 51, 0, 53) << RangeInRevision(0, 58, 0, 60);
    compareUses(top->localDeclarations().at(1), ranges);

    ranges.clear();
    ranges << RangeInRevision(0, 47, 0, 48) << RangeInRevision(0, 55, 0, 56) << RangeInRevision(0, 62, 0, 63);
    compareUses(top->childContexts().first()->localDeclarations().first(), ranges);
}

void TestUses::memberFunctionInString()
{

    //                 0         1         2         3         4          5          6         7
    //                 012345678901234567890123456789012345678901234567 890123456789 01234567890123456789
    QByteArray method("<?php class A { function foo() {} } $a=new A(); \"{$a->foo()}\"; ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    //$a
    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 50, 0, 52));

    //foo
    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 54, 0, 57));
}

void TestUses::variableTypeChange()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { } $a = 'a'; $a; $a = 0; $a; $a = 'x'; $a; ");
    //                                15        25  29      37  41        51
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(0, 25, 0, 27);
    ranges << RangeInRevision(0, 29, 0, 31);
    ranges << RangeInRevision(0, 37, 0, 39);
    ranges << RangeInRevision(0, 41, 0, 43);
    ranges << RangeInRevision(0, 51, 0, 53);
    compareUses(top->localDeclarations().at(1), ranges);
}

void TestUses::variableTypeChangeInFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo() { $a='a'; $a; $a=0; $a; $a=false; }");
    //                                     20      28  32    38  42

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(0, 28, 0, 30);
    ranges << RangeInRevision(0, 32, 0, 34);
    ranges << RangeInRevision(0, 38, 0, 40);
    ranges << RangeInRevision(0, 42, 0, 44);
    compareUses(top->childContexts().at(1)->localDeclarations().at(0), ranges);
}

void TestUses::classExtends()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { } class B extends A { } ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().at(0), RangeInRevision(0, 31, 0, 32));
}

void TestUses::classImplements()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface A { } class B implements A { } ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().at(0), RangeInRevision(0, 38, 0, 39));
}

void TestUses::classImplementsMultiple()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface A { } interface B { } class C implements A, B { } ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().at(0), RangeInRevision(0, 54, 0, 55));
    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 57, 0, 58));
}

void TestUses::interfaceExtends()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface A { } interface B extends A { }");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().at(0), RangeInRevision(0, 39, 0, 40));
}

void TestUses::interfaceExtendsMultiple()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface A { } interface B { } interface C extends A, B { }");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().at(0), RangeInRevision(0, 55, 0, 56));
    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 58, 0, 59));
}

void TestUses::staticMemberFunctionCall()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public static function foo() {} } A::foo(); ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 47, 0, 48));
    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 50, 0, 53));
}

void TestUses::staticMemberVariable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public static $foo; } $foo=0; A::$foo; $foo;");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 43, 0, 44));
    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 46, 0, 50));
    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 52, 0, 56));
}

void TestUses::constant()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? define('A', 'foo'); echo A;");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 28, 0, 29));
}

void TestUses::classConstant()
{
    {
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { const FOO = 'abc'; } echo A::FOO;");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 39, 0, 40));
    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 42, 0, 45));
    }
    {
    // bug: https://bugs.kde.org/show_bug.cgi?id=241597

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { const FOO = 'abc'; }\n"
                      "class B extends A { function foo() { self::FOO; } }\n"
                      "A::FOO;\n"
                      "B::FOO;\n");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    Declaration* dec = top->childContexts().first()->localDeclarations().first();
    QVERIFY(dec->abstractType()->modifiers() & AbstractType::ConstModifier);
    QCOMPARE(dec->qualifiedIdentifier().toString(), QString("a::FOO"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(1, 43, 1, 46)
                                          << RangeInRevision(2, 3, 2, 6)
                                          << RangeInRevision(3, 3, 3, 6));
    }
}

void TestUses::classParent()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function x() {} } class B extends A { function x() { parent::x(); }} ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<RangeInRevision> range;
    range << RangeInRevision(0, 47, 0, 48);
    range << RangeInRevision(0, 66, 0, 72);
    compareUses(top->localDeclarations().first(), range);

    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 74, 0, 75));
}

void TestUses::classSelf()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function x() { self::x(); } } ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 28, 0, 32));
    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 34, 0, 35));
}
void TestUses::classThis()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function x() { $this->x(); } } ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 28, 0, 33));
    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 35, 0, 36));
}

void TestUses::objectWithClassName()
{
    //                 0         1         2         3         4         5         6         7         8
    //                 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class Aa { public static $i; const j=0; public $k; } $Aa = new Aa; $Aa->k; Aa::j; Aa::$i;");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(0, 66, 0, 66 + 2);
    ranges << RangeInRevision(0, 78, 0, 78 + 2);
    ranges << RangeInRevision(0, 85, 0, 85 + 2);
    compareUses(top->localDeclarations().first(), ranges);

    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 70, 0, 70 + 3));
}

void TestUses::classAndConstWithSameName()
{
    //                 0         1         2         3         4         5         6         7         8
    //                 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { } define('A', 0); A; new A; define('B', 0); class B { } new B; B; ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 38, 0, 39));
    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 31, 0, 32));
    compareUses(top->localDeclarations().at(2), RangeInRevision(0, 76, 0, 77));
    compareUses(top->localDeclarations().at(3), RangeInRevision(0, 73, 0, 74));
}


void TestUses::classAndFunctionWithSameName()
{
    //                 0         1         2         3         4         5         6         7         8
    //                 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { } function A() {} new A; A(); ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 35, 0, 36));
    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 38, 0, 39));
}

void TestUses::constAndVariableWithSameName()
{
    //                 0         1         2         3         4         5         6         7         8
    //                 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $A = 0; define('A', 0); A; $A; ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    compareUses(top->localDeclarations().first(), RangeInRevision(0, 30, 0, 32));
    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 27, 0, 28));
}

void TestUses::functionAndClassWithSameName()
{
    //                 0         1         2         3         4         5         6         7         8
    //                 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface foo { function asdf(); } class asdf {} class bar extends asdf implements foo {} ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* fnAsdf = top->childContexts().first()->localDeclarations().first();
    QCOMPARE(fnAsdf->uses().keys().count(), 0);

    compareUses(top->localDeclarations().at(1), RangeInRevision(0, 70, 0, 74));
}

void TestUses::constantInClassMember()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? define('TEST', 1); class A { var $a = TEST; var $b = array( TEST ); } TEST;");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* constant = top->findDeclarations(Identifier(QStringLiteral("TEST"))).first();

    QList<RangeInRevision> uses;
    uses << RangeInRevision(0, 41, 0, 45);
    uses << RangeInRevision(0, 63, 0, 67);
    uses << RangeInRevision(0, 73, 0, 77);
    compareUses(constant, uses);
}

void TestUses::useInAsignment()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = 0; $b = $a; ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration *d = top->localDeclarations().first();
    compareUses(d, RangeInRevision(0, 16, 0, 18));
}

void TestUses::foreachArray()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = array(1); foreach($a as $k=>$i) { var_dump($k, $i); } ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    // $a, $k, $i
    QCOMPARE(top->localDeclarations().size(), 3);

    // $a
    Declaration *d = top->localDeclarations().at(0);
    compareUses(d, RangeInRevision(0, 26, 0, 28));

    // $k
    d = top->localDeclarations().at(1);
    compareUses(d, RangeInRevision(0, 51, 0, 53));

    // $i
    d = top->localDeclarations().at(2);
    compareUses(d, RangeInRevision(0, 55, 0, 57));
}

void TestUses::assignmentToMemberArray()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?php class x { var $y; function z($a) { $b = $a; $this->y[$a] = true; } }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    // class x
    Declaration *x = top->localDeclarations().first();
    QVERIFY(x);

    // $this
    compareUses(x, RangeInRevision(0, 50, 0, 55));

    // var $y
    Declaration *y = x->logicalInternalContext(top)->findDeclarations(Identifier(QStringLiteral("y"))).first();
    QVERIFY(y);

    // $this->y
    compareUses(y, RangeInRevision(0, 57, 0, 58));

    // function z
    Declaration *z = x->logicalInternalContext(top)->findDeclarations(Identifier(QStringLiteral("z"))).first();
    QVERIFY(z);

    // $a
    Declaration *a = z->logicalInternalContext(top)->findDeclarations(Identifier(QStringLiteral("a"))).first();
    QVERIFY(a);
    compareUses(a, QList<RangeInRevision>()
                // $b = $a
                << RangeInRevision(0, 46, 0, 48)
                // $this->y[$a]
                << RangeInRevision(0, 59, 0, 61)
               );
}

void TestUses::staticArrayIndex()
{
    // bug: https://bugs.kde.org/show_bug.cgi?id=241160

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?php class x { static private $a = array(); function z($i) { self::$a[$i]; } }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* a = top->childContexts().first()->localDeclarations().first();
    QCOMPARE(a->identifier().toString(), QString("a"));
    compareUses(a, RangeInRevision(0, 68, 0, 70));

    Declaration* i = top->childContexts().first()->childContexts().first()->localDeclarations().first();
    QCOMPARE(i->identifier().toString(), QString("i"));
    compareUses(i, RangeInRevision(0, 71, 0, 73));
}

void TestUses::functionParamNewDeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo($a) { $a; $a = 0; }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration *d = top->childContexts().first()->localDeclarations().first();
    QList<RangeInRevision> ranges;
    ranges << RangeInRevision(0, 22, 0, 24);
    ranges << RangeInRevision(0, 26, 0, 28);
    compareUses(d, ranges);
}

void TestUses::catchClass()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? try { } catch (Exception $e) {}");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration *d = top->findDeclarations(QualifiedIdentifier(QStringLiteral("exception"))).first();
    compareUses(d, RangeInRevision(0, 18, 0, 27));
}

void TestUses::variableRedeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $s = 'a'; $s = $s . $s;");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList< Declaration* > decs = top->findDeclarations(QualifiedIdentifier(QStringLiteral("s")));
    QCOMPARE(decs.size(), 1);
    Declaration *d = decs.first();
    compareUses(d, QList<RangeInRevision>()
                     << RangeInRevision(0, 13, 0, 15)
                     << RangeInRevision(0, 18, 0, 20)
                     << RangeInRevision(0, 23, 0, 25)
                );
}

void TestUses::caseInsensitiveFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function fooBar(){}\n"
                      "fOoBar();\nFOOBAR();\nfoobar();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->findLocalDeclarations(Identifier(QStringLiteral("foobar")));
    QCOMPARE(decs.size(), 1);
    Declaration *d = decs.first();
    compareUses(d, QList<RangeInRevision>()
                    << RangeInRevision(1, 0, 1, 6)
                    << RangeInRevision(2, 0, 2, 6)
                    << RangeInRevision(3, 0, 3, 6)
                );
}

void TestUses::caseInsensitiveMethod()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class asdf{ static function barFoo(){} function fooBar() {} } $a = new asdf;\n"
                      "$a->fOoBar();\n$a->FOOBAR();\n$a->foobar();\n"
                      "asdf::barfoo();\nasdf::bArFoo();\nasdf::BARFOO();\n");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    {
        QList<Declaration*> decs = top->childContexts().first()->findDeclarations(QualifiedIdentifier(QStringLiteral("foobar")));
        QCOMPARE(decs.size(), 1);
        Declaration *d = decs.first();
        compareUses(d, QList<RangeInRevision>()
                        << RangeInRevision(1, 4, 1, 10)
                        << RangeInRevision(2, 4, 2, 10)
                        << RangeInRevision(3, 4, 3, 10)
                    );
    }

    {
        QList<Declaration*> decs = top->childContexts().first()->findDeclarations(QualifiedIdentifier(QStringLiteral("barfoo")));
        QCOMPARE(decs.size(), 1);
        Declaration *d = decs.first();
        compareUses(d, QList<RangeInRevision>()
                        << RangeInRevision(4, 6, 4, 12)
                        << RangeInRevision(5, 6, 5, 12)
                        << RangeInRevision(6, 6, 6, 12)
                    );
    }
}

void TestUses::caseInsensitiveClass()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class asDf{}\n"
                      "new asdf();\nnew ASDF();\nnew asDF();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->findLocalDeclarations(Identifier(QStringLiteral("asdf")));
    QCOMPARE(decs.size(), 1);
    Declaration *d = decs.first();
    compareUses(d, QList<RangeInRevision>()
                    << RangeInRevision(1, 4, 1, 8)
                    << RangeInRevision(2, 4, 2, 8)
                    << RangeInRevision(3, 4, 3, 8)
                );
}

void TestUses::functionUseBeforeDeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? test(); function test() {}");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVector<Declaration*> decs = top->localDeclarations();
    QCOMPARE(decs.size(), 1);
    QCOMPARE(decs.first()->range(), RangeInRevision(0, 20, 0, 24));
    compareUses(decs.first(), RangeInRevision(0, 3, 0, 7));
}

void TestUses::propertyAndMethodWithSameName()
{

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class a{ function name1(){} public $name1; public $name2; function name2() {} }\n"
                      "$a = new a;\n"
                      "$a->name1(); $a->name1;\n"
                      "$a->name2; $a->name2();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVector<Declaration*> decs = top->childContexts().first()->localDeclarations();
    QCOMPARE(decs.size(), 4);

    // method name1
    QVERIFY(decs[0]->identifier().nameEquals(Identifier("name1")));
    QVERIFY(decs[0]->isFunctionDeclaration());
    compareUses(decs[0], RangeInRevision(2, 4, 2, 9));
    // property name1
    QVERIFY(decs[1]->identifier().nameEquals(Identifier("name1")));
    QVERIFY(!decs[1]->isFunctionDeclaration());
    compareUses(decs[1], RangeInRevision(2, 17, 2, 22));

    // property name2
    QVERIFY(decs[2]->identifier().nameEquals(Identifier("name2")));
    QVERIFY(!decs[2]->isFunctionDeclaration());
    compareUses(decs[2], RangeInRevision(3, 4, 3, 9));
    // method name2
    QVERIFY(decs[3]->identifier().nameEquals(Identifier("name2")));
    QVERIFY(decs[3]->isFunctionDeclaration());
    compareUses(decs[3], RangeInRevision(3, 15, 3, 20));
}

void TestUses::nestedMethodCalls()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?\n"
                      "class a{ /** @return a **/ function a(){} }\n"
                      "class b{ function b(){} }\n"
                      "$a = new a;\n"
                      "$b = new b;\n"
                      "$a->a($b->b());");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVector<Declaration*> topDecs  = top->localDeclarations();
    QCOMPARE(topDecs.size(), 4);

    // class a
    QVERIFY(topDecs[0]->identifier().nameEquals(Identifier("a")));
    QVERIFY(dynamic_cast<ClassDeclaration*>(topDecs[0]));
    compareUses(topDecs[0], RangeInRevision(3, 9, 3, 10));
    // class b
    QVERIFY(topDecs[1]->identifier().nameEquals(Identifier("b")));
    QVERIFY(dynamic_cast<ClassDeclaration*>(topDecs[1]));
    compareUses(topDecs[1], RangeInRevision(4, 9, 4, 10));

    // $a
    QVERIFY(topDecs[2]->identifier().nameEquals(Identifier("a")));
    QVERIFY(dynamic_cast<VariableDeclaration*>(topDecs[2]));
    compareUses(topDecs[2], RangeInRevision(5, 0, 5, 2));
    // $b
    QVERIFY(topDecs[3]->identifier().nameEquals(Identifier("b")));
    QVERIFY(dynamic_cast<VariableDeclaration*>(topDecs[3]));
    compareUses(topDecs[3], RangeInRevision(5, 6, 5, 8));

    // function a
    Declaration* methodADec = topDecs[0]->internalContext()->localDeclarations().first();
    QVERIFY(methodADec->isFunctionDeclaration());
    compareUses(methodADec, RangeInRevision(5, 4, 5, 5));

    // function b
    Declaration* methodBDec = topDecs[1]->internalContext()->localDeclarations().first();
    QVERIFY(methodBDec->isFunctionDeclaration());
    compareUses(methodBDec, RangeInRevision(5, 10, 5, 11));
}

void TestUses::unset()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = 1; unset($a);");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVector<Declaration*> decs = top->localDeclarations();
    QCOMPARE(decs.size(), 1);
    QCOMPARE(decs.first()->range(), RangeInRevision(0, 3, 0, 5));
    compareUses(decs.first(), RangeInRevision(0, 17, 0, 19));
}

void TestUses::functionArguments()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo($a, &$b) { $a = 0; $b = 2; }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->childContexts().size(), 2);
    QCOMPARE(top->childContexts().first()->type(), DUContext::Function);

    // $a
    Declaration *d = top->childContexts().at(0)->localDeclarations().at(0);
    compareUses(d, RangeInRevision(0, 27, 0, 29));

    // $b
    d = top->childContexts().at(0)->localDeclarations().at(1);
    compareUses(d, RangeInRevision(0, 35, 0, 37));
}

void TestUses::namespaces()
{
    //                         0         1         2         3         4         5         6         7
    //                         01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "namespace Foo\\Bar {\n"
                              "const MyConst = 1;\n"
                              "$z = MyConst;\n"
                              "function MyFunc(){}\n"
                              "class MyClass{ const ClassConst = 2; }\n"
                              "interface MyInterface{}\n"
                              "}\n"
                              "namespace {\n"
                              "\\Foo\\Bar\\MyConst;\n"
                              "\\Foo\\Bar\\MyClass::ClassConst;\n"
                              "\\Foo\\Bar\\MyFunc();\n"
                              "new \\Foo\\Bar\\MyClass;\n"
                              "function Func(\\Foo\\Bar\\MyClass $a){}\n"
                              "class a extends \\Foo\\Bar\\MyClass implements \\Foo\\Bar\\MyInterface {}\n"
                              "}\n"
                              "namespace Foo {\n"
                              "}\n", DumpAll);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    Declaration* dec;

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo"))).last();
    QCOMPARE(dec->kind(), Declaration::Namespace);
    compareUses(dec, QList<RangeInRevision>()
                                          << RangeInRevision(9, 1, 9, 4)
                                          << RangeInRevision(10, 1, 10, 4)
                                          << RangeInRevision(11, 1, 11, 4)
                                          << RangeInRevision(12, 5, 12, 8)
                                          << RangeInRevision(13, 15, 13, 18)
                                          << RangeInRevision(14, 17, 14, 20)
                                          << RangeInRevision(14, 45, 14, 48));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar"))).first();
    QCOMPARE(dec->kind(), Declaration::Namespace);
    QVERIFY(dec->internalContext());
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(9, 5, 9, 8)
                                          << RangeInRevision(10, 5, 10, 8)
                                          << RangeInRevision(11, 5, 11, 8)
                                          << RangeInRevision(12, 9, 12, 12)
                                          << RangeInRevision(13, 19, 13, 22)
                                          << RangeInRevision(14, 21, 14, 24)
                                          << RangeInRevision(14, 49, 14, 52));
    QCOMPARE(dec->internalContext()->localDeclarations().size(), 4);
    for(Declaration* d : dec->internalContext()->localDeclarations()) {
        qDebug() << d->toString() << d->qualifiedIdentifier();
    }

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::MyConst"))).first();
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(3, 5, 3, 12)
                                          << RangeInRevision(9, 9, 9, 16));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::myclass"))).first();
    QVERIFY(dynamic_cast<ClassDeclaration*>(dec));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(10, 9, 10, 16)
                                          << RangeInRevision(12, 13, 12, 20)
                                          << RangeInRevision(13, 23, 13, 30)
                                          << RangeInRevision(14, 25, 14, 32)
               );
    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::myinterface"))).first();
    QVERIFY(dynamic_cast<ClassDeclaration*>(dec));
    compareUses(dec, RangeInRevision(14, 53, 14, 64) );

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::myclass::ClassConst"))).first();
    compareUses(dec, RangeInRevision(10, 18, 10, 28));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::myfunc"))).first();
    compareUses(dec, RangeInRevision(11, 9, 11, 15));
}

void TestUses::useNamespace()
{
    //                         0         1         2         3         4         5         6         7
    //                         01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "namespace Foo\\Bar {class A{} function B(){} const C = 1;}\n"
                              "namespace VeryLong {class A{} function B(){} const C = 1;}\n"
                              "namespace Baz {class A{} const C = 1;}\n"
                              "namespace {\n"
                              "use Foo\\Bar, VeryLong as Short;\n"
                              "use Baz\\A as Bazaar;\n"
                              "new Bar\\A; Bar\\B(); Bar\\C;\n"
                              "new Short\\A; Short\\B(); Short\\C;\n"
                              "new Bazaar;\n"
                              "}\n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    Declaration* dec;
    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo"))).first();
    QCOMPARE(dec->kind(), Declaration::Namespace);
    compareUses(dec, RangeInRevision(5, 4, 5, 7));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar"))).first();
    QCOMPARE(dec->kind(), Declaration::Namespace);
    compareUses(dec, RangeInRevision(5, 8, 5, 11));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("verylong"))).first();
    QCOMPARE(dec->kind(), Declaration::Namespace);
    compareUses(dec, RangeInRevision(5, 13, 5, 21));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("bar"))).first();
    QCOMPARE(dec->kind(), Declaration::NamespaceAlias);
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(7, 4, 7, 7)
                                          << RangeInRevision(7, 11, 7, 14)
                                          << RangeInRevision(7, 20, 7, 23) );

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("short"))).first();
    QCOMPARE(dec->kind(), Declaration::NamespaceAlias);
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(8, 4, 8, 9)
                                          << RangeInRevision(8, 13, 8, 18)
                                          << RangeInRevision(8, 24, 8, 29) );

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("baz::a"))).first();
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(6, 8, 6, 9)
                                          << RangeInRevision(9, 4, 9, 10));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::a"))).first();
    compareUses(dec, RangeInRevision(7, 8, 7, 9));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::b"))).first();
    compareUses(dec, RangeInRevision(7, 15, 7, 16));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("foo::bar::C"))).first();
    compareUses(dec, RangeInRevision(7, 24, 7, 25));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("verylong::a"))).first();
    compareUses(dec, RangeInRevision(8, 10, 8, 11));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("verylong::b"))).first();
    compareUses(dec, RangeInRevision(8, 19, 8, 20));

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("verylong::C"))).first();
    compareUses(dec, RangeInRevision(8, 30, 8, 31));
}

void TestUses::lateStatic()
{
    //                         0         1         2         3         4         5         6         7
    //                         01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php class a { function b() { static::b(); } }", DumpAll);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    compareUses(top->childContexts().first()->localDeclarations().first(), RangeInRevision(0, 39, 0, 40));
}

void TestUses::closures()
{
    //                         0         1         2         3         4         5         6         7
    //                         01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php $a = 1; $b = 2;\n"
                              "$l = function($b) use ($a) { return $a - $b; };\n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->localDeclarations().count(), 4);

    Declaration* a = top->localDeclarations().at(0);
    QCOMPARE(a->identifier().toString(), QString("a"));
    compareUses(a, QList<RangeInRevision>() << RangeInRevision(1, 23, 1, 25) << RangeInRevision(1, 36, 1, 38));

    Declaration* b = top->localDeclarations().at(1);
    QCOMPARE(b->identifier().toString(), QString("b"));
    QVERIFY(b->uses().isEmpty());
}

void TestUses::instanceof()
{
    //                         0         1         2         3         4         5
    //                         012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php class a {}\n"
                              "$a = new a;\n"
                              "$b = $a instanceof a;\n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->localDeclarations().count(), 3);

    Declaration* a = top->localDeclarations().at(0);
    QCOMPARE(a->identifier().toString(), QString("a"));
    compareUses(a, QList<RangeInRevision>()
                    << RangeInRevision(1, 9, 1, 10)
                    << RangeInRevision(2, 19, 2, 20));

}

void TestUses::classNameString()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class Foo { } echo 'Foo';");
    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());


    Declaration* foo = top->localDeclarations().at(0);
    QCOMPARE(foo->identifier().toString(), QString("foo"));
    compareUses(foo, RangeInRevision(0, 22, 0, 27));
}

void TestUses::useTrait()
{
    //                         0         1         2         3         4         5         6         7
    //                         01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "trait A { public function one(){} public function two(){} }\n"
                              "trait B { public function one(){} private function three(){} }\n"
                              "trait C { public $baz; public function one(){} public static function five(){} }\n"
                              "class Foo { use A;}\n"
                              "class Bar { use A,B { A::one insteadof B; B::three as public four; } }\n"
                              "class Baz { use A,B,C { C::one insteadof A,B; C::five as six; } }\n"
                              "$a = new Foo(); $a->one(); $a->two();\n"
                              "$b = new Bar(); $b->one(); $b->two(); $b->four();\n"
                              "$c = new Baz(); $c->one(); $c->two(); $c->baz; $c::six();\n", DumpAll);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    Declaration* dec;
    QVector<Declaration*> topDecs = top->localDeclarations();
    TraitMethodAliasDeclaration* method;
    TraitMemberAliasDeclaration* member;

    QCOMPARE(topDecs.size(), 9);

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("a"))).first();
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(4, 16, 4, 17)
                                          << RangeInRevision(5, 16, 5, 17)
                                          << RangeInRevision(5, 22, 5, 23)
                                          << RangeInRevision(6, 16, 6, 17)
                                          << RangeInRevision(6, 41, 6, 42) );

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("b"))).first();
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(5, 18, 5, 19)
                                          << RangeInRevision(5, 39, 5, 40)
                                          << RangeInRevision(5, 42, 5, 43)
                                          << RangeInRevision(6, 18, 6, 19)
                                          << RangeInRevision(6, 43, 6, 44) );

    dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("c"))).first();
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(6, 20, 6, 21)
                                          << RangeInRevision(6, 24, 6, 25)
                                          << RangeInRevision(6, 46, 6, 47) );

    dec = topDecs[3]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("one"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("a"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(7, 20, 7, 23) );

    dec = topDecs[3]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("two"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("a"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(7, 31, 7, 34) );

    dec = topDecs[4]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("one"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("a"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(8, 20, 8, 23) );

    dec = topDecs[4]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("two"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("a"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(8, 31, 8, 34) );

    dec = topDecs[4]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("four"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("b"));
    QCOMPARE(method->aliasedDeclaration().data()->identifier(), Identifier("three"));
    QCOMPARE(method->accessPolicy(), Declaration::AccessPolicy::Public);
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(8, 42, 8, 46) );

    dec = topDecs[5]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("one"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("c"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(9, 20, 9, 23) );

    dec = topDecs[5]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("two"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("a"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(9, 31, 9, 34) );

    dec = topDecs[5]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("baz"))).first();
    member = dynamic_cast<TraitMemberAliasDeclaration*>(dec);
    QVERIFY(member);
    QCOMPARE(member->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("c"));
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(9, 42, 9, 45) );

    dec = topDecs[5]->internalContext()->findLocalDeclarations(Identifier(QStringLiteral("six"))).first();
    method = dynamic_cast<TraitMethodAliasDeclaration*>(dec);
    QVERIFY(method);
    QCOMPARE(method->aliasedDeclaration().data()->context()->owner()->identifier(), Identifier("c"));
    QCOMPARE(method->aliasedDeclaration().data()->identifier(), Identifier("five"));
    QVERIFY(method->isStatic());
    compareUses(dec, QList<RangeInRevision>() << RangeInRevision(9, 51, 9, 54) );
}

void TestUses::exceptionFinally()
{
    //                 0         1         2         3         4
    //                 01234567890123456789012345678901234567890123456
    QByteArray method("<? $a = 0; try { $a = 2; } finally { $a = 3; }");
    TopDUContext *top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration *a = top->localDeclarations().at(0);
    QCOMPARE(a->identifier().toString(), QString("a"));
    compareUses(a, QList<RangeInRevision>() << RangeInRevision(0, 17, 0, 19)
            << RangeInRevision(0, 37, 0, 39));
}

}

