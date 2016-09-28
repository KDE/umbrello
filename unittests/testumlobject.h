/*
    Copyright 2015  Ralf Habacker  <ralf.habacker@freenet.de>

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

#ifndef TESTUMLOBJECT_H
#define TESTUMLOBJECT_H

#include "testbase.h"

class TestUMLObject : public TestBase
{
    Q_OBJECT
private slots:
    void test_copyInto();
    void test_clone();
    void test_doc();
    void test_equal();
    void test_fullyQualifiedName();
    void test_isAbstract();
    void test_isStatic();
    void test_resolveRef();
    void test_saveAndLoad();
    void test_setBaseType();
    void test_setSterotype();
    void test_setUMLPackage();
    void test_setVisibility();
    void test_toString();
};

#endif // TESTUMLOBJECT_H
