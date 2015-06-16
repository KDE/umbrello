/*
    Copyright 2011  Andi Fischer  <andi.fischer@hispeed.ch>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TEST_CPPWRITER_H
#define TEST_CPPWRITER_H

#include "testbase.h"

/**
 * Unit test for class CppWriter (cppwriter.h).
 */
class TEST_cppwriter: public TestBase
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test_language();
    void test_writeClass();
    void test_reservedKeywords();
    void test_defaultDatatypes();
};

#endif // TEST_CPPWRITER_H
