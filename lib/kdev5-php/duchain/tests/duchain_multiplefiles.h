/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2010 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef TESTDUCHAINMULTIPLEFILES_H
#define TESTDUCHAINMULTIPLEFILES_H

#include "duchaintestbase.h"

namespace KDevelop {
    class TestCore;
    class TestProjectController;
}

namespace Php
{

class TestDUChainMultipleFiles : public DUChainTestBase
{
    Q_OBJECT
private:
    Q_SLOT void initTestCase();

    Q_SLOT void testImportsGlobalFunction();
    Q_SLOT void testImportsBaseClassNotYetParsed();
    Q_SLOT void testNonExistingBaseClass();
    Q_SLOT void testImportsGlobalFunctionNotYetParsed();
    Q_SLOT void testNonExistingGlobalFunction();
    Q_SLOT void testImportsStaticFunctionNotYetParsed();
    Q_SLOT void testNonExistingStaticFunction();
    Q_SLOT void testForeachImportedIdentifier();
    Q_SLOT void testUpdateForeach();
    Q_SLOT void testTodoExtractorReparse();
    Q_SLOT void testIteratorForeachReparse();
private:
    KDevelop::TestProjectController* m_projectController;
};

}

#endif
