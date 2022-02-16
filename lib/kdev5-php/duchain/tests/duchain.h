/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TESTDUCHAIN_H
#define TESTDUCHAIN_H

#include "tests/duchaintestbase.h"

namespace Php
{
class TestDUChain : public DUChainTestBase
{
    Q_OBJECT

public:
    TestDUChain();

private slots:
    void declareFunction();
    void declareVar();
    void varTypehint();
    void declareClass();
    void classMemberVar();
    void declareTypehintFunction();
    void declareTypehintArrayFunction();
    void declareTypehintCallableFunction();
    void returnTypeClass();
    void declarationReturnType();
    void declarationReturnTypeInRecursingFunction();
    void returnTypeViaMember();
    void declarationMultipleReturnTypes();
    void declarationReturnTypeDocBlock();
    void declarationReturnTypeDocBlockIntegral();
    void declarationReturnTypeClassChain();
    void classImplementsInterface();
    void classExtends();
    void staticMethod();
    void ownStaticMethod();
    void thisVar();
    void objectFunctionCall();
    void objectFunctionCall2();
    void objectFunctionCall3();
    void objectVariable();
    void staticMemberVariable();
    void ownStaticMemberVariable();
    void classConst();
    void classConst_data();
    void fileConst();
    void fileConst_data();
    void define();
    void defaultFunctionParam();
    void globalFunction();
    void globalVariableFromInternalFunctions();
    void newObjectFromOtherFile();
    void unknownReturnType();
    void staticFunctionCallFromOtherFile();
    void classConstantFromOtherFile();
    void globalFunctionCallFromOtherFile();
    void constantFromOtherFile();
    void singleton();
    void internalFunctions();
    void trueFalse();
    void null();
    void array();
    void functionDocBlock();
    void variableDocBlock();
    void functionDocBlockParams();
    void memberFunctionDocBlockParams();
    void foreachLoop();
    void php4StyleConstructor();
    void constructor();
    void destructor();
    void functionInFunction();
    void objectWithClassName();
    void largeNumberOfDeclarations();
    void staticVariable();
    void returnTypeTwoDeclarations();
    void globalVariableNotVisibleInFunction();
    void globalVariableInFunction();
    void nonGlobalVariableInFunction();
    void superglobalInFunction();
    void returnWithoutFunction();
    void circularInheritance();
    void findDeclarations();
    void memberTypeAfterMethod();
    void catchDeclaration();
    void resourceType();
    void foreachIterator();
    void foreachIterator2();
    void foreachIterator3();
    void foreachIterator4();
    void returnThis();
    void unsureReturnType();
    void unsureReturnType2();
    void unsureReturnType3();
    void unsureReturnType4();
    void referencedArgument();
    void unsureReferencedArgument();
    void defaultArgument();
    void declareMemberOutOfClass();
    void declareMemberOutOfClass2();
    void declareMemberInClassMethod();
    void thisRedeclaration();
    void implicitArrayDeclaration();
    void implicitReferenceDeclaration();
    void classContextRange();
    void lateClassMembers();
    void list();
    void alternateDocCommentTypeHints();
    void findFunctionArgs();
    void undeclaredPropertyInString();
    void undeclaredVarPropertyInString();
    void upcommingClassInString();
    void namespaces();
    void namespacesNoCurly();
    void useNamespace();
    void namespaceStaticVar();
    void namespacedCatch();
    void errorRecovery_data();
    void errorRecovery();
    void varStatic();
    void staticNowdoc();
    void curlyVarAfterObj();
    void embeddedHTML_data();
    void embeddedHTML();
    void cases();
    void closureParser();
    void closures();
    void closureEmptyUse();
    void iifeParser();
    void iife();
    void gotoTest();
    void ternary();
    void bug296709();
    void declareFinalMethod();
    void testTodoExtractor();
    void useThisAsArray();
    void wrongUseOfThisAsArray();
    void staticFunctionClassPhp54();
};

}

#endif
