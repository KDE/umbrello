/* This file is part of KDevelop
    Copyright 2010 Niko Sams <niko.sams@gmail.com>

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

#ifndef TESTDUCHAINMULTIPLEFILES_H
#define TESTDUCHAINMULTIPLEFILES_H

#include "duchain/tests/duchaintestbase.h"

namespace KDevelop {
    class TestCore;
    class TestProjectController;
}

namespace Php
{

class TestDUChainMultipleFiles : public DUChainTestBase
{
    Q_OBJECT
private slots:
    void initTestCase();

    void testImportsGlobalFunction();
    void testImportsBaseClassNotYetParsed();
    void testNonExistingBaseClass();
    void testImportsGlobalFunctionNotYetParsed();
    void testNonExistingGlobalFunction();
    void testImportsStaticFunctionNotYetParsed();
    void testNonExistingStaticFunction();
    void testForeachImportedIdentifier();
    void testUpdateForeach();
private:
    KDevelop::TestProjectController* m_projectController;
};

}

#endif
