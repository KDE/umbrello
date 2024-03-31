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

private:
    Q_SLOT void declareFunction();
    Q_SLOT void declareVar();
    Q_SLOT void varTypehint();
    Q_SLOT void declareClass();
    Q_SLOT void classMemberVar();
    Q_SLOT void declareTypehintFunction();
    Q_SLOT void declareTypehintArrayFunction();
    Q_SLOT void declareTypehintCallableFunction();
    Q_SLOT void returnTypeClass();
    Q_SLOT void declarationReturnType();
    Q_SLOT void declarationReturnTypeInRecursingFunction();
    Q_SLOT void returnTypeViaMember();
    Q_SLOT void declarationMultipleReturnTypes();
    Q_SLOT void declarationReturnTypeDocBlock();
    Q_SLOT void declarationReturnTypeDocBlockIntegral();
    Q_SLOT void declarationReturnTypeClassChain();
    Q_SLOT void classImplementsInterface();
    Q_SLOT void classExtends();
    Q_SLOT void staticMethod();
    Q_SLOT void ownStaticMethod();
    Q_SLOT void thisVar();
    Q_SLOT void objectFunctionCall();
    Q_SLOT void objectFunctionCall2();
    Q_SLOT void objectFunctionCall3();
    Q_SLOT void objectVariable();
    Q_SLOT void staticMemberVariable();
    Q_SLOT void ownStaticMemberVariable();
    Q_SLOT void classConst();
    Q_SLOT void classConst_data();
    Q_SLOT void fileConst();
    Q_SLOT void fileConst_data();
    Q_SLOT void define();
    Q_SLOT void defaultFunctionParam();
    Q_SLOT void globalFunction();
    Q_SLOT void globalVariableFromInternalFunctions();
    Q_SLOT void newObjectFromOtherFile();
    Q_SLOT void unknownReturnType();
    Q_SLOT void staticFunctionCallFromOtherFile();
    Q_SLOT void classConstantFromOtherFile();
    Q_SLOT void globalFunctionCallFromOtherFile();
    Q_SLOT void constantFromOtherFile();
    Q_SLOT void singleton();
    Q_SLOT void internalFunctions();
    Q_SLOT void trueFalse();
    Q_SLOT void null();
    Q_SLOT void array();
    Q_SLOT void functionDocBlock();
    Q_SLOT void variableDocBlock();
    Q_SLOT void functionDocBlockParams();
    Q_SLOT void memberFunctionDocBlockParams();
    Q_SLOT void foreachLoop();
    Q_SLOT void php4StyleConstructor();
    Q_SLOT void constructor();
    Q_SLOT void destructor();
    Q_SLOT void functionInFunction();
    Q_SLOT void objectWithClassName();
    Q_SLOT void largeNumberOfDeclarations();
    Q_SLOT void staticVariable();
    Q_SLOT void returnTypeTwoDeclarations();
    Q_SLOT void globalVariableNotVisibleInFunction();
    Q_SLOT void globalVariableInFunction();
    Q_SLOT void nonGlobalVariableInFunction();
    Q_SLOT void superglobalInFunction();
    Q_SLOT void returnWithoutFunction();
    Q_SLOT void circularInheritance();
    Q_SLOT void findDeclarations();
    Q_SLOT void memberTypeAfterMethod();
    Q_SLOT void catchDeclaration();
    Q_SLOT void resourceType();
    Q_SLOT void foreachIterator();
    Q_SLOT void foreachIterator2();
    Q_SLOT void foreachIterator3();
    Q_SLOT void foreachIterator4();
    Q_SLOT void returnThis();
    Q_SLOT void unsureReturnType();
    Q_SLOT void unsureReturnType2();
    Q_SLOT void unsureReturnType3();
    Q_SLOT void unsureReturnType4();
    Q_SLOT void referencedArgument();
    Q_SLOT void unsureReferencedArgument();
    Q_SLOT void defaultArgument();
    Q_SLOT void declareMemberOutOfClass();
    Q_SLOT void declareMemberOutOfClass2();
    Q_SLOT void declareMemberInClassMethod();
    Q_SLOT void thisRedeclaration();
    Q_SLOT void implicitArrayDeclaration();
    Q_SLOT void implicitReferenceDeclaration();
    Q_SLOT void classContextRange();
    Q_SLOT void lateClassMembers();
    Q_SLOT void list();
    Q_SLOT void alternateDocCommentTypeHints();
    Q_SLOT void findFunctionArgs();
    Q_SLOT void undeclaredPropertyInString();
    Q_SLOT void undeclaredVarPropertyInString();
    Q_SLOT void upcommingClassInString();
    Q_SLOT void namespaces();
    Q_SLOT void namespacesNoCurly();
    Q_SLOT void useNamespace();
    Q_SLOT void namespaceStaticVar();
    Q_SLOT void namespacedCatch();
    Q_SLOT void errorRecovery_data();
    Q_SLOT void errorRecovery();
    Q_SLOT void varStatic();
    Q_SLOT void staticNowdoc();
    Q_SLOT void curlyVarAfterObj();
    Q_SLOT void embeddedHTML_data();
    Q_SLOT void embeddedHTML();
    Q_SLOT void cases();
    Q_SLOT void closureParser();
    Q_SLOT void closures();
    Q_SLOT void closureEmptyUse();
    Q_SLOT void iifeParser();
    Q_SLOT void iife();
    Q_SLOT void gotoTest();
    Q_SLOT void ternary();
    Q_SLOT void bug296709();
    Q_SLOT void declareFinalMethod();
    Q_SLOT void testTodoExtractor();
    Q_SLOT void useThisAsArray();
    Q_SLOT void wrongUseOfThisAsArray();
    Q_SLOT void staticFunctionClassPhp54();
};

}

#endif
