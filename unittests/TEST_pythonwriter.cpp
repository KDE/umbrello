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

#include "pythonwriter.h"

#include "classifier.h"

const bool IS_NOT_IMPL = false;

/**
 * Unit test for class PythonWriter (pythonwriter.h).
 */
class TEST_pythonwriter: public QObject
{
    Q_OBJECT
private slots:
    void test_language();
    void test_writeClass();
    void test_reservedKeywords();
    void test_toBeImplemented();
};

//-----------------------------------------------------------------------------

void TEST_pythonwriter::test_language()
{
    PythonWriter* py = new PythonWriter();
    Uml::ProgrammingLanguage lang = py->language();
    QCOMPARE( lang, Uml::ProgrammingLanguage::Python);
}

void TEST_pythonwriter::test_writeClass()
{
    PythonWriter* py = new PythonWriter();
    UMLClassifier* c = new UMLClassifier("Customer", "12345678");
    UMLAttribute* attr;
    attr = c->createAttribute("name_");
    attr = c->createAttribute("address_");

    py->writeClass(c);
    // does the just created file exist?
    QCOMPARE( IS_NOT_IMPL, true);
}

void TEST_pythonwriter::test_reservedKeywords()
{
    PythonWriter* py = new PythonWriter();
    QStringList list = py->reservedKeywords();
    QCOMPARE( list.empty(), false);
    QCOMPARE( list[0], "abs");
    QCOMPARE( list[11], "class");
    QCOMPARE( list.last(), "zip");
}

void TEST_pythonwriter::test_toBeImplemented()
{
    QCOMPARE( IS_NOT_IMPL, true );
}

QTEST_MAIN(TEST_pythonwriter)
#include "test_pythonwriter.moc"
