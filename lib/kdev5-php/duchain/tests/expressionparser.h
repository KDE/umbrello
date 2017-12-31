/* This file is part of KDevelop
    Copyright 2008 Niko Sams <niko.sams@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
