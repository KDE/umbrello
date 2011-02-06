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

#include <QtTest>
#include <QtCore>

#include "cppwriter.h"

#include "classifier.h"

const bool IS_NOT_IMPL = false;

/**
 * Unit test for class CppWriter (cppwriter.h).
 */
class TEST_cppwriter: public QObject
{
    Q_OBJECT
private slots:
    void test_language();
    void test_writeClass();
    void test_reservedKeywords();
    void test_defaultDatatypes();
    void test_toBeImplemented();
};

//-----------------------------------------------------------------------------

void TEST_cppwriter::test_language()
{
    CppWriter* cpp = new CppWriter();
    Uml::ProgrammingLanguage lang = cpp->language();
    QCOMPARE( lang, Uml::ProgrammingLanguage::Cpp);
}

void TEST_cppwriter::test_writeClass()
{
    CppWriter* cpp = new CppWriter();
    UMLClassifier* c = new UMLClassifier("Customer", "12345678");
    UMLAttribute* attr;
    attr = c->createAttribute("name_");
    attr = c->createAttribute("address_");
    UMLOperation* op;
    op = c->createOperation("getName");
    op = c->createOperation("getAddress");

    cpp->writeClass(c);
    // does the just created file exist?
    QCOMPARE( IS_NOT_IMPL, true);
}

void TEST_cppwriter::test_reservedKeywords()
{
    CppWriter* cpp = new CppWriter();
    QStringList list = cpp->reservedKeywords();
    QCOMPARE( list.empty(), false);
    QCOMPARE( list[0], "and");
    QCOMPARE( list[11], "case");
    QCOMPARE( list.last(), "xor_eq");
}

void TEST_cppwriter::test_defaultDatatypes()
{
    CppWriter* cpp = new CppWriter();
    QStringList list = cpp->defaultDatatypes();
    QCOMPARE( list.empty(), false);
    QCOMPARE( list[0], "int");
    QCOMPARE( list[5], "short");
    QCOMPARE( list.last(), "string");
}

QTEST_MAIN(TEST_cppwriter)
#include "test_cppwriter.moc"
