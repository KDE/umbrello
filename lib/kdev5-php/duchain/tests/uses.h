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

private:
    Q_SLOT void newObject();
    Q_SLOT void functionCall();
    Q_SLOT void memberFunctionCall();
    Q_SLOT void memberVariable();
    Q_SLOT void variable();
    Q_SLOT void varInString();
    Q_SLOT void variableInNamespace();
    Q_SLOT void globalVariableInNamespace();
    Q_SLOT void variableInOtherNamespace();
    Q_SLOT void memberVarInString();
    Q_SLOT void memberFunctionInString();
    Q_SLOT void variableTypeChange();
    Q_SLOT void variableTypeChangeInFunction();
    Q_SLOT void classExtends();
    Q_SLOT void classImplements();
    Q_SLOT void classImplementsMultiple();
    Q_SLOT void interfaceExtends();
    Q_SLOT void interfaceExtendsMultiple();
    Q_SLOT void staticMemberFunctionCall();
    Q_SLOT void staticMemberVariable();
    Q_SLOT void constant();
    Q_SLOT void classConstant();
    Q_SLOT void classParent();
    Q_SLOT void classSelf();
    Q_SLOT void classThis();
    Q_SLOT void objectWithClassName();
    Q_SLOT void classAndConstWithSameName();
    Q_SLOT void classAndFunctionWithSameName();
    Q_SLOT void constAndVariableWithSameName();
    Q_SLOT void functionAndClassWithSameName();
    Q_SLOT void constantInClassMember();
    Q_SLOT void useInAsignment();
    Q_SLOT void foreachArray();
    Q_SLOT void assignmentToMemberArray();
    Q_SLOT void staticArrayIndex();
    Q_SLOT void functionParamNewDeclaration();
    Q_SLOT void catchClass();
    Q_SLOT void variableRedeclaration();
    Q_SLOT void caseInsensitiveFunction();
    Q_SLOT void caseInsensitiveMethod();
    Q_SLOT void caseInsensitiveClass();
    Q_SLOT void functionUseBeforeDeclaration();
    Q_SLOT void propertyAndMethodWithSameName();
    Q_SLOT void nestedMethodCalls();
    Q_SLOT void unset();
    Q_SLOT void functionArguments();
    Q_SLOT void namespaces();
    Q_SLOT void useNamespace();
    Q_SLOT void lateStatic();
    Q_SLOT void closures();
    Q_SLOT void instanceof();
    Q_SLOT void classNameString();
    Q_SLOT void useTrait();
    Q_SLOT void exceptionFinally();
};

}

#endif
