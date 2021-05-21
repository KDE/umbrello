/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TESTPYTHONWRITER_H
#define TESTPYTHONWRITER_H

#include "testbase.h"

/**
 * Unit test for class PythonWriter (pythonwriter.h).
 */
class TestPythonWriter: public TestCodeGeneratorBase
{
    Q_OBJECT
private slots:
    void test_language();
    void test_writeClass();
    void test_reservedKeywords();
};

#endif // TESTPYTHONWRITER_H
