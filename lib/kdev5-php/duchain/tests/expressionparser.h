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

private slots:
    void newClass();
    void newSelf();
    void newStatic();
    void memberVariable();
    void memberFunction();
    void newTrait();
    void newTraitWithAbstractMethod();
    void invalidTrait_data();
    void invalidTrait();
    void invalidTraitUse_data();
    void invalidTraitUse();
    void namespaceUseNameConflict();
    void globalFunction();
    void chainCall();
    void thisObject();
    void integralTypes();
    void newObject();
    void cast();
    void operations();
    void findArg();
    void array_data();
    void array();
    void arrayFunctionDereferencing_data();
    void arrayFunctionDereferencing();
    void arrayLiteralDereferencing_data();
    void arrayLiteralDereferencing();
    void stringAsArray_data();
    void stringAsArray();
    void classMemberOnInstantiation();
    void classNameConstant_data();
    void classNameConstant();
};

}

#endif
