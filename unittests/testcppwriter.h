/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTCPPWRITER_H
#define TESTCPPWRITER_H

#include "testbase.h"

/**
 * Unit test for class CppWriter (cppwriter.h).
 */
class TestCppWriter: public TestCodeGeneratorBase
{
    Q_OBJECT
private Q_SLOTS:
    void test_language();
    void test_writeClass();
    void test_reservedKeywords();
    void test_defaultDatatypes();
};

#endif // TESTCPPWRITER_H
