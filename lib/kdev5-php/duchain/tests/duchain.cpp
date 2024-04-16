/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "duchain.h"

#include <QtTest/QtTest>

#include <language/duchain/parsingenvironment.h>
#include <language/duchain/problem.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/integraltype.h>
#include <language/duchain/types/unsuretype.h>
#include <language/duchain/namespacealiasdeclaration.h>

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/icompletionsettings.h>

#include "helper.h"

#include "../declarations/classdeclaration.h"
#include "../declarations/classmethoddeclaration.h"
#include "../declarations/functiondeclaration.h"
#include "../declarations/variabledeclaration.h"

#include "../types/structuretype.h"
#include "../types/integraltypeextended.h"

#include <QStandardPaths>

using namespace KDevelop;
using namespace Php;

QTEST_MAIN(Php::TestDUChain)

TestDUChain::TestDUChain()
{
}

void TestDUChain::declareFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo() {}");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->localDeclarations().count(), 1);

    Declaration* dec = top->localDeclarations().at(0);
    QVERIFY(dec);
    QCOMPARE(dec->context(), top);
    QCOMPARE(dec->internalContext(), top->childContexts().at(1));

    // no return means void as return type
    FunctionType::Ptr ftype = FunctionType::Ptr::dynamicCast(dec->abstractType());
    QVERIFY(ftype);
    IntegralType::Ptr itype = IntegralType::Ptr::dynamicCast(ftype->returnType());
    QVERIFY(itype->dataType() == IntegralType::TypeVoid);


    QCOMPARE(top->childContexts().at(0)->type(), DUContext::Function);
    QCOMPARE(top->childContexts().at(1)->type(), DUContext::Other);
}

void TestDUChain::declareVar()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} class B {} $i = new A(); $j = new B(); $i = new B(); $i = 'foo';"
                        " $a = substr($i, 0, 1);");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->localDeclarations().count(), 5);

    //class A
    Declaration* dec = top->localDeclarations().at(0);
    QCOMPARE(dec->uses().count(), 1);
    QCOMPARE(dec->uses().begin()->count(), 1);

    //$i
    Declaration* decVar = top->localDeclarations().at(2);
    QCOMPARE(decVar->identifier(), Identifier("i"));
    qDebug() << decVar->abstractType()->toString();
    UnsureType::Ptr unsureType = decVar->type<UnsureType>();
    QVERIFY(unsureType);
    QCOMPARE(unsureType->typesSize(), 3u);
    // = new A();
    QCOMPARE(unsureType->types()[0].abstractType().cast<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("a"));
    QVERIFY(unsureType->types()[0].abstractType()->equals(dec->abstractType().data()));
    // = new B();
    //class B
    dec = top->localDeclarations().at(1);
    QCOMPARE(dec->uses().count(), 1);
    QCOMPARE(dec->uses().begin()->count(), 2);
    QCOMPARE(unsureType->types()[1].abstractType().cast<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QVERIFY(unsureType->types()[1].abstractType()->equals(dec->abstractType().data()));
    // = 'foo';
    QVERIFY(unsureType->types()[2].abstractType().cast<IntegralType>());
    QVERIFY(unsureType->types()[2].abstractType().cast<IntegralType>()->dataType() == IntegralType::TypeString);

    //$j
    decVar = top->localDeclarations().at(3);
    QCOMPARE(decVar->identifier(), Identifier("j"));
    StructureType::Ptr classType = decVar->type<StructureType>();
    QVERIFY(classType);
    QCOMPARE(classType->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QVERIFY(classType->equals(dec->abstractType().data()));

    // $a
    decVar = top->localDeclarations().at(4);
    QCOMPARE(decVar->identifier(), Identifier("a"));
    QVERIFY(decVar->type<IntegralType>());
}

void TestDUChain::varTypehint()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} /** @var A **/ $i = foo();");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    //class A
    Declaration* dec = top->localDeclarations().at(0);

    //$i
    Declaration* decVar = top->localDeclarations().at(1);
    QCOMPARE(decVar->identifier(), Identifier("i"));
    StructureType::Ptr classType = decVar->type<StructureType>();
    QVERIFY(classType);
    QCOMPARE(classType->qualifiedIdentifier(), QualifiedIdentifier("a"));
    QVERIFY(classType->equals(dec->abstractType().data()));
}

void TestDUChain::declareClass()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public function foo($i) {} protected static function bar() {} private function baz() {} function boo() {} }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 1);

    DUContext* contextClassA = top->childContexts().first();


    QCOMPARE(top->localDeclarations().count(), 1);
    Declaration* dec = top->localDeclarations().first();
    QCOMPARE(dec->kind(), Declaration::Type);
    QCOMPARE(dec->toString(), QString("class A"));
    QCOMPARE(dec->qualifiedIdentifier(), QualifiedIdentifier("a"));
    QCOMPARE(dec->isDefinition(), true);
    QCOMPARE(dec->logicalInternalContext(top), contextClassA);

    qDebug() << contextClassA->localScopeIdentifier().toString();
    QCOMPARE(contextClassA->localScopeIdentifier(), QualifiedIdentifier("a"));
    QCOMPARE(contextClassA->childContexts().count(), 8);
    QCOMPARE(contextClassA->childContexts().first()->localScopeIdentifier(), QualifiedIdentifier("foo"));

    DUContext* contextMethodBodyFoo = contextClassA->childContexts().at(1);
    QCOMPARE(contextMethodBodyFoo->localScopeIdentifier(), QualifiedIdentifier("foo"));
    QCOMPARE(contextMethodBodyFoo->importedParentContexts().count(), 1);
    QCOMPARE(contextMethodBodyFoo->childContexts().count(), 0);
    QVERIFY(contextMethodBodyFoo->importedParentContexts().first().context(top) ==
            contextClassA->childContexts().first());

    //foo()
    dec = contextClassA->localDeclarations().at(0);
    ClassFunctionDeclaration* funDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
    QVERIFY(funDec);
    QCOMPARE(funDec->kind(), Declaration::Type);
    QCOMPARE(funDec->identifier(), Identifier("foo"));
    QCOMPARE(funDec->accessPolicy(), Declaration::Public);
    QCOMPARE(funDec->isStatic(), false);

    {
        // no return means void as return type
        FunctionType::Ptr ftype = FunctionType::Ptr::dynamicCast(dec->abstractType());
        QVERIFY(ftype);
        IntegralType::Ptr itype = IntegralType::Ptr::dynamicCast(ftype->returnType());
        QVERIFY(itype->dataType() == IntegralType::TypeVoid);
    }

    //bar()
    dec = contextClassA->localDeclarations().at(1);
    funDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
    QVERIFY(funDec);
    QCOMPARE(funDec->identifier(), Identifier("bar"));
    QCOMPARE(funDec->accessPolicy(), Declaration::Protected);
    QCOMPARE(funDec->isStatic(), true);

    //baz()
    dec = contextClassA->localDeclarations().at(2);
    funDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
    QVERIFY(funDec);
    QCOMPARE(funDec->identifier(), Identifier("baz"));
    QCOMPARE(funDec->accessPolicy(), Declaration::Private);
    QCOMPARE(funDec->isStatic(), false);

    //boo()
    dec = contextClassA->localDeclarations().at(3);
    funDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
    QVERIFY(funDec);
    QCOMPARE(funDec->identifier(), Identifier("boo"));
    QCOMPARE(funDec->accessPolicy(), Declaration::Public);
}

void TestDUChain::classMemberVar()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public $foo; /** @var A **/ protected $bar; private static $baz = ''; var $boo = 1; }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 1);

    DUContext* contextClassA = top->childContexts().first();

    QCOMPARE(top->localDeclarations().count(), 1);
    Declaration* dec = top->localDeclarations().first();
    QCOMPARE(dec->qualifiedIdentifier(), QualifiedIdentifier("a"));
    QCOMPARE(dec->isDefinition(), true);
    QCOMPARE(dec->logicalInternalContext(top), contextClassA);

    QCOMPARE(contextClassA->localScopeIdentifier(), QualifiedIdentifier("a"));
    QCOMPARE(contextClassA->childContexts().count(), 0);
    QCOMPARE(contextClassA->localDeclarations().count(), 4);

    //$foo
    ClassMemberDeclaration* var = dynamic_cast<ClassMemberDeclaration*>(contextClassA->localDeclarations().first());
    QVERIFY(var);
    QCOMPARE(var->identifier(), Identifier("foo"));
    QCOMPARE(var->accessPolicy(), Declaration::Public);
    QCOMPARE(var->isStatic(), false);
    QVERIFY(var->type<IntegralType>());
    QVERIFY(var->type<IntegralType>()->dataType() == IntegralType::TypeMixed);

    //$bar
    var = dynamic_cast<ClassMemberDeclaration*>(contextClassA->localDeclarations().at(1));
    QVERIFY(var);
    QCOMPARE(var->identifier(), Identifier("bar"));
    QCOMPARE(var->accessPolicy(), Declaration::Protected);
    QCOMPARE(var->isStatic(), false);
    StructureType::Ptr type = var->type<StructureType>();
    QVERIFY(type);
    QCOMPARE(type->qualifiedIdentifier(), QualifiedIdentifier("a"));

    //$baz
    var = dynamic_cast<ClassMemberDeclaration*>(contextClassA->localDeclarations().at(2));
    QVERIFY(var);
    QCOMPARE(var->identifier(), Identifier("baz"));
    QCOMPARE(var->accessPolicy(), Declaration::Private);
    QCOMPARE(var->isStatic(), true);
    QVERIFY(var->type<IntegralType>());
    QVERIFY(var->type<IntegralType>()->dataType() == IntegralType::TypeString);

    //$boo
    var = dynamic_cast<ClassMemberDeclaration*>(contextClassA->localDeclarations().at(3));
    QVERIFY(var);
    QCOMPARE(var->identifier(), Identifier("boo"));
    QCOMPARE(var->accessPolicy(), Declaration::Public);
    QCOMPARE(var->isStatic(), false);
    QVERIFY(var->type<IntegralType>());
    QVERIFY(var->type<IntegralType>()->dataType() == IntegralType::TypeInt);
}

void TestDUChain::returnTypeClass()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} function foo() { return new A(); } function bar() { $i = new A(); return $i; }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 5);
    QCOMPARE(top->localDeclarations().count(), 3);

    Declaration* dec = top->localDeclarations().at(1);
    QCOMPARE(dec->qualifiedIdentifier(), QualifiedIdentifier("foo"));
    FunctionType::Ptr functionType = dec->type<FunctionType>();
    QVERIFY(functionType);
    StructureType::Ptr retType = StructureType::Ptr::dynamicCast(functionType->returnType());
    QVERIFY(retType);
    QCOMPARE(retType->qualifiedIdentifier(), QualifiedIdentifier("a"));

    dec = top->localDeclarations().at(2);
    QCOMPARE(dec->qualifiedIdentifier(), QualifiedIdentifier("bar"));
    functionType = dec->type<FunctionType>();
    QVERIFY(functionType);
    retType = StructureType::Ptr::dynamicCast(functionType->returnType());
    QVERIFY(retType);
    QCOMPARE(retType->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestDUChain::declarationReturnType()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} function foo() { return new A(); } $i = foo(); ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 3);
    QCOMPARE(top->localDeclarations().count(), 3);

    Declaration* dec = top->localDeclarations().at(1);
    FunctionType::Ptr fType = dec->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(StructureType::Ptr::dynamicCast(fType->returnType()));
    QCOMPARE(StructureType::Ptr::dynamicCast(fType->returnType())->qualifiedIdentifier(), QualifiedIdentifier("a"));

    dec = top->localDeclarations().at(2);
    QCOMPARE(dec->identifier(), Identifier("i"));
    StructureType::Ptr type = dec->type<StructureType>();
    QVERIFY(type);
    QCOMPARE(type->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestDUChain::declarationReturnTypeInRecursingFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} /** @return A **/ function foo() { $i = foo(); } ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList< Declaration* > decs = top->childContexts().last()->findDeclarations(Identifier(QStringLiteral("i")));
    QCOMPARE(decs.size(), 1);
    Declaration* dec = decs.first();
    StructureType::Ptr type = dec->type<StructureType>();
    QVERIFY(type);
    QCOMPARE(type->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestDUChain::declarationMultipleReturnTypes()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} function foo() { return null; return new A(); return null; }\n"
                      "function bar() { return 1; return 2; }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fType = top->localDeclarations().at(1)->type<FunctionType>();
    QVERIFY(fType);
    qDebug() << fType->toString();
    TypePtr<UnsureType> ut = UnsureType::Ptr::dynamicCast(fType->returnType());
    QVERIFY(ut);
    QCOMPARE(2u, ut->typesSize());

    ///TODO: why are the types not in the correct order, i.e. null, A
    QVERIFY(ut->types()[0].type<StructureType>());
    QVERIFY(ut->types()[0].type<StructureType>()->declaration(top));
    QCOMPARE(ut->types()[0].type<StructureType>()->declaration(top)->qualifiedIdentifier(), QualifiedIdentifier("a"));

    QVERIFY(ut->types()[1].type<IntegralType>());
    QVERIFY(ut->types()[1].type<IntegralType>()->dataType() == IntegralType::TypeNull);

    fType = top->localDeclarations().at(2)->type<FunctionType>();
    QVERIFY(fType);
    qDebug() << fType->toString();
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType()));
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType())->dataType() == IntegralType::TypeInt);
}

void TestDUChain::returnTypeViaMember()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class a { /** @return a **/ function fa() {} }\n"
                      "class b { /** @var a **/ static $astatic; /** @var a **/ var $anormal;\n"
                      "  function fb1($param) { $i = self::$astatic->fa($param); }\n"
                      "  function fb2($param) { $i = $this->anormal->fa($param); } }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVector<Declaration*> decs = top->localDeclarations();
    QCOMPARE(decs.size(), 2);

    ClassDeclaration* aDec = dynamic_cast<ClassDeclaration*>(decs.first());
    QVERIFY(aDec);

    ClassDeclaration* bDec = dynamic_cast<ClassDeclaration*>(decs.last());
    QVERIFY(bDec);
    QCOMPARE(bDec->logicalInternalContext(top)->localDeclarations().size(), 4);

    typedef QPair<QString, QString> idPair;
    Q_FOREACH ( const idPair & pair, QList< idPair >()
                                        << qMakePair(QString("fb1"), QString("astatic"))
                                        << qMakePair(QString("fb2"), QString("anormal")) )
    {
        qDebug() << pair.first << pair.second;
        ClassMethodDeclaration* fDec = dynamic_cast<ClassMethodDeclaration*>(
            bDec->logicalInternalContext(top)->findDeclarations(Identifier(pair.first)).first()
        );
        QVERIFY(fDec);

        ClassMemberDeclaration* mDec = dynamic_cast<ClassMemberDeclaration*>(
            bDec->logicalInternalContext(top)->findDeclarations(Identifier(pair.second)).first()
        );
        QVERIFY(mDec);
        QVERIFY(mDec->type<StructureType>());
        QCOMPARE(mDec->type<StructureType>()->declaration(top), aDec);

        QCOMPARE(fDec->logicalInternalContext(top)->localDeclarations().size(), 1);
        Declaration* iDec = fDec->logicalInternalContext(top)->localDeclarations().first();
        QCOMPARE(iDec->identifier().toString(), QString("i"));
        QVERIFY(iDec->type<StructureType>());
        QCOMPARE(iDec->type<StructureType>()->declaration(top), aDec);
    }
}

void TestDUChain::declarationReturnTypeDocBlock()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { /** @return A **/ function bar() {} } "
                      "   class B {} "
                      "/** @return A **/ function foo() { return new B(); } "
                      "/** @return object **/ function bar() {}");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    //function bar
    Declaration* dec = top->childContexts().at(0)->localDeclarations().at(0);
    FunctionType::Ptr fType = dec->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(StructureType::Ptr::dynamicCast(fType->returnType()));
    QCOMPARE(StructureType::Ptr::dynamicCast(fType->returnType())->qualifiedIdentifier(), QualifiedIdentifier("a"));

    //function foo
    dec = top->localDeclarations().at(2);
    fType = dec->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(StructureType::Ptr::dynamicCast(fType->returnType()));
    QCOMPARE(StructureType::Ptr::dynamicCast(fType->returnType())->qualifiedIdentifier(), QualifiedIdentifier("a"));

    //function bar
    dec = top->localDeclarations().at(3);
    fType = dec->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(StructureType::Ptr::dynamicCast(fType->returnType()));
    QCOMPARE(StructureType::Ptr::dynamicCast(fType->returnType())->qualifiedIdentifier(), QualifiedIdentifier("stdclass"));

    //test hint in internal functions file of a type that is added later on
    // function
    QList<Declaration*> decs = top->findDeclarations(Identifier(QStringLiteral("should_return_exception")));
    QCOMPARE(decs.size(), 1);
    dec = decs.first();
    fType = dec->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(StructureType::Ptr::dynamicCast(fType->returnType()));
    QCOMPARE(StructureType::Ptr::dynamicCast(fType->returnType())->qualifiedIdentifier(), QualifiedIdentifier("exception"));
    // method
    decs = top->findDeclarations(Identifier(QStringLiteral("internal_test_class")));
    QCOMPARE(decs.size(), 1);
    ClassDeclaration* cdec = dynamic_cast<ClassDeclaration*>(decs.first());
    QVERIFY(cdec);
    decs = cdec->logicalInternalContext(top)->findDeclarations(Identifier(QStringLiteral("should_return_exception")));
    QCOMPARE(decs.size(), 1);
    dec = decs.first();
    fType = dec->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(StructureType::Ptr::dynamicCast(fType->returnType()));
    QCOMPARE(StructureType::Ptr::dynamicCast(fType->returnType())->qualifiedIdentifier(), QualifiedIdentifier("exception"));
}

void TestDUChain::declarationReturnTypeDocBlockIntegral()
{
    QByteArray method("<? /** @return string **/ function foo() {} /** @return mixed **/ function bar() {} class A { /** @return int **/ function aaa() {} }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    //function foo
    FunctionType::Ptr fType = top->localDeclarations().at(0)->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType()));
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType())->dataType() == IntegralType::TypeString);

    //function bar
    fType = top->localDeclarations().at(1)->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType()));
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType())->dataType() == IntegralType::TypeMixed);

    //function aaa
    fType = top->childContexts().at(4)->localDeclarations().first()->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType()));
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType())->dataType() == IntegralType::TypeInt);
}

void TestDUChain::declarationReturnTypeClassChain()
{
    QByteArray method("<? class A { /** @return this **/ function a() {} /** @return self **/ function b() {} }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    // class a
    DUContext* ctx = top->childContexts().first();
    QCOMPARE(ctx->type(), DUContext::Class);
    QVERIFY(ctx->owner());
    QVERIFY(StructureType::Ptr::dynamicCast(ctx->owner()->abstractType()));

    //function a
    // FIXME
    QEXPECT_FAIL("", "This test fails after porting the plugin to KF5.", Abort);
    QVERIFY(/* func a (this) */ ctx->localDeclarations().at(0)->type<FunctionType>().data() == ctx->owner()->abstractType().data());
    QVERIFY(/* func b (self) */ ctx->localDeclarations().at(1)->type<FunctionType>().data() == ctx->owner()->abstractType().data());
}

void TestDUChain::declareTypehintFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A {} function foo(A $i) { return $i; } ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 3);
    QCOMPARE(top->localDeclarations().count(), 2);

    Declaration* dec = top->localDeclarations().at(0);
    QCOMPARE(dec->internalContext(), top->childContexts().at(0));
    QCOMPARE(dec->uses().count(), 1);
    QCOMPARE(dec->uses().begin()->count(), 1);

    QCOMPARE(top->childContexts().at(0)->localScopeIdentifier(), QualifiedIdentifier("a"));
    QCOMPARE(top->childContexts().at(0)->childContexts().count(), 0);

    DUContext* contextFunctionFoo = top->childContexts().at(1);
    QCOMPARE(contextFunctionFoo->localScopeIdentifier(), QualifiedIdentifier("foo"));

    DUContext* contextFunctionBodyFoo = top->childContexts().at(2);
    QCOMPARE(contextFunctionBodyFoo->localScopeIdentifier(), QualifiedIdentifier("foo"));
    QCOMPARE(contextFunctionBodyFoo->importedParentContexts().count(), 1);
    QCOMPARE(contextFunctionBodyFoo->childContexts().count(), 0);

    QVERIFY(contextFunctionBodyFoo->importedParentContexts().first().context(top) ==
            contextFunctionFoo);

    QVERIFY(top->childContexts().at(1)->localDeclarations().first()->type<StructureType>());
    QCOMPARE(top->childContexts().at(1)->localDeclarations().first()->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("a"));

    FunctionType::Ptr fType = top->localDeclarations().at(1)->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(StructureType::Ptr::dynamicCast(fType->returnType()));
    QCOMPARE(StructureType::Ptr::dynamicCast(fType->returnType())->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestDUChain::declareTypehintArrayFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo(array $i) { } ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fun = top->localDeclarations().first()->type<FunctionType>();
    QVERIFY(fun);
    QCOMPARE(fun->arguments().count(), 1);
    QVERIFY(IntegralType::Ptr::dynamicCast(fun->arguments().first()));
    QVERIFY(IntegralType::Ptr::dynamicCast(fun->arguments().first())->dataType() == IntegralType::TypeArray);

    IntegralType::Ptr type = top->childContexts().first()->localDeclarations().first()->type<IntegralType>();
    QVERIFY(type);
    QVERIFY(type->dataType() == IntegralType::TypeArray);
}

void TestDUChain::declareTypehintCallableFunction()
{
    //                 0         1         2         3
    //                 0123456789012345678901234567890123
    QByteArray method("<? function foo(callable $i) { } ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fun = top->localDeclarations().first()->type<FunctionType>();
    QVERIFY(fun);
    QCOMPARE(fun->arguments().count(), 1);
    QVERIFY(IntegralType::Ptr::dynamicCast(fun->arguments().first()));
    QVERIFY(IntegralType::Ptr::dynamicCast(fun->arguments().first())->dataType() == IntegralType::TypeMixed);

    IntegralType::Ptr type = top->childContexts().first()->localDeclarations().first()->type<IntegralType>();
    QVERIFY(type);
    QVERIFY(type->dataType() == IntegralType::TypeMixed);
}

void TestDUChain::classImplementsInterface()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? interface I { } class A implements I { }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->localDeclarations().count(), 2);

    //interface I
    Declaration* dec = top->localDeclarations().at(0);
    QVERIFY(dec->isDefinition());
    QCOMPARE(dec->identifier(), Identifier("i"));
    QCOMPARE(dec->toString(), QString("interface I"));
    StructureType::Ptr typeI = dec->type<StructureType>();
    QCOMPARE(typeI->qualifiedIdentifier(), QualifiedIdentifier("i"));
    QVERIFY(typeI->declaration(top) == dec);
    ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(dec);
    QVERIFY(classDec);
    QCOMPARE(classDec->classType(), ClassDeclarationData::Interface);

    QCOMPARE(dec->internalContext(), top->childContexts().at(0));
    QCOMPARE(dec->internalContext()->childContexts().count(), 0);
    QCOMPARE(dec->internalContext()->importedParentContexts().count(), 0);
    QCOMPARE(dec->internalContext()->localScopeIdentifier(), QualifiedIdentifier("i"));

    QCOMPARE(dec->uses().count(), 1);
    QCOMPARE(dec->uses().begin()->count(), 1);

    IndexedType indexedTypeI = classDec->indexedType();

    //class A
    dec = top->localDeclarations().at(1);
    QVERIFY(dec->isDefinition());
    QCOMPARE(dec->identifier(), Identifier("a"));
    StructureType::Ptr typeA = dec->type<StructureType>();
    QCOMPARE(typeA->qualifiedIdentifier(), QualifiedIdentifier("a"));
    QVERIFY(typeA->declaration(top) == dec);
    classDec = dynamic_cast<ClassDeclaration*>(dec);
    QVERIFY(classDec);
    QCOMPARE(classDec->classType(), ClassDeclarationData::Class);

    QCOMPARE(dec->internalContext(), top->childContexts().at(1));
    QCOMPARE(dec->internalContext()->childContexts().count(), 0);
    QCOMPARE(dec->internalContext()->localScopeIdentifier(), QualifiedIdentifier("a"));
    //class A imports interface I context
    QCOMPARE(dec->internalContext()->importedParentContexts().count(), 1);
    QVERIFY(dec->internalContext()->importedParentContexts().at(0).context(top) == top->childContexts().at(0));

    QCOMPARE(classDec->baseClassesSize(), 1u);
    QCOMPARE(classDec->baseClasses()[0].baseClass, indexedTypeI);

    QCOMPARE(dec->uses().count(), 0);
}

void TestDUChain::classExtends()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { } class B extends A { } ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());
    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->localDeclarations().count(), 2);

    //class A
    Declaration* dec = top->localDeclarations().at(0);
    QVERIFY(dec->isDefinition());
    QCOMPARE(dec->identifier(), Identifier("a"));
    StructureType::Ptr typeA = dec->type<StructureType>();
    QCOMPARE(typeA->qualifiedIdentifier(), QualifiedIdentifier("a"));
    QVERIFY(typeA->declaration(top) == dec);
    ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(dec);
    QVERIFY(classDec);
    QCOMPARE(classDec->classType(), ClassDeclarationData::Class);

    QCOMPARE(dec->internalContext(), top->childContexts().at(0));
    QCOMPARE(dec->internalContext()->childContexts().count(), 0);
    QCOMPARE(dec->internalContext()->importedParentContexts().count(), 0);
    QCOMPARE(dec->internalContext()->localScopeIdentifier(), QualifiedIdentifier("a"));

    QCOMPARE(dec->uses().count(), 1);
    QCOMPARE(dec->uses().begin()->count(), 1);

    IndexedType indexedTypeA = classDec->indexedType();

    //class B
    dec = top->localDeclarations().at(1);
    QVERIFY(dec->isDefinition());
    QCOMPARE(dec->identifier(), Identifier("b"));
    StructureType::Ptr typeB = dec->type<StructureType>();
    QCOMPARE(typeB->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QVERIFY(typeB->declaration(top) == dec);
    classDec = dynamic_cast<ClassDeclaration*>(dec);
    QVERIFY(classDec);
    QCOMPARE(classDec->classType(), ClassDeclarationData::Class);

    QCOMPARE(dec->internalContext(), top->childContexts().at(1));
    QCOMPARE(dec->internalContext()->childContexts().count(), 0);
    QCOMPARE(dec->internalContext()->localScopeIdentifier(), QualifiedIdentifier("b"));
    //class B imports class A context
    QCOMPARE(dec->internalContext()->importedParentContexts().count(), 1);
    QVERIFY(dec->internalContext()->importedParentContexts().at(0).context(top) == top->childContexts().at(0));

    QCOMPARE(classDec->baseClassesSize(), 1u);
    QCOMPARE(classDec->baseClasses()[0].baseClass, indexedTypeA);

    QCOMPARE(dec->uses().count(), 0);
}


void TestDUChain::staticMethod()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B {} class A { static function foo() { return new B(); } } $i = A::foo();");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    StructureType::Ptr type = top->localDeclarations().at(2)->type<StructureType>();
    QVERIFY(type);
    QCOMPARE(type->qualifiedIdentifier(), QualifiedIdentifier("b"));
}

void TestDUChain::ownStaticMethod()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B {} class A { static function foo() { return new B(); } function bar() { $i = self::foo(); $j = A::foo(); } } ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->childContexts().at(1));
    QVERIFY(top->childContexts().at(1)->localDeclarations().at(0));
    QVERIFY(top->childContexts().at(1)->localDeclarations().at(0)->type<FunctionType>());
    AbstractType::Ptr ret = top->childContexts().at(1)->localDeclarations().at(0)
                            ->type<FunctionType>()->returnType();

    QVERIFY(StructureType::Ptr::dynamicCast(ret));
    QCOMPARE(StructureType::Ptr::dynamicCast(ret)->declaration(top), top->localDeclarations().at(0));

    QVERIFY(top->childContexts().at(1)->childContexts().at(1 + 2));
    QVERIFY(top->childContexts().at(1)->childContexts().at(1 + 2)->localDeclarations().at(0));
    QVERIFY(top->childContexts().at(1)->childContexts().at(1 + 2)->localDeclarations().at(0)->type<StructureType>());
    QCOMPARE(top->childContexts().at(1)->childContexts().at(1 + 2)->localDeclarations().at(0)
             ->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QCOMPARE(top->childContexts().at(1)->childContexts().at(1 + 2)->localDeclarations().at(1)
             ->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
}
void TestDUChain::thisVar()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function x() { return $this; } function y() { return $this->x(); } } ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fn = top->childContexts().at(0)->localDeclarations().at(0)->type<FunctionType>();
    QVERIFY(fn);
    StructureType::Ptr cls = StructureType::Ptr::dynamicCast(fn->returnType());
    QVERIFY(cls);
    QCOMPARE(cls->qualifiedIdentifier(), QualifiedIdentifier("a"));

    fn = top->childContexts().at(0)->localDeclarations().at(1)->type<FunctionType>();
    QVERIFY(fn);
    cls = StructureType::Ptr::dynamicCast(fn->returnType());
    QVERIFY(cls);
    QCOMPARE(cls->qualifiedIdentifier(), QualifiedIdentifier("a"));
}

void TestDUChain::objectFunctionCall()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B {} class A { function x() { return new B(); } function y() { $a = new A(); return $a->x(); } } ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fn = top->childContexts().at(1)->localDeclarations().at(0)->type<FunctionType>();
    QVERIFY(fn);
    StructureType::Ptr cls = StructureType::Ptr::dynamicCast(fn->returnType());
    QVERIFY(cls);
    QCOMPARE(cls->qualifiedIdentifier(), QualifiedIdentifier("b"));

    fn = top->childContexts().at(1)->localDeclarations().at(1)->type<FunctionType>();
    QVERIFY(fn);
    cls = StructureType::Ptr::dynamicCast(fn->returnType());
    QVERIFY(cls);
    QCOMPARE(cls->qualifiedIdentifier(), QualifiedIdentifier("b"));
}

void TestDUChain::objectFunctionCall2()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class C {} class B { function c() { return new C(); } } class A { function x() { return new B(); } function y() { $a = new A(); return $a->x()->c(); } } ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fn = top->childContexts().at(2)->localDeclarations().at(1)->type<FunctionType>();
    QVERIFY(fn);
    StructureType::Ptr cls = StructureType::Ptr::dynamicCast(fn->returnType());
    QVERIFY(cls);
    QCOMPARE(cls->qualifiedIdentifier(), QualifiedIdentifier("c"));
}

void TestDUChain::objectFunctionCall3()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B {} class A { function b() { return new B(); } } $i = new A(); $j = $i->b();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().at(2)->qualifiedIdentifier(), QualifiedIdentifier("i"));
    QCOMPARE(top->localDeclarations().at(2)->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("a"));;
    QCOMPARE(top->localDeclarations().at(3)->qualifiedIdentifier(), QualifiedIdentifier("j"));
    QCOMPARE(top->localDeclarations().at(3)->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));;
}

void TestDUChain::objectVariable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B {} class A { /** @var B **/ public $foo; } $a = new A(); $i = $a->foo;");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().at(3)->qualifiedIdentifier(), QualifiedIdentifier("i"));
    QCOMPARE(top->localDeclarations().at(3)->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));;
}

void TestDUChain::staticMemberVariable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B {} class A { /** @var B **/ public static $foo; } $i = A::$foo;");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().at(2)->qualifiedIdentifier(), QualifiedIdentifier("i"));
    QCOMPARE(top->localDeclarations().at(2)->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));;
}
void TestDUChain::ownStaticMemberVariable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class B {} class A { /** @var B **/ public static $foo; function bar() { $i = self::$foo; $j = A::$foo; }} ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* barContext = top->childContexts().at(1)->childContexts().at(1);
    QCOMPARE(barContext->localDeclarations().at(0)->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QCOMPARE(barContext->localDeclarations().at(1)->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
}

void TestDUChain::classConst_data()
{
    QTest::addColumn<QString>("classBody");
    QTest::addColumn<int>("problems");

    QTest::newRow("int") << "const C = 1;" << 0;
    QTest::newRow("string") << "const C = 'asdf';" << 0;
    QTest::newRow("float") << "const C = 0.5;" << 0;
    QTest::newRow("bool") << "const C = true;" << 0;
    QTest::newRow("selfConst") << "const C2 = 1; const C = self::C2;" << 0;
    QTest::newRow("parentConst") << "const C = parent::P;" << 0;
    QTest::newRow("null") << "const C = null;" << 0;

    QTest::newRow("array") << "const C = array();" << 1;
}

void TestDUChain::classConst()
{
    QFETCH(QString, classBody);
    QFETCH(int, problems);

    QString fullClass("<? class B { const P = 1; } class A extends B { " + classBody + " } ");

    TopDUContext* top = parse(fullClass.toUtf8(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->problems().count(), problems);

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("a::C")).count(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("a::C")).first()->context(), top->childContexts().last());
}

void TestDUChain::fileConst_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<int>("problems");
    QTest::addColumn<uint>("dataType");

    QTest::newRow("int") << "const C = 1;" << 0 << (uint) IntegralType::TypeInt;
    QTest::newRow("string") << "const C = 'asdf';" << 0 << (uint) IntegralType::TypeString;
    QTest::newRow("float") << "const C = 0.5;" << 0 << (uint) IntegralType::TypeFloat;
    QTest::newRow("bool") << "const C = true;" << 0 << (uint) IntegralType::TypeBoolean;

    QTest::newRow("array") << "const C = array();" << 1 << (uint) IntegralType::TypeArray;
}

void TestDUChain::fileConst()
{
    QFETCH(QString, code);
    QFETCH(int, problems);
    QFETCH(uint, dataType);

    code.prepend("<?php ");

    TopDUContext* top = parse(code.toUtf8(), DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->problems().count(), problems);

    QList< Declaration* > decs = top->findDeclarations(QualifiedIdentifier(QStringLiteral("C")));
    QCOMPARE(decs.count(), 1);
    IntegralType::Ptr type = decs.first()->abstractType().cast<IntegralType>();
    QVERIFY(type);
    QCOMPARE(type->dataType(), dataType);
    QVERIFY(type->modifiers() & AbstractType::ConstModifier);
}

void TestDUChain::define()
{
    // the last define tests that we don't crash under that circumstance
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? define('FOO', 'foo'); function x() { define('BAR', 'bar'); } define(); ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("FOO")).count(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("BAR")).count(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("FOO")).first()->context(), top);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("BAR")).first()->context(), top);

    QVERIFY(top->findDeclarations(QualifiedIdentifier("FOO")).first()->abstractType()->modifiers() & AbstractType::ConstModifier);
    QVERIFY(top->findDeclarations(QualifiedIdentifier("BAR")).first()->abstractType()->modifiers() & AbstractType::ConstModifier);
}
void TestDUChain::defaultFunctionParam()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo($a, $b = false, $c = null) {} ");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    AbstractFunctionDeclaration* fun = dynamic_cast<AbstractFunctionDeclaration*>(top->localDeclarations().first());
    QVERIFY(fun);

    QCOMPARE(fun->defaultParametersSize(), 2u);
    QCOMPARE(fun->defaultParameters()[0].str(), QString("false"));
    QCOMPARE(fun->defaultParameters()[1].str(), QString("null"));
}

void TestDUChain::globalFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? substr(); ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().count(), 1);
    QVERIFY(DUChain::self()->chainForDocument(internalFunctionFile()));
    QCOMPARE(DUChain::self()->chainForDocument(internalFunctionFile()), top->importedParentContexts().first().context(top));

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("substr")).count(), 1);
}

void TestDUChain::globalVariableFromInternalFunctions()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? substr(); ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->importedParentContexts().count(), 1);
    QVERIFY(DUChain::self()->chainForDocument(internalFunctionFile()));
    QCOMPARE(DUChain::self()->chainForDocument(internalFunctionFile()), top->importedParentContexts().first().context(top));

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("_GET")).count(), 1);
}

void TestDUChain::newObjectFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo.php"), "<?php class Foo { } ");
    DUChainReleaser releaseAddTop(addTop);
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = new Foo(); ");
    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(hasImportedParentContext(top, addTop));

    QCOMPARE(top->localDeclarations().first()->type<StructureType>()->declaration(top),
             addTop->localDeclarations().first());
}

void TestDUChain::unknownReturnType()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo() {} $a = foo();");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    //function bar
    Declaration* dec = top->localDeclarations().at(0);
    FunctionType::Ptr fType = dec->type<FunctionType>();
    QVERIFY(fType);
    QVERIFY(IntegralType::Ptr::dynamicCast(fType->returnType()));
    QVERIFY(IntegralType::Ptr::staticCast(fType->returnType())->dataType() == IntegralType::TypeVoid);
}

void TestDUChain::staticFunctionCallFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo2.php"), "<?php class Foo { public static function a() {} } ");
    DUChainReleaser releaseAddTop(addTop);
    TopDUContext* top = parse("<? Foo::a(); ", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(hasImportedParentContext(top, addTop));
}

void TestDUChain::classConstantFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo2.php"), "<?php class Foo { const BAR = 0; } ");
    DUChainReleaser releaseAddTop(addTop);
    TopDUContext* top = parse("<? Foo::BAR; ", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(hasImportedParentContext(top, addTop));
}

void TestDUChain::globalFunctionCallFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo3.php"), "<?php function a() {} ");
    DUChainReleaser releaseAddTop(addTop);
    TopDUContext* top = parse("<? a(); ", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(hasImportedParentContext(top, addTop));
}

void TestDUChain::constantFromOtherFile()
{

    TopDUContext* addTop = parseAdditionalFile(IndexedString("/duchaintest/foo3.php"), "<?php define('A', 0); ");
    DUChainReleaser releaseAddTop(addTop);
    TopDUContext* top = parse("<? define('B', 0); A; ", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    QVERIFY(hasImportedParentContext(top, addTop));
}

void TestDUChain::singleton()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public static function self() { static $i; if(!$i) $i = new self(); return $i; }}");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    FunctionType::Ptr fun = top->childContexts().first()->localDeclarations().first()->type<FunctionType>();
    QVERIFY(fun);
    StructureType::Ptr ret = StructureType::Ptr::dynamicCast(fun->returnType());
    qDebug() << fun->returnType()->toString();
    QVERIFY(ret);
    QCOMPARE(ret->declaration(top), top->localDeclarations().first());
}

void TestDUChain::internalFunctions()
{
    return; //disabled because it is too slow
    QString fileName = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdevphpsupport/phpfunctions.php"));
    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    TopDUContext* top = parse(file.readAll(), DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
}

void TestDUChain::trueFalse()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = true; $b = false; ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->localDeclarations().at(0)->type<IntegralType>()->dataType() == IntegralType::TypeBoolean);
    QVERIFY(top->localDeclarations().at(1)->type<IntegralType>()->dataType() == IntegralType::TypeBoolean);
}

void TestDUChain::null()
{
    QByteArray method("<? $a = null; ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->localDeclarations().at(0)->type<IntegralType>()->dataType() == IntegralType::TypeNull);
}

void TestDUChain::array()
{
    QByteArray method("<? $a = array(); $b = array(1, 2, 3); $b[] = 'test';");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->localDeclarations().at(0)->type<IntegralType>()->dataType() == IntegralType::TypeArray);
    QVERIFY(top->localDeclarations().at(1)->type<IntegralType>()->dataType() == IntegralType::TypeArray);
    // $b[] = 'test'; is not a redeclaration of b! Esp. it's type should not change.
    QCOMPARE(top->findDeclarations(Identifier("b")).count(), 1);
}

void TestDUChain::functionDocBlock()
{
    {
        TopDUContext* top = parse("<? /**\n *Foo\n **/\nfunction foo() {} ", DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->localDeclarations().first()->comment(), QByteArray("Foo"));
    }

    {
        TopDUContext* top = parse("<? /**\n *Bar\n **/\nclass A { /**\n *Foo\n **/\nfunction foo() {} }", DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->localDeclarations().first()->comment(), QByteArray("Bar"));
        QCOMPARE(top->childContexts().first()->localDeclarations().first()->comment(), QByteArray("Foo"));
    }

    {
        TopDUContext* top = parse("<? /**\n *Foo\n **/\ninterface A { }", DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->localDeclarations().first()->comment(), QByteArray("Foo"));
    }

    {
        TopDUContext* top = parse("<? class A { /**\n *Foo\n **/\npublic $foo; }", DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->childContexts().first()->localDeclarations().first()->comment(), QByteArray("Foo"));
    }

    {
        TopDUContext* top = parse("<? class A { /**\n *Foo\n **/\nconst FOO=0; }", DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->childContexts().first()->localDeclarations().first()->comment(), QByteArray("Foo"));
    }

    {
        TopDUContext* top = parse("<?\n/// Foo\n/// Bar\nfunction foo() {} ", DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->localDeclarations().first()->comment(), QByteArray("Foo\n Bar"));
    }

    {
        // same as above but with indentation
        TopDUContext* top = parse("<?\n  /// Foo\n  /// Bar\n  function foo() {} ", DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->localDeclarations().first()->comment(), QByteArray("Foo\n Bar"));
    }
}

void TestDUChain::variableDocBlock()
{
    {
        TopDUContext* top = parse("<? /**\n *Foo\n **/\n$a = 0; /**\n *Foo\n **/\nstatic $b;", DumpAll);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->localDeclarations().first()->comment(), QByteArray("Foo"));
        QCOMPARE(top->localDeclarations().at(1)->comment(), QByteArray("Foo"));
    }
    {
        TopDUContext* top = parse("<? /// Foo\n$a = 0; /// Foo\nstatic $b;", DumpAll);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());
        QCOMPARE(top->localDeclarations().first()->comment(), QByteArray("Foo"));
        QCOMPARE(top->localDeclarations().at(1)->comment(), QByteArray("Foo"));
    }
}

void TestDUChain::functionDocBlockParams()
{
    TopDUContext* top = parse("<? class A {} /**\n * @param\tint\n *\t@param A\n *  @param   mixed **/\nfunction foo($a, $b, $c, $d) {} ", DumpNone);
    {
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());

        QCOMPARE(top->localDeclarations().at(1)->type<FunctionType>()->arguments().count(), 4);

        AbstractType::Ptr arg = top->localDeclarations().at(1)->type<FunctionType>()->arguments().at(0);
        QVERIFY(IntegralType::Ptr::dynamicCast(arg));
        QVERIFY(IntegralType::Ptr::dynamicCast(arg)->dataType() == IntegralType::TypeInt);
        QVERIFY(top->childContexts().at(1)->localDeclarations().at(0)->type<IntegralType>());
        QVERIFY(top->childContexts().at(1)->localDeclarations().at(0)->type<IntegralType>()->dataType() == IntegralType::TypeInt);

        arg = top->localDeclarations().at(1)->type<FunctionType>()->arguments().at(1);
        QVERIFY(StructureType::Ptr::dynamicCast(arg));
        QCOMPARE(StructureType::Ptr::dynamicCast(arg)->declaration(top), top->localDeclarations().at(0));
        QCOMPARE(top->childContexts().at(1)->localDeclarations().at(1)->type<StructureType>()->declaration(top), top->localDeclarations().at(0));

        arg = top->localDeclarations().at(1)->type<FunctionType>()->arguments().at(2);
        QVERIFY(IntegralType::Ptr::dynamicCast(arg));
        QVERIFY(IntegralType::Ptr::dynamicCast(arg)->dataType() == IntegralType::TypeMixed);

        arg = top->localDeclarations().at(1)->type<FunctionType>()->arguments().at(3);
        QVERIFY(IntegralType::Ptr::dynamicCast(arg));
        QVERIFY(IntegralType::Ptr::dynamicCast(arg)->dataType() == IntegralType::TypeMixed);
    }
}

void TestDUChain::memberFunctionDocBlockParams()
{
    TopDUContext* top = parse("<? class A { /**\n * @param bool\n * @param A\n * @param array\n **/\nfunction foo($a, $b, $c) {} }", DumpNone);
    {
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());

        QCOMPARE(top->childContexts().first()->localDeclarations().first()->type<FunctionType>()->arguments().count(), 3);

        AbstractType::Ptr arg = top->childContexts().first()->localDeclarations().first()->type<FunctionType>()->arguments().at(0);
        QVERIFY(IntegralType::Ptr::dynamicCast(arg));
        QVERIFY(IntegralType::Ptr::dynamicCast(arg)->dataType() == IntegralType::TypeBoolean);

        arg = top->childContexts().first()->localDeclarations().first()->type<FunctionType>()->arguments().at(1);
        QVERIFY(StructureType::Ptr::dynamicCast(arg));
        QCOMPARE(StructureType::Ptr::dynamicCast(arg)->declaration(top), top->localDeclarations().at(0));

        arg = top->childContexts().first()->localDeclarations().first()->type<FunctionType>()->arguments().at(2);
        QVERIFY(IntegralType::Ptr::dynamicCast(arg));
        QVERIFY(IntegralType::Ptr::dynamicCast(arg)->dataType() == IntegralType::TypeArray);
    }
}

void TestDUChain::foreachLoop()
{
    {
    TopDUContext* top = parse("<? $a = array(1); foreach($a as $k=>$i) { $i; }", DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().count(), 3);
    QCOMPARE(top->localDeclarations().at(1)->qualifiedIdentifier(), QualifiedIdentifier("k"));
    QVERIFY(top->localDeclarations().at(1)->abstractType().cast<IntegralType>());
    QCOMPARE(top->localDeclarations().at(1)->abstractType().cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeMixed));
    QCOMPARE(top->localDeclarations().at(2)->qualifiedIdentifier(), QualifiedIdentifier("i"));
    QVERIFY(top->localDeclarations().at(2)->abstractType().cast<IntegralType>());
    QCOMPARE(top->localDeclarations().at(2)->abstractType().cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeMixed));
    }
    {
    // bug: https://bugs.kde.org/show_bug.cgi?id=237110
    TopDUContext* top = parse("<? $a = array(1); foreach($a as $b) { $c = new stdclass; }", DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().count(), 3);
    QCOMPARE(top->localDeclarations().at(1)->qualifiedIdentifier(), QualifiedIdentifier("b"));
    qDebug() << top->localDeclarations().at(1)->toString();
    QVERIFY(top->localDeclarations().at(1)->abstractType().cast<IntegralType>());
    QCOMPARE(top->localDeclarations().at(1)->abstractType().cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeMixed));
    QCOMPARE(top->localDeclarations().at(2)->qualifiedIdentifier(), QualifiedIdentifier("c"));
    QVERIFY(top->localDeclarations().at(2)->abstractType().cast<StructureType>());
    QCOMPARE(top->localDeclarations().at(2)->abstractType().cast<StructureType>()->qualifiedIdentifier().toString(), QString("stdclass"));
    }
}

void TestDUChain::php4StyleConstructor()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class Aa { function Aa() { $this->bb(); } } ");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->childContexts().first()->localDeclarations().at(0);
    QVERIFY(dec);
    QCOMPARE(dec->qualifiedIdentifier(), QualifiedIdentifier("aa::aa"));
    ClassFunctionDeclaration* classFuncDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
    QVERIFY(classFuncDec);
    QVERIFY(classFuncDec->isConstructor());
}

void TestDUChain::constructor()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    {
        QByteArray method("<? class foobar { function __construct() {} } ");
        TopDUContext* top = parse(method, DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());

        Declaration* dec = top->childContexts().first()->localDeclarations().at(0);
        QVERIFY(dec);
        ClassFunctionDeclaration* classFuncDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
        QVERIFY(classFuncDec);
        QVERIFY(!classFuncDec->isDestructor());
        QVERIFY(classFuncDec->isConstructor());
    }
    {
        QByteArray method("<? class foobar { function foobar() {} } ");
        TopDUContext* top = parse(method, DumpNone);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());

        Declaration* dec = top->childContexts().first()->localDeclarations().at(0);
        QVERIFY(dec);
        ClassFunctionDeclaration* classFuncDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
        QVERIFY(classFuncDec);
        QVERIFY(!classFuncDec->isDestructor());
        QVERIFY(classFuncDec->isConstructor());
    }
}

void TestDUChain::destructor()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class foobar { function __destruct() {} } ");
    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->childContexts().first()->localDeclarations().at(0);
    QVERIFY(dec);
    ClassFunctionDeclaration* classFuncDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
    QVERIFY(classFuncDec);
    QVERIFY(classFuncDec->isDestructor());
    QVERIFY(!classFuncDec->isConstructor());
}

void TestDUChain::functionInFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function aaa() { function bbb() { } }");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().at(0)->qualifiedIdentifier(), QualifiedIdentifier("aaa"));
}

void TestDUChain::objectWithClassName()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class setupPage {} $setupPage = new setupPage; $setupPage->foo();");
    TopDUContext* top = parse(method, DumpNone, QUrl(QStringLiteral("file:///internal/testObjectWithClassName.php")));
    DUChainReleaser releaseTop(top);

    // update top (the pointer will be the same)
    QByteArray method2("<? $setupPage = new setupPage; $setupPage->foo();");
    TopDUContext* top2 = parse(method2, DumpNone, QUrl(QStringLiteral("file:///internal/testObjectWithClassName.php")));
    QVERIFY(top2 == top);
}

void TestDUChain::largeNumberOfDeclarations()
{
    TopDUContext* top = new TopDUContext(IndexedString(QUrl(QStringLiteral("file:///internal/testurl"))), RangeInRevision(0, 0, 6000, 0), 0);
    DUChain::self()->addDocumentChain(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    for (int i = 0; i < 6000; ++i) {
        RangeInRevision newRange(i, 0, i, 1);
        Declaration* dec = new Declaration(newRange, top);
        dec->setIdentifier(Identifier(QStringLiteral("dec%0").arg(i)));
        dec->setAbstractType(AbstractType::Ptr(0));
    }
}

void TestDUChain::staticVariable()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? define('AA', 0); function aaa() { static $foo; static $bar=0; static $baz='a'; static $bam=array(); static $woid=+'1'; static $a=AA; }");
    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->childContexts().at(1)->localDeclarations().count(), 6);

    QCOMPARE(top->childContexts().at(1)->localDeclarations().first()->qualifiedIdentifier(), QualifiedIdentifier("aaa::foo"));
    QVERIFY(top->childContexts().at(1)->localDeclarations().first()->type<IntegralType>());
    QCOMPARE(top->childContexts().at(1)->localDeclarations().first()->type<IntegralType>()->dataType(), (uint)IntegralType::TypeMixed);

    QCOMPARE(top->childContexts().at(1)->localDeclarations().at(1)->qualifiedIdentifier(), QualifiedIdentifier("aaa::bar"));
    QVERIFY(top->childContexts().at(1)->localDeclarations().at(1)->type<IntegralType>());
    QCOMPARE(top->childContexts().at(1)->localDeclarations().at(1)->type<IntegralType>()->dataType(), (uint)IntegralType::TypeInt);

    QCOMPARE(top->childContexts().at(1)->localDeclarations().at(2)->qualifiedIdentifier(), QualifiedIdentifier("aaa::baz"));
    QVERIFY(top->childContexts().at(1)->localDeclarations().at(2)->type<IntegralType>());
    QCOMPARE(top->childContexts().at(1)->localDeclarations().at(2)->type<IntegralType>()->dataType(), (uint)IntegralType::TypeString);

    QVERIFY(top->childContexts().at(1)->localDeclarations().at(3)->type<IntegralType>());
    QCOMPARE(top->childContexts().at(1)->localDeclarations().at(3)->type<IntegralType>()->dataType(), (uint)IntegralType::TypeArray);

    QVERIFY(top->childContexts().at(1)->localDeclarations().at(4)->type<IntegralType>());
    QCOMPARE(top->childContexts().at(1)->localDeclarations().at(4)->type<IntegralType>()->dataType(), (uint)IntegralType::TypeInt);

    QVERIFY(top->childContexts().at(1)->localDeclarations().at(5)->type<IntegralType>());
    QCOMPARE(top->childContexts().at(1)->localDeclarations().at(5)->type<IntegralType>()->dataType(), (uint)IntegralType::TypeInt);
}

void TestDUChain::returnTypeTwoDeclarations()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? function foo() { $i='a'; $i=0; return $i; } ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->localDeclarations().at(0);
    FunctionType::Ptr functionType = dec->type<FunctionType>();
    QVERIFY(functionType);
    UnsureType::Ptr retType = UnsureType::Ptr::dynamicCast(functionType->returnType());
    QVERIFY(retType);
    QCOMPARE(retType->typesSize(), 2u);
    QVERIFY(retType->types()[0].abstractType().cast<IntegralType>());
    QCOMPARE(retType->types()[0].abstractType().cast<IntegralType>()->dataType(), (uint)IntegralType::TypeString);
    QVERIFY(retType->types()[1].abstractType().cast<IntegralType>());
    QCOMPARE(retType->types()[1].abstractType().cast<IntegralType>()->dataType(), (uint)IntegralType::TypeInt);
}

void TestDUChain::globalVariableNotVisibleInFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = 0; function foo() { $a; }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("a")).first()->uses().count(), 0);
}

void TestDUChain::globalVariableInFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = 0; function foo() { global $a; $a; }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("a")).first()->uses().count(), 1);
}

void TestDUChain::nonGlobalVariableInFunction()
{
    // bug: https://bugs.kde.org/show_bug.cgi?id=240920

    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $a = 0; function foo() { $a = 1; }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->findLocalDeclarations(Identifier("a")).count(), 1);
    QCOMPARE(top->findLocalDeclarations(Identifier("a")).first()->uses().count(), 0);

    QCOMPARE(top->childContexts().count(), 2);
    QCOMPARE(top->childContexts().last()->findLocalDeclarations(Identifier("a")).count(), 1);
    QCOMPARE(top->childContexts().last()->findLocalDeclarations(Identifier("a")).first()->uses().count(), 0);
}

void TestDUChain::superglobalInFunction()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? $_GET; function foo() { $_GET; }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("_GET")).count(), 1);
    Declaration* dec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("_GET"))).first();
    QVERIFY(dynamic_cast<VariableDeclaration*>(dec));
    QVERIFY(static_cast<VariableDeclaration*>(dec)->isSuperglobal());
    QCOMPARE(dec->uses().keys().count(), 1);
    QCOMPARE(dec->uses().values().count(), 1);
    QCOMPARE(dec->uses().values().first().count(), 2);
    QCOMPARE(dec->uses().values().first().first(), RangeInRevision(0, 3, 0, 8));
    QCOMPARE(dec->uses().values().first().at(1), RangeInRevision(0, 27, 0, 32));
}

void TestDUChain::returnWithoutFunction()
{
    //yes, this is possible in php, you then have $a as return value of an include call
    QByteArray method("<? $a = 0; return $a; ");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
}

void TestDUChain::circularInheritance()
{
    //circular inheritance is not allowed in PHP and should not crash kdevelop
    QByteArray method("<? class a extends b {} class b extends c {} class c extends a {}");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);

    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->localDeclarations().at(2)->internalContext()->importedParentContexts().empty());
    QCOMPARE(top->localDeclarations().at(1)->internalContext()->importedParentContexts().count(), 1);
    QCOMPARE(top->localDeclarations().at(1)->internalContext()->importedParentContexts().first().context(top),
             top->localDeclarations().at(2)->internalContext());
    QCOMPARE(top->localDeclarations().at(0)->internalContext()->importedParentContexts().count(), 1);
    QCOMPARE(top->localDeclarations().at(0)->internalContext()->importedParentContexts().first().context(top),
             top->localDeclarations().at(1)->internalContext());
}

void TestDUChain::findDeclarations()
{
    DUChainWriteLocker lock(DUChain::lock());

    TopDUContext* top1 = new TopDUContext(IndexedString(QUrl(QStringLiteral("file:///internal/testfile1"))), RangeInRevision(0, 0, 0, 10), 0);
    DUChainReleaser releaseTop1(top1);
    DUChain::self()->addDocumentChain(top1);
    TopDUContext* top2 = new TopDUContext(IndexedString(QUrl(QStringLiteral("file:///internal/testfile2"))), RangeInRevision(0, 0, 0, 10), 0);
    DUChainReleaser releaseTop2(top2);
    DUChain::self()->addDocumentChain(top2);

    Declaration* declaration = new Declaration(RangeInRevision(0, 0, 0, 3), top1);
    declaration->setIdentifier(Identifier(QStringLiteral("foo")));

    QVERIFY(!top1->usingImportsCache());
    QVERIFY(!top2->usingImportsCache());

    QCOMPARE(1, top1->findDeclarations(Identifier("foo")).count());
    QCOMPARE(0, top2->findDeclarations(Identifier("foo")).count());
    top2->addImportedParentContext(top1);

    QVERIFY(!top1->usingImportsCache());
    QVERIFY(!top2->usingImportsCache());

    QCOMPARE(1, top2->findDeclarations(Identifier("foo")).count());
    top2->clearImportedParentContexts();
    QCOMPARE(top2->importedParentContexts().size(), 0);

    QVERIFY(!top1->usingImportsCache());
    QVERIFY(!top2->usingImportsCache());

    QCOMPARE(0, top2->findDeclarations(Identifier("foo")).count());
    top2->addImportedParentContext(top1);
    QCOMPARE(1, top2->findDeclarations(Identifier("foo")).count());
}

void TestDUChain::memberTypeAfterMethod()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { function foo(){} public $bar; }");

    TopDUContext* top = parse(method, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    DUContext* contextClassA = top->childContexts().first();

    // function foo
    {
        ClassMemberDeclaration* var = dynamic_cast<ClassMemberDeclaration*>(contextClassA->localDeclarations().first());
        QVERIFY(var);
        QCOMPARE(var->identifier(), Identifier("foo"));
        QCOMPARE(var->accessPolicy(), Declaration::Public);
        QCOMPARE(var->isStatic(), false);
        QVERIFY(var->type<FunctionType>());
        IntegralType::Ptr ret = var->type<FunctionType>()->returnType().cast<IntegralType>();
        QVERIFY(ret);
        QVERIFY(ret->dataType() == IntegralType::TypeVoid);
    }

    // public $bar
    {
        ClassMemberDeclaration* var = dynamic_cast<ClassMemberDeclaration*>(contextClassA->localDeclarations().at(1));
        QVERIFY(var);
        QCOMPARE(var->identifier(), Identifier("bar"));
        QCOMPARE(var->accessPolicy(), Declaration::Public);
        QCOMPARE(var->isStatic(), false);
        QVERIFY(var->type<IntegralType>());
        QVERIFY(var->type<IntegralType>()->dataType() == IntegralType::TypeMixed);
    }
}


void TestDUChain::catchDeclaration()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? try {} catch (Exception $e) {}");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    VariableDeclaration* ex = dynamic_cast<VariableDeclaration*>(top->localDeclarations().first());
    QVERIFY(ex);
    QCOMPARE(ex->identifier(), Identifier("e"));
    QVERIFY(ex->type<StructureType>());
    QCOMPARE(QualifiedIdentifier("exception"), ex->type<StructureType>()->declaration(top)->qualifiedIdentifier());
}

void TestDUChain::resourceType()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? /**\n * @return resource\n**/\nfunction test() {}");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    FunctionDeclaration* fun = dynamic_cast<FunctionDeclaration*>(top->localDeclarations().first());
    QVERIFY(fun);
    FunctionType::Ptr ftype = FunctionType::Ptr::dynamicCast(fun->abstractType());
    QVERIFY(ftype);
    IntegralType::Ptr rtype = IntegralType::Ptr::dynamicCast(ftype->returnType());
    QVERIFY(rtype);
    QCOMPARE(rtype->toString(), QString("resource"));
    QVERIFY(rtype->dataType() == IntegralTypeExtended::TypeResource);
}

void TestDUChain::foreachIterator()
{
    QByteArray code;
    code.append("<? class B {} class A implements Iterator {");
    code.append("public function rewind() {} ");
    code.append("/**\n * @return B\n */public function current() {} ");
    code.append("public function key() {} ");
    code.append("public function next() {} ");
    code.append("public function valid() {} ");
    code.append("} ");
    code.append("$a = new A();");
    code.append("foreach($a as $i) { $i; }");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* iDec = top->localDeclarations().at(3);
    QCOMPARE(iDec->qualifiedIdentifier(), QualifiedIdentifier("i"));
    QVERIFY(iDec->type<StructureType>());
    QCOMPARE(iDec->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QVERIFY(top->localDeclarations().first() == iDec->type<StructureType>()->declaration(top));
}

void TestDUChain::foreachIterator2()
{
    QByteArray code;
    code.append("<? class B {} class A implements Iterator {");
    code.append("public function rewind() {} ");
    code.append("/**\n * @return B\n */public function current() {} ");
    code.append("public function key() {} ");
    code.append("public function next() {} ");
    code.append("public function valid() {} ");
    code.append("} ");
    code.append("foreach(new A() as $i) { $i; }");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->localDeclarations().size(), 3);
    Declaration* iDec = top->localDeclarations().at(2);
    QCOMPARE(iDec->qualifiedIdentifier(), QualifiedIdentifier("i"));
    qDebug() << iDec->abstractType()->toString();
    QVERIFY(iDec->type<StructureType>());
    QCOMPARE(iDec->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QVERIFY(top->localDeclarations().first() == iDec->type<StructureType>()->declaration(top));
}

void TestDUChain::foreachIterator3()
{
    QByteArray code;
    code.append("<? class B {} class A implements Iterator {");
    code.append("public function rewind() {} ");
    code.append("/**\n * @return B\n */public function current() {} ");
    code.append("public function key() {} ");
    code.append("public function next() {} ");
    code.append("public function valid() {} ");
    code.append("} ");
    code.append("class C extends A { }");
    code.append("foreach(new C() as $i) { $i; }");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* iDec = top->localDeclarations().at(3);
    QCOMPARE(iDec->qualifiedIdentifier(), QualifiedIdentifier("i"));
    QVERIFY(iDec->type<StructureType>());
    QCOMPARE(iDec->type<StructureType>()->qualifiedIdentifier(), QualifiedIdentifier("b"));
    QVERIFY(top->localDeclarations().first() == iDec->type<StructureType>()->declaration(top));
}

void TestDUChain::foreachIterator4()
{
    // see also: https://bugs.kde.org/show_bug.cgi?id=276603
    QByteArray code = "<?\n"
                      "class A {\n"
                      "  public static $s;\n"
                      "  function foo() {\n"
                      "    foreach(array(1,2) as $this->i){}\n"
                      "    foreach(array(1,2) as $this->k => $this->v){}\n"
                      "    foreach(array(1,2) as A::$s){}\n"
                      "  }\n"
                      "}\n";

    TopDUContext* top = parse(code, DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().isEmpty());

    Declaration* aDec = top->localDeclarations().first();

    DUContext* fooCtx = top->childContexts().first()->childContexts().last();
    QVERIFY(fooCtx->owner());

    QCOMPARE(aDec->uses().size(), 1);
    QCOMPARE(aDec->uses().begin()->size(), 4);
}

void TestDUChain::returnThis()
{
    QByteArray code("<? class A { \n/**\n * @return $this\n */\npublic function x() {} } ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->childContexts().first()->localDeclarations().first();
    QVERIFY(dec->type<FunctionType>());
    AbstractType::Ptr t = dec->type<FunctionType>()->returnType();
    qDebug() << t->toString();
    QVERIFY(StructureType::Ptr::dynamicCast(t));
    QVERIFY(StructureType::Ptr::dynamicCast(t)->declaration(top) == top->localDeclarations().first());
}

void TestDUChain::unsureReturnType()
{
    QByteArray code("<? /**\n * @return bool|int\n */\nfunction x() {} ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->localDeclarations().first();
    QVERIFY(dec->type<FunctionType>());
    TypePtr<UnsureType> ut = dec->type<FunctionType>()->returnType().cast<UnsureType>();
    QVERIFY(ut);
    QCOMPARE((uint)2, ut->typesSize());
    QVERIFY(ut->types()[0].type<IntegralType>());
    QVERIFY(ut->types()[0].type<IntegralType>()->dataType() == IntegralType::TypeBoolean);
    QVERIFY(ut->types()[1].type<IntegralType>());
    QVERIFY(ut->types()[1].type<IntegralType>()->dataType() == IntegralType::TypeInt);
}


void TestDUChain::unsureReturnType2()
{
    QByteArray code("<? class A {} class B {}\n/**\n * @return A|B\n */\nfunction x() {} ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->localDeclarations().at(2);
    QVERIFY(dec->type<FunctionType>());
    TypePtr<UnsureType> ut = dec->type<FunctionType>()->returnType().cast<UnsureType>();
    QVERIFY(ut);
    QCOMPARE((uint)2, ut->typesSize());
    QVERIFY(ut->types()[0].type<StructureType>());
    QCOMPARE(ut->types()[0].type<StructureType>()->toString(), QString("A"));
    QVERIFY(ut->types()[1].type<StructureType>());
    QCOMPARE(ut->types()[1].type<StructureType>()->toString(), QString("B"));
}

void TestDUChain::unsureReturnType3()
{
    QByteArray code("<? function x() { if(rand(0,1)) return false; else return 1; return \"a\"; } ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->localDeclarations().at(0);
    QVERIFY(dec->type<FunctionType>());
    qDebug() << dec->type<FunctionType>()->returnType()->toString();
    TypePtr<UnsureType> ut = dec->type<FunctionType>()->returnType().cast<UnsureType>();
    QVERIFY(ut);
    QCOMPARE((uint)3, ut->typesSize());
    QVERIFY(ut->types()[0].type<IntegralType>());
    QVERIFY(ut->types()[0].type<IntegralType>()->dataType() == IntegralType::TypeInt);
    QVERIFY(ut->types()[1].type<IntegralType>());
    QVERIFY(ut->types()[1].type<IntegralType>()->dataType() == IntegralType::TypeBoolean);
    QVERIFY(ut->types()[2].type<IntegralType>());
    QVERIFY(ut->types()[2].type<IntegralType>()->dataType() == IntegralType::TypeString);
}

void TestDUChain::unsureReturnType4()
{
    QByteArray code("<? \n/**\n * @param bool|int\n */\nfunction x($a) { return $a; } ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->localDeclarations().first();
    QVERIFY(dec->type<FunctionType>());
    TypePtr<UnsureType> ut = dec->type<FunctionType>()->returnType().cast<UnsureType>();
    QVERIFY(ut);
    QCOMPARE((uint)2, ut->typesSize());
    QVERIFY(ut->types()[0].type<IntegralType>());
    QVERIFY(ut->types()[0].type<IntegralType>()->dataType() == IntegralType::TypeBoolean);
    QVERIFY(ut->types()[1].type<IntegralType>());
    QVERIFY(ut->types()[1].type<IntegralType>()->dataType() == IntegralType::TypeInt);
}

void TestDUChain::referencedArgument()
{
    // php does not return references
    QByteArray code("<? \nfunction x(&$a) { $a = 1; return $a; } ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->localDeclarations().first();
    QVERIFY(dec->type<FunctionType>());
    qDebug() << dec->abstractType()->toString();
    IntegralType::Ptr aType = dec->type<FunctionType>()->returnType().cast<IntegralType>();
    QVERIFY(aType);
    QCOMPARE(aType->dataType(), (uint)IntegralType::TypeInt);
    QCOMPARE(top->childContexts().first()->type(), DUContext::Function);
    ReferenceType::Ptr rType = top->childContexts().first()->localDeclarations().first()->abstractType().cast<ReferenceType>();
    QVERIFY(rType);
    QVERIFY(rType->baseType()->equals(aType.data()));
}

void TestDUChain::unsureReferencedArgument()
{
    // php does not return references
    QByteArray code("<? \nfunction x(&$a) { $a = 1; $a = 'asdf'; return $a; } ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->localDeclarations().first();
    QVERIFY(dec->type<FunctionType>());
    qDebug() << dec->abstractType()->toString();
    UnsureType::Ptr aType = dec->type<FunctionType>()->returnType().cast<UnsureType>();
    QVERIFY(aType);
    QCOMPARE(aType->typesSize(), 2u);
    QCOMPARE(aType->types()[0].abstractType().cast<IntegralType>()->dataType(), (uint)IntegralType::TypeInt);
    QCOMPARE(aType->types()[1].abstractType().cast<IntegralType>()->dataType(), (uint)IntegralType::TypeString);
    QCOMPARE(top->childContexts().first()->type(), DUContext::Function);
    ReferenceType::Ptr rType = top->childContexts().first()->localDeclarations().first()->abstractType().cast<ReferenceType>();
    QVERIFY(rType);
    QVERIFY(rType->baseType()->equals(aType.data()));
}

void TestDUChain::defaultArgument()
{
    // php does not return references
    QByteArray code("<? \nfunction x($a = 1) {} ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    Declaration* dec = top->childContexts().first()->localDeclarations().first();
    QVERIFY(dec->type<IntegralType>());
    QCOMPARE(dec->type<IntegralType>()->dataType(), (uint)IntegralType::TypeInt);
}

void TestDUChain::declareMemberOutOfClass()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? class foo{ protected $prot; private $priv; }\n"
                    // allowed, should only declare asdf once
                    "$bar = new foo; $bar->asdf = true; $bar->asdf = false;\n"
                    // not allowed:
                    "$bar->prot = 1;\n"
                    // not allowed:
                    "$bar->priv = 1;");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    { // $bar is only declared once
        QList<Declaration*> decs = top->findLocalDeclarations(Identifier(QStringLiteral("bar")));
        QCOMPARE(decs.size(), 1);
        Declaration *dec = decs.first();
        QVERIFY(dec->type<StructureType>());
        QVERIFY(dec->type<StructureType>()->declaration(top)->identifier().nameEquals(Identifier("foo")));

        // while we are at it, compare uses
        QCOMPARE(dec->uses().keys().count(), 1);
        QCOMPARE(dec->uses().values().count(), 1);
        QCOMPARE(dec->uses().values().first().count(), 4);
        qDebug() << dec->uses().values().first().at(0).castToSimpleRange();
        QCOMPARE(dec->uses().values().first().at(0), RangeInRevision(1, 16, 1, 20));
        qDebug() << dec->uses().values().first().at(1).castToSimpleRange();
        QCOMPARE(dec->uses().values().first().at(1), RangeInRevision(1, 35, 1, 39));
        qDebug() << dec->uses().values().first().at(2).castToSimpleRange();
        QCOMPARE(dec->uses().values().first().at(2), RangeInRevision(2, 0, 2, 4));
        qDebug() << dec->uses().values().first().at(3).castToSimpleRange();
        QCOMPARE(dec->uses().values().first().at(3), RangeInRevision(3, 0, 3, 4));
    }

    { // check if asdf got declared
        QList<Declaration*> decs = top->childContexts().first()->findDeclarations(Identifier(QStringLiteral("asdf")));
        // the type of both assignments to $bar->asdf are the same, hence it should only be declared once
        QCOMPARE(decs.size(), 1);
        ClassMemberDeclaration* cmdec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
        QVERIFY(cmdec);
        QVERIFY(cmdec->accessPolicy() == Declaration::Public);
        QVERIFY(!cmdec->isStatic());
        QVERIFY(cmdec->type<IntegralType>());
        QVERIFY(cmdec->type<IntegralType>()->dataType() == IntegralType::TypeBoolean);
    }

    // check that prot and priv don't get redeclared
    QCOMPARE(top->problems().count(), 2);
    QCOMPARE(top->problems().at(0)->finalLocation().start().line(), 2);
    QCOMPARE(top->problems().at(1)->finalLocation().start().line(), 3);
}

void TestDUChain::declareMemberOutOfClass2()
{
    // see also: https://bugs.kde.org/show_bug.cgi?id=283356
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? $a = new A();\n"
                    // allowed, should re-use existing declaration
                    "$a->x = 1;\n"
                    "class A { var $x = 1; }");
    TopDUContext* top = parse(code, DumpAST);
    QVERIFY(top);
    // update
    top = parse(code, DumpNone, top->url().toUrl(), ReferencedTopDUContext(top));
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());

    QList<Declaration*> decs = top->findLocalDeclarations(Identifier(QStringLiteral("a")));
    QCOMPARE(decs.size(), 2);
    {
        Declaration *dec = decs.first();
        QVERIFY(dynamic_cast<VariableDeclaration*>(dec));
        QVERIFY(dec->type<StructureType>());
        QVERIFY(dec->type<StructureType>()->declaration(top)->identifier().nameEquals(Identifier("a")));
    }
    {
        Declaration *dec = decs.last();
        QVERIFY(dynamic_cast<ClassDeclaration*>(dec));
        QVERIFY(dec->type<StructureType>());
        QVERIFY(dec->type<StructureType>()->declaration(top)->identifier().nameEquals(Identifier("a")));
    }

    { // check if x got declared
        QList<Declaration*> decs = top->childContexts().first()->findDeclarations(Identifier(QStringLiteral("x")));
        // the type of both assignments to $bar->asdf are the same, hence it should only be declared once
        QCOMPARE(decs.size(), 1);
        ClassMemberDeclaration* cmdec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
        QVERIFY(cmdec);
        QVERIFY(cmdec->accessPolicy() == Declaration::Public);
        QVERIFY(!cmdec->isStatic());
        QVERIFY(cmdec->type<IntegralType>());
        QCOMPARE(cmdec->type<IntegralType>()->dataType(), (uint) IntegralType::TypeInt);
    }
}

void TestDUChain::declareMemberInClassMethod()
{
    QByteArray code("<? class foo { private $priv = 0; protected $prot = 0; } class bar extends foo {\n"
                    // should declare member variable asdf (once!) as public
                    " function test() { $this->asdf = true; $this->asdf = false; }\n"
                    // should only declare bar once as private
                    " private $xyz = 0; function test2() { $this->xyz = 42; }\n"
                    // should not create any local declarations, and issue an error for trying to
                    // assign something to a private member variable of a parent class
                    " function test3() { $this->prot = 42;\n$this->priv = 42; }\n"
                    " }");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    { // asdf
        QList<Declaration*> decs = top->childContexts().last()->findLocalDeclarations(Identifier(QStringLiteral("asdf")));
        QCOMPARE(decs.size(), 1);
        ClassMemberDeclaration *dec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
        QVERIFY(dec);
        QVERIFY(dec->accessPolicy() == Declaration::Public);
        QVERIFY(!dec->isStatic());
        QVERIFY(dec->type<IntegralType>());
        QVERIFY(dec->type<IntegralType>()->dataType() == IntegralType::TypeBoolean);
    }

    { // xyz
        QList<Declaration*> decs = top->childContexts().last()->findLocalDeclarations(Identifier(QStringLiteral("xyz")));
        QCOMPARE(decs.size(), 1);
        ClassMemberDeclaration *dec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
        QVERIFY(dec);
        QVERIFY(dec->accessPolicy() == Declaration::Private);
        QVERIFY(!dec->isStatic());
        QVERIFY(dec->type<IntegralType>());
        QVERIFY(dec->type<IntegralType>()->dataType() == IntegralType::TypeInt);
    }

    { // prot and priv
        QVERIFY(top->childContexts().last()->findLocalDeclarations(Identifier("prot")).isEmpty());
        QVERIFY(top->childContexts().last()->findLocalDeclarations(Identifier("priv")).isEmpty());
    }

    // only one problem: error trying to assign to a private member of a parent class
    QCOMPARE(top->problems().count(), 1);
    QCOMPARE(top->problems().first()->finalLocation().start().line(), 4);
}

void TestDUChain::thisRedeclaration()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? class foo{ function foo(){ $this->test = true; $this = false;} }");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    // only $this = false is a problem, $this->test = true is perfectly valid
    QCOMPARE(top->problems().count(), 1);
    qDebug() << top->problems().first()->finalLocation();
    QVERIFY(top->problems().first()->finalLocation() == KDevelop::DocumentRange(top->url(), KTextEditor::Range(0, 50, 0, 55)));
}

void TestDUChain::implicitArrayDeclaration()
{
    ///TODO: adapt to unsure type once it's supported
    {
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? $a[1] = true;");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->findDeclarations(Identifier(QStringLiteral("a")));
    QCOMPARE(decs.size(), 1);
    VariableDeclaration* vdec = dynamic_cast<VariableDeclaration*>(decs.first());
    QVERIFY(vdec);
    QVERIFY(vdec->type<IntegralType>());
    QVERIFY(vdec->type<IntegralType>()->dataType() == IntegralType::TypeArray);
    }

    {
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? $b = 1; $a[$b] = true;");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->findDeclarations(Identifier(QStringLiteral("a")));
    QCOMPARE(decs.size(), 1);
    VariableDeclaration* vdec = dynamic_cast<VariableDeclaration*>(decs.first());
    QVERIFY(vdec);
    QVERIFY(vdec->type<IntegralType>());
    QVERIFY(vdec->type<IntegralType>()->dataType() == IntegralType::TypeArray);
    }

    {
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? class foo{} $bar = new foo; $bar->a[1] = true;");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->childContexts().first()->findDeclarations(Identifier(QStringLiteral("a")));
    QCOMPARE(decs.size(), 1);
    ClassMemberDeclaration* cmdec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
    QVERIFY(cmdec);
    QVERIFY(cmdec->type<IntegralType>());
    QVERIFY(cmdec->type<IntegralType>()->dataType() == IntegralType::TypeArray);
    }

    {
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? class foo{} $bar = new foo; $b = 1; $bar->a[$b] = true;");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->childContexts().first()->findDeclarations(Identifier(QStringLiteral("a")));
    QCOMPARE(decs.size(), 1);
    ClassMemberDeclaration* cmdec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
    QVERIFY(cmdec);
    QVERIFY(cmdec->type<IntegralType>());
    QVERIFY(cmdec->type<IntegralType>()->dataType() == IntegralType::TypeArray);
    }
}

void TestDUChain::implicitReferenceDeclaration()
{
    {
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? function asdf(&$foo) {} asdf($bar);");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> decs = top->findDeclarations(Identifier(QStringLiteral("bar")));
    QCOMPARE(decs.size(), 1);
    QVERIFY(dynamic_cast<VariableDeclaration*>(decs.first()));
    QVERIFY(decs.first()->type<IntegralType>());
    qDebug() << decs.first()->type<IntegralType>()->dataType() << decs.first()->toString();
    QVERIFY(decs.first()->type<IntegralType>()->dataType() == IntegralType::TypeNull);
    }

    {
    // a user reported a crash with the code example below

    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<? function test(&$p) {} class foo{ private $a; function test() {test($this->a);}  }");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY( top->childContexts().last()->localScopeIdentifier() == QualifiedIdentifier("foo"));

    // a is already declared
    QList<Declaration*> decs = top->childContexts().last()->findDeclarations(Identifier(QStringLiteral("a")));
    QCOMPARE(decs.size(), 1);
    ClassMemberDeclaration* cmdec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
    QVERIFY(cmdec);
    QVERIFY(cmdec->type<IntegralType>());

    qDebug() << cmdec->type<IntegralType>()->dataType() << cmdec->toString();
    QVERIFY(cmdec->type<IntegralType>()->dataType() == IntegralType::TypeMixed);
    }
}

void TestDUChain::classContextRange()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<?php class A { } $a = new A; ?> <?php $b = 1; $a->foobar = 1; $a->barFoo= 0;");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->childContexts().first()->range(), KDevelop::RangeInRevision(0, 6, 0, 17));
    QCOMPARE(top->childContexts().first()->localDeclarations().count(), 2);
}

void TestDUChain::lateClassMembers()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray code("<?php class A { function f() { $this->val = 'b'; } private $val = 'a'; } ");
    TopDUContext* top = parse(code, DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ClassDeclaration* cdec = dynamic_cast<ClassDeclaration*>(top->localDeclarations().first());
    QVERIFY(cdec);
    QList<Declaration*> decs = cdec->logicalInternalContext(top)->findDeclarations(Identifier(QStringLiteral("val")));
    QCOMPARE(decs.count(), 1);
    ClassMemberDeclaration* cmdec = dynamic_cast<ClassMemberDeclaration*>(decs.first());
    QVERIFY(cmdec);
    QCOMPARE(cmdec->accessPolicy(), Declaration::Private);
}

void TestDUChain::list()
{
    Q_FOREACH ( const QString& code, QStringList() << "<?php list($i, $j, $k) = array(1,2,3);"
                                                 << "<?php $a = array(1,2,3); list($i,$j,$k) = $a;"
                                                 << "<?php function t() { return array(1,2,3); } list($i,$j,$k) = t();" )
    {
        //               0         1         2         3         4         5         6         7
        //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
        TopDUContext* top = parse(code.toUtf8(), DumpAST);
        DUChainReleaser releaseTop(top);
        DUChainWriteLocker lock(DUChain::lock());

        Q_FOREACH ( const QString& identifier, QStringList() << "i" << "j" << "k" ) {
            qDebug() << "searching for declaration of " << identifier;
            QList<Declaration*> decs = top->findDeclarations(Identifier(identifier));
            QCOMPARE(decs.size(), 1);
            Declaration *dec = decs.first();
            QVERIFY(dec->type<IntegralType>());
            QCOMPARE(dec->type<IntegralType>()->dataType(), (uint) IntegralType::TypeMixed);
            ///TODO: support arrays better and compare to actual type
        }
    }
}

void TestDUChain::alternateDocCommentTypeHints()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php class test {\n/// @var test\nprivate static $test;\n}", DumpAST);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    ClassDeclaration* cdec = dynamic_cast<ClassDeclaration*>(top->localDeclarations().first());
    QVERIFY(cdec);
    QVERIFY(cdec->type<StructureType>());
    QVector<Declaration*> decs = cdec->logicalInternalContext(top)->localDeclarations();
    QCOMPARE(decs.size(), 1);
    Declaration *dec = decs.first();
    QVERIFY(dec->type<StructureType>());
    QCOMPARE(dec->type<StructureType>()->declaration(top), cdec);
}

void TestDUChain::findFunctionArgs()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php function foo($bar, $asdf) {}", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    FunctionDeclaration* funcDec = dynamic_cast<FunctionDeclaration*>(top->localDeclarations().first());
    QVERIFY(funcDec);
    QVERIFY(funcDec->internalContext());
    QVERIFY(funcDec->internalFunctionContext());
    QVERIFY(funcDec->internalContext()->imports(funcDec->internalFunctionContext()));

    QList<Declaration*> decs;
    Q_FOREACH ( Declaration* arg, funcDec->internalFunctionContext()->localDeclarations() ) {
        decs = funcDec->internalContext()->findDeclarations(arg->identifier());
        QCOMPARE(decs.size(), 1);
        decs = funcDec->internalContext()->findDeclarations(arg->qualifiedIdentifier());
        qDebug() << arg->qualifiedIdentifier().toString();
        QEXPECT_FAIL("", "strangely the arg dec is only found with its identifier, not by its qualifiedidentifier...", Continue);
        QCOMPARE(decs.size(), 1);
    }
}

void TestDUChain::undeclaredPropertyInString()
{
    // testcase for bug 209814

    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php class foo { var $foo; function bar() { \"$this->baz\"; } }", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QCOMPARE(top->childContexts().size(), 1);
    DUContext* classCtx = top->childContexts().first();
    QVERIFY(classCtx->type() == DUContext::Class);
    QCOMPARE(classCtx->localDeclarations().size(), 2);
    QCOMPARE(classCtx->findDeclarations(Identifier("foo")).size(), 1);
    QCOMPARE(classCtx->findDeclarations(Identifier("bar")).size(), 1);
}

void TestDUChain::undeclaredVarPropertyInString()
{
    // testcase for bug 210043

    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php \"$a->baz\";", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    // just don't crash
}

void TestDUChain::upcommingClassInString()
{
    // testcase for bug 232687

    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                                "class A {\n"
                                "    function A () {\n"
                                "        $b = new B();\n"
                                "        echo \"$b->blah\";\n"
                                "    }\n"
                                "}\n"
                                "class B {\n"
                                "  var $blah;\n"
                                "}\n", DumpNone);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    // just don't crash
}

void TestDUChain::namespaces()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "namespace asdf{\n"
                              "function a(){}\n"
                              "define('b', 0);\n"
                              "class c {}\n"
                              "}\n"
                              "namespace NS1\\NS2 {\n"
                              "function a(){}\n"
                              "define('b', 0);\n"
                              "class c {}\n"
                              "}\n"
                              "namespace {\n"
                              "function a(){}\n"
                              "}\n"
                              , DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->problems().count(), 0);

    QCOMPARE(top->childContexts().size(), 4);
    QCOMPARE(top->childContexts().at(0)->localScopeIdentifier().toString(), QString("asdf"));
    QCOMPARE(top->childContexts().at(1)->localScopeIdentifier().toString(), QString("ns1"));

    QCOMPARE(top->childContexts().at(2)->type(), DUContext::Function);
    QCOMPARE(top->childContexts().at(3)->localScopeIdentifier().toString(), QString("a"));

    QCOMPARE(top->localDeclarations().size(), 3);
    QCOMPARE(top->localDeclarations().at(0)->kind(), Declaration::Namespace);
    QCOMPARE(top->localDeclarations().at(1)->kind(), Declaration::Namespace);
    QCOMPARE(top->localDeclarations().at(2)->kind(), Declaration::Type);

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("asdf")).size(), 1);
    QCOMPARE(top->childContexts().at(0)->localDeclarations().size(), 3);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("asdf::a")).size(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("asdf::b")).size(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("asdf::c")).size(), 1);

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("ns1")).size(), 1);
    QCOMPARE(top->childContexts().at(1)->localDeclarations().size(), 1);
    QCOMPARE(top->childContexts().at(1)->localDeclarations().first()->kind(), Declaration::Namespace);
    ///TODO: support \ as separator
    QCOMPARE(top->childContexts().at(1)->localDeclarations().first()->qualifiedIdentifier().toString(), QString("ns1::ns2"));
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("ns1::ns2")).size(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("ns1::ns2")).first()->logicalInternalContext(top)->localDeclarations().size(), 3);
    QCOMPARE(top->childContexts().at(1)->childContexts().size(), 1);
    QCOMPARE(top->childContexts().at(1)->childContexts().first()->localDeclarations().size(), 3);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("ns1::ns2")).first()->logicalInternalContext(top)->localDeclarations().first()->qualifiedIdentifier().toString(),
             QString("ns1::ns2::a"));
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("ns1::ns2::a")).size(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("ns1::ns2::b")).size(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("ns1::ns2::c")).size(), 1);

    QCOMPARE(top->findDeclarations(QualifiedIdentifier("a")).size(), 1);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("b")).size(), 0);
    QCOMPARE(top->findDeclarations(QualifiedIdentifier("c")).size(), 0);

    ///TODO: prevent redeclarations of namespaces
}

void TestDUChain::namespacesNoCurly()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "namespace asdf;\n"
                              "function a(){}\n"
                              "define('b', 0);\n"
                              "class c {}\n"
                              "\n"
                              "namespace NS1\\NS2;\n"
                              "function a(){}\n"
                              "define('b', 0);\n"
                              "class c {}\n"
                              "\n"
                              , DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->problems().count(), 0);
    Q_FOREACH(ProblemPointer p, top->problems()) {
        qDebug() << p->description() << p->explanation() << p->finalLocation();
    }
    QCOMPARE(top->childContexts().size(), 2);
    QCOMPARE(top->childContexts().at(0)->localScopeIdentifier().toString(), QString("asdf"));
    QCOMPARE(top->childContexts().at(1)->localScopeIdentifier().toString(), QString("ns1"));

    QCOMPARE(top->localDeclarations().size(), 2);
    QCOMPARE(top->localDeclarations().at(0)->kind(), Declaration::Namespace);
    QCOMPARE(top->localDeclarations().at(1)->kind(), Declaration::Namespace);
}

void TestDUChain::useNamespace()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "namespace ns1\\ns2 {\n"
                              "function a(){}\n"
                              "const b = 0;\n"
                              "class c {}\n"
                              "}\n"
                              "namespace ns3\\ns4 {\n"
                              "function a(){}\n"
                              "const b = 0;\n"
                              "class c {}\n"
                              "}\n"
                              "namespace {\n"
                              "use ns1\\ns2, ns3\\ns4 as ns5;\n"
                              "use \\ns3\\ns4 as ns6;\n"
                              "}\n"
                              , DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QCOMPARE(top->localDeclarations().count(), 5);

    Declaration* dec = top->localDeclarations().at(2);
    QCOMPARE(dec->qualifiedIdentifier().toString(), QString("ns2"));
    QVERIFY(dynamic_cast<NamespaceAliasDeclaration*>(dec));

    dec = top->localDeclarations().at(3);
    QCOMPARE(dec->qualifiedIdentifier().toString(), QString("ns5"));
    QVERIFY(dynamic_cast<NamespaceAliasDeclaration*>(dec));

    dec = top->localDeclarations().at(4);
    QCOMPARE(dec->qualifiedIdentifier().toString(), QString("ns6"));
    QVERIFY(dynamic_cast<NamespaceAliasDeclaration*>(dec));
    ///TODO: find out why this is explicitly required
    QVERIFY(!dynamic_cast<NamespaceAliasDeclaration*>(dec)->importIdentifier().explicitlyGlobal());
}

void TestDUChain::namespaceStaticVar()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "namespace ns {\n"
                              "class c{ static public $foo; }\n"
                              "}\n"
                              "namespace {\n"
                              "\\ns\\c::$foo;\n"
                              "}\n"
                              , DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());
    Declaration* fooDec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("ns::c::foo"))).first();
    QVERIFY(fooDec);

    QVERIFY(!fooDec->uses().isEmpty());
    QVERIFY(!fooDec->uses().begin()->isEmpty());
    QCOMPARE(fooDec->uses().begin()->begin()->start.line, 5);
}

void TestDUChain::namespacedCatch()
{
    // see also: https://bugs.kde.org/show_bug.cgi?id=281451
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "namespace ns {\n"
                              "class e{}\n"
                              "}\n"
                              "namespace {\n"
                              "try { /* ... */ }\n"
                              "catch(\\ns\\e $exception) { /* ... */ }"
                              "}\n"
                              , DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    QVERIFY(top->problems().isEmpty());
    Declaration* eDec = top->findDeclarations(QualifiedIdentifier(QStringLiteral("ns::e"))).first();
    QVERIFY(eDec);

    QVERIFY(!eDec->uses().isEmpty());
    QVERIFY(!eDec->uses().begin()->isEmpty());
    QCOMPARE(eDec->uses().begin()->begin()->start.line, 6);
}

struct TestUse {
    TestUse(const QString& _id, Declaration::Kind _kind, int _uses)
        : id(_id), kind(_kind), uses(_uses)
    {}
    TestUse()
    {}
    QualifiedIdentifier id;
    Declaration::Kind kind;
    int uses;
};

Q_DECLARE_METATYPE ( TestUse )
Q_DECLARE_METATYPE ( QList<TestUse> )

void TestDUChain::errorRecovery_data()
{
    QTest::addColumn<QString>("code");

    QTest::addColumn< QList<TestUse> >("usesMap");

    QTest::newRow("conditional") << QStringLiteral("<?php $a = 1; if ( false ) { in va lid } $a = 2; ")
                                 << (QList<TestUse>()
                                    << TestUse(QStringLiteral("a"), Declaration::Instance, 1));

    QTest::newRow("namespace") << QStringLiteral("<?php namespace foo { const a = 1; } namespace y { -a sdflyxjcv 91348 } namespace { foo\\a; }")
                                 << (QList<TestUse>()
                                    << TestUse(QStringLiteral("foo"), Declaration::Namespace, 1)
                                    << TestUse(QStringLiteral("y"), Declaration::Namespace, 0)
                                    << TestUse(QStringLiteral("foo::a"), Declaration::Instance, 1));

    QTest::newRow("class") << QStringLiteral("<?php class foo { const bar = 1; invalid static function func() {} } foo::bar; foo::func;")
                                 << (QList<TestUse>()
                                    << TestUse(QStringLiteral("foo"), Declaration::Type, 0)
                                    << TestUse(QStringLiteral("foo::bar"), Declaration::Instance, 1)
                                    << TestUse(QStringLiteral("foo::func"), Declaration::Type, 1)
                                    );
}

void TestDUChain::errorRecovery()
{
    QFETCH(QString, code);
    QFETCH(QList<TestUse>, usesMap);

    TopDUContext* top = parse(code.toLocal8Bit(), DumpAll);

    QVERIFY(top);

    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;

    Q_FOREACH ( const TestUse& use, usesMap ) {
        QList< Declaration* > decs = top->findDeclarations(use.id);
        QCOMPARE(decs.count(), 1);
        Declaration* dec = decs.first();
        QCOMPARE(dec->kind(), use.kind);
        if (use.uses) {
            QCOMPARE(dec->uses().count(), 1);
            QCOMPARE(dec->uses().begin()->count(), use.uses);
        }
    }
}

void TestDUChain::varStatic()
{
    //bug: https://bugs.kde.org/244076

    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "class c { static a = 1; static function foo() {} }\n"
                              "$o = 'c';\n"
                              "$o::a;\n"
                              "$o::foo();\n"
                              , DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().empty());

    // we cannot support anything though :(
}

void TestDUChain::staticNowdoc()
{
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "class c {\n"
                              "public $bar = <<<'FOO'\nbar\nFOO;\n"
                              "public const C = <<<'FOO'\nbar\nFOO;\n"
                              "}\n"
                              , DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().empty());

    QCOMPARE(top->childContexts().first()->localDeclarations().count(), 2);
    QCOMPARE(top->childContexts().first()->localDeclarations().first()->type<IntegralType>()->dataType(),
             static_cast<uint>(IntegralType::TypeString));
    QCOMPARE(top->childContexts().first()->localDeclarations().last()->type<IntegralType>()->dataType(),
             static_cast<uint>(IntegralType::TypeString));
}

void TestDUChain::curlyVarAfterObj()
{
    // bug: https://bugs.kde.org/show_bug.cgi?id=241645

    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse("<?php\n"
                              "class c {\n"
                              "public $bar = 'foo';\n"
                              "public $asdf = 'bar';\n"
                              "public function foo(){}\n"
                              "}\n"
                              "$a = new c;\n"
                              "$a->{$a->bar}();\n"
                              "$a->{$a->asdf};\n"
                              , DumpNone);
    QVERIFY(top);

    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().empty());
}

void TestDUChain::embeddedHTML_data()
{
    QTest::addColumn<QString>("code");

    QTest::newRow("if") << QStringLiteral("<?php if ( true ) : ?>\n<?php endif; ?>");
    QTest::newRow("elseif") << QStringLiteral("<?php if ( true ) : ?>\n<?php elseif ( false ) : ?>\n<?php endif; ?>");
    QTest::newRow("foreach") << QStringLiteral("<?php foreach ( array(1,2) as $i ) : ?>\n<?php endforeach; ?>\n");
    QTest::newRow("switch") << QStringLiteral("<?php switch ( 1 ) : case 1: ?>\n<?php break; endswitch; ?>\n");
    QTest::newRow("for") << QStringLiteral("<?php for ( ;; ) : ?>\n<?php endfor; ?>\n");
    QTest::newRow("while") << QStringLiteral("<?php while ( true ) : ?>\n<?php endwhile; ?>\n");
    QTest::newRow("else") << QStringLiteral("<?php if (true):\n echo 'ok1';\n else:\n echo 'ok2';\n endif; ?>");

}

void TestDUChain::embeddedHTML()
{
    QFETCH(QString, code);

    TopDUContext* top = parse(code.toLocal8Bit(), DumpNone);

    QVERIFY(top);

    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().empty());
}

void TestDUChain::cases()
{
    // testcase for bug https://bugs.kde.org/show_bug.cgi?id=245832
    TopDUContext* top = parse("<?php switch(1) { case 1:\n case 2:\n break; default: break; }", DumpNone);

    QVERIFY(top);

    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().empty());
}

void TestDUChain::closureParser()
{
    // testcase for the parser after closures where introduced,
    // to make sure nothing brakes and all parser conflicts are resolved
    TopDUContext* top = parse("<?php\n"
                              "$lambda1 = function() {return 0;};\n"
                              "$lambda2 = function() use ($lambda1) {return 0;};\n"
                              "$lambda3 = function & () use (&$lambda2, $lambda1) {return 0;};\n"
                              "$lambda4 = function & ($a, &$b, stdClass $c) use (&$lambda2, $lambda1) {return 0;};\n"
                              "\n"
                              "class a {\n"
                              "  function foo() {}\n"
                              "  function & bar() {}\n"
                              "}\n"
                              "function foo() {}\n"
                              "function & bar() {}\n", DumpNone);

    QVERIFY(top);

    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().empty());
}

void TestDUChain::closures()
{
    TopDUContext* top = parse("<?php $l = function($a, stdClass $b) { return 0; };\n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().isEmpty());

    QCOMPARE(top->localDeclarations().count(), 2);
    Declaration* l = top->localDeclarations().first();
    QCOMPARE(l->identifier().toString(), QString("l"));
    Declaration* closure = top->localDeclarations().last();
    QVERIFY(closure->identifier().isEmpty());

    FunctionType::Ptr funcType = closure->type<FunctionType>();
    QVERIFY(funcType);

    QCOMPARE(funcType->arguments().count(), 2);
    QVERIFY(funcType->arguments().at(0).cast<IntegralType>());
    QCOMPARE(funcType->arguments().at(0).cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeMixed));
    QVERIFY(funcType->arguments().at(1).cast<StructureType>());
    QCOMPARE(funcType->arguments().at(1).cast<StructureType>()->qualifiedIdentifier().toString(), QString("stdclass"));

    QVERIFY(funcType->returnType().cast<IntegralType>());
    QCOMPARE(funcType->returnType().cast<IntegralType>()->dataType(), static_cast<uint>(IntegralType::TypeInt));

    QVERIFY(l->abstractType()->equals(closure->abstractType().constData()));
}

void TestDUChain::closureEmptyUse()
{
    // test case for: https://bugs.kde.org/show_bug.cgi?id=267105
    // don't crash but report parse error
    TopDUContext* top = parse("<?php $c = function ($v) use () { return $v > 2; };\n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QCOMPARE(top->problems().size(), 1);
}

void TestDUChain::iifeParser()
{
    // testcase for bug https://bugs.kde.org/show_bug.cgi?id=370515
    TopDUContext* top = parse("<?php\n"
                              "$lambda1 = (function() {return 5;})();\n"
                              "$lambda2 = (function($a) {return $a;})(50);\n"
                              "$lambda3 = (function($a){ return function($b) use ($a){echo $a + $b;};})(50); \n"
                              "$lambda4 = (function ($a){echo $a;})(10) + (function ($a){echo $a ;})(20);"
                              , DumpNone);

    QVERIFY(top);

    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().empty());
}

void TestDUChain::iife()
{
    TopDUContext* top = parse("<?php $l = (function($a){ return function($b) use ($a){echo $a + $b;};})(50); \n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().isEmpty());
    QCOMPARE(top->localDeclarations().count(), 2);
    Declaration* l = top->localDeclarations().first();
    QCOMPARE(l->identifier().toString(), QString("l"));
    Declaration* iife = top->localDeclarations().last();
    QVERIFY(iife->identifier().isEmpty());
}

void TestDUChain::gotoTest()
{
    TopDUContext* top = parse("<?php goto dest; dest: \n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().isEmpty());

    ///TODO: create declaration for destination label
    ///TODO: create use for goto label
    ///TODO: report error when trying to jump into loop or switch statement
}

void TestDUChain::ternary()
{
    TopDUContext* top = parse("<?php $a = true ? 1 : 2; $b = false ?: 3; \n", DumpNone);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().isEmpty());
}

void TestDUChain::bug296709()
{
    // see also: https://bugs.kde.org/show_bug.cgi?id=296709
    //               0         1         2         3         4         5         6         7
    //               01234567890123456789012345678901234567890123456789012345678901234567890123456789
    TopDUContext* top = parse(
                    "<?php\n"
                    "foreach(array() as $a) {\n"
                    "  $a[0] = 1;\n"
                    "}\n", DumpAll);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock;
    QVERIFY(top->problems().isEmpty());
    QList< Declaration* > decs = top->findLocalDeclarations(Identifier(QStringLiteral("a")));
    QCOMPARE(decs.size(), 1);
    QCOMPARE(decs.at(0)->range(), RangeInRevision(1, 19, 1, 21));
    QCOMPARE(decs.at(0)->uses().count(), 1);
    QCOMPARE(decs.at(0)->uses().begin()->count(), 1);
    QCOMPARE(decs.at(0)->uses().begin()->first(), RangeInRevision(2, 2, 2, 4));
}


void TestDUChain::declareFinalMethod()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<? class A { public final function foo() {} }");

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(!top->parentContext());
    QCOMPARE(top->childContexts().count(), 1);

    DUContext* contextClassA = top->childContexts().first();

    Declaration* dec = contextClassA->localDeclarations().at(0);
    ClassFunctionDeclaration* funDec = dynamic_cast<ClassFunctionDeclaration*>(dec);
    QVERIFY(funDec);
    QCOMPARE(funDec->qualifiedIdentifier(), QualifiedIdentifier("a::foo"));
    QVERIFY(funDec->isFinal());
}

void Php::TestDUChain::testTodoExtractor()
{
    //                 0         1         2         3         4         5         6         7
    //                 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    QByteArray method("<?\n"
                      "/* TODO: bla */\n"
                      "/// FIXME blub");

    QVERIFY(KDevelop::ICore::self()->languageController()->completionSettings()->todoMarkerWords().contains("TODO"));
    QVERIFY(KDevelop::ICore::self()->languageController()->completionSettings()->todoMarkerWords().contains("FIXME"));

    TopDUContext* top = parse(method, DumpAll);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top);
    QCOMPARE(top->problems().size(), 2);
    QCOMPARE(top->problems().at(0)->description(), QString("TODO: bla"));
    QCOMPARE(top->problems().at(0)->range(), RangeInRevision(1, 3, 1, 12));
    QCOMPARE(top->problems().at(1)->description(), QString("FIXME blub"));
    QCOMPARE(top->problems().at(1)->range(), RangeInRevision(2, 4, 2, 14));
}

void TestDUChain::useThisAsArray()
{
    QByteArray method("<?php\n"
                      "  interface ArrayAccess{} "
                      "  class A implements \\ArrayAccess\n"
                      "  {\n"
                      "      $values = [];\n"
                      "      function offsetGet($offset) { return $this->values[$offset]; }\n"
                      "      function offsetSet($offset, $value) { $this->values[$offset] = $value; }\n"
                      "      function offsetExists($offset) { return array_key_exists($offset, $this->values); }\n"
                      "      function offsetUnset($offset) { unset($this->values[$offset]); }\n"
                      "      function setTest() { $this['test'] = 'test'; }  \n"
                      "  }\n");
    
    TopDUContext* top = parse(method);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    
    QCOMPARE(top->importedParentContexts().count(), 1);
    QVERIFY(DUChain::self()->chainForDocument(internalFunctionFile()));
    QCOMPARE(DUChain::self()->chainForDocument(internalFunctionFile()), top->importedParentContexts().first().context(top));
    
    QVERIFY(top->problems().isEmpty());
}

void TestDUChain::wrongUseOfThisAsArray()
{
    // missing functions from \ArrayAccess and not declared abstract
    QByteArray method("<?php\n"
                      "  interface ArrayAccess{} "
                      "  class A implements \\ArrayAccess\n"
                      "  {\n"
                      "      public function setTest() { $this['test'] = 'test'; }  \n"
                      "  }\n");

    TopDUContext* top = parse(method);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());
    
    QCOMPARE(top->problems().size(),1);
}

void TestDUChain::staticFunctionClassPhp54()
{
    QByteArray method("<?php\n"
                      "  class A\n"
                      "  {\n"
                      "      public static function func() {}  \n"
                      "  }\n"
                      " A::{'func'}();  \n");

    TopDUContext* top = parse(method);
    QVERIFY(top);
    DUChainReleaser releaseTop(top);
    DUChainWriteLocker lock(DUChain::lock());

    QVERIFY(top->problems().isEmpty());
    QCOMPARE(top->localDeclarations().count(),1);

    Declaration* dec = top->localDeclarations().at(0);
    ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(dec);
    QCOMPARE(classDec->uses().count(),1);
}
