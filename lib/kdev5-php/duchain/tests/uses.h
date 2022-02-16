/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TESTUSES_H
#define TESTUSES_H

#include "../../duchain/tests/duchaintestbase.h"

namespace Php
{
class TestUses : public DUChainTestBase
{
    Q_OBJECT

public:
    TestUses();

private slots:
    void newObject();
    void functionCall();
    void memberFunctionCall();
    void memberVariable();
    void variable();
    void varInString();
    void variableInNamespace();
    void globalVariableInNamespace();
    void variableInOtherNamespace();
    void memberVarInString();
    void memberFunctionInString();
    void variableTypeChange();
    void variableTypeChangeInFunction();
    void classExtends();
    void classImplements();
    void classImplementsMultiple();
    void interfaceExtends();
    void interfaceExtendsMultiple();
    void staticMemberFunctionCall();
    void staticMemberVariable();
    void constant();
    void classConstant();
    void classParent();
    void classSelf();
    void classThis();
    void objectWithClassName();
    void classAndConstWithSameName();
    void classAndFunctionWithSameName();
    void constAndVariableWithSameName();
    void functionAndClassWithSameName();
    void constantInClassMember();
    void useInAsignment();
    void foreachArray();
    void assignmentToMemberArray();
    void staticArrayIndex();
    void functionParamNewDeclaration();
    void catchClass();
    void variableRedeclaration();
    void caseInsensitiveFunction();
    void caseInsensitiveMethod();
    void caseInsensitiveClass();
    void functionUseBeforeDeclaration();
    void propertyAndMethodWithSameName();
    void nestedMethodCalls();
    void unset();
    void functionArguments();
    void namespaces();
    void useNamespace();
    void lateStatic();
    void closures();
    void instanceof();
    void classNameString();
    void useTrait();
    void exceptionFinally();
};

}

#endif
