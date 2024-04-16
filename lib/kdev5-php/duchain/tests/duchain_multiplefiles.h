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
private Q_SLOTS:
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
    void testTodoExtractorReparse();
    void testIteratorForeachReparse();
private:
    KDevelop::TestProjectController* m_projectController;
};

}

#endif
