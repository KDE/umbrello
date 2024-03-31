/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TESTEXPRESSIONPARSER_H
#define TESTEXPRESSIONPARSER_H

#include "../../duchain/tests/duchaintestbase.h"

namespace Php
{
class TestExpressionParser : public DUChainTestBase
{
    Q_OBJECT

public:
    TestExpressionParser();

private:
    Q_SLOT void newClass();
    Q_SLOT void newSelf();
    Q_SLOT void newStatic();
    Q_SLOT void memberVariable();
    Q_SLOT void memberFunction();
    Q_SLOT void newTrait();
    Q_SLOT void newTraitWithAbstractMethod();
    Q_SLOT void invalidTrait_data();
    Q_SLOT void invalidTrait();
    Q_SLOT void invalidTraitUse_data();
    Q_SLOT void invalidTraitUse();
    Q_SLOT void namespaceUseNameConflict();
    Q_SLOT void globalFunction();
    Q_SLOT void chainCall();
    Q_SLOT void thisObject();
    Q_SLOT void integralTypes();
    Q_SLOT void newObject();
    Q_SLOT void cast();
    Q_SLOT void operations();
    Q_SLOT void findArg();
    Q_SLOT void array_data();
    Q_SLOT void array();
    Q_SLOT void arrayFunctionDereferencing_data();
    Q_SLOT void arrayFunctionDereferencing();
    Q_SLOT void arrayLiteralDereferencing_data();
    Q_SLOT void arrayLiteralDereferencing();
    Q_SLOT void stringAsArray_data();
    Q_SLOT void stringAsArray();
    Q_SLOT void classMemberOnInstantiation();
    Q_SLOT void classNameConstant_data();
    Q_SLOT void classNameConstant();
};

}

#endif
