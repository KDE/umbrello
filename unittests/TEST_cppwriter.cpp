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

#include "TEST_cppwriter.h"

// app include
#include "classifier.h"
#include "cppwriter.h"

const bool IS_NOT_IMPL = false;

//-----------------------------------------------------------------------------

class CppWriterTest : public CppWriter
{
public:
    QString findFileName(UMLPackage* concept, const QString &ext)
    {
       return CppWriter::findFileName(concept,ext);
    }
};

void TEST_cppwriter::test_language()
{
    CppWriter* cpp = new CppWriter();
    Uml::ProgrammingLanguage::Enum lang = cpp->language();
    QCOMPARE(lang, Uml::ProgrammingLanguage::Cpp);
}

void TEST_cppwriter::test_writeClass()
{
    CppWriterTest* cpp = new CppWriterTest();
    UMLClassifier* c = new UMLClassifier("Customer", "12345678");
    UMLAttribute* attr;
    attr = c->createAttribute("name_");
    attr = c->createAttribute("address_");
    c->addAttribute(attr);
    UMLOperation* op;
    op = c->createOperation("getName");
    op = c->createOperation("getAddress");
    c->addOperation(op);

    cpp->writeClass(c);
    // does the just created file exist?
    QFile fileHeader(temporaryPath() + cpp->findFileName(c, QLatin1String(".h")));
    QFile fileCPP(temporaryPath() + cpp->findFileName(c, QLatin1String(".cpp")));
    QCOMPARE(fileHeader.exists(), true);
    QCOMPARE(fileCPP.exists(), true);
}

void TEST_cppwriter::test_reservedKeywords()
{
    CppWriter* cpp = new CppWriter();
    QStringList list = cpp->reservedKeywords();
    QCOMPARE(list.empty(), false);
    QCOMPARE(list[0], QLatin1String("and"));
    QCOMPARE(list[11], QLatin1String("case"));
    QCOMPARE(list.last(), QLatin1String("xor_eq"));
}

void TEST_cppwriter::test_defaultDatatypes()
{
    CppWriter* cpp = new CppWriter();
    QStringList list = cpp->defaultDatatypes();
    QCOMPARE(list.empty(), false);
    QCOMPARE(list[0], QLatin1String("int"));
    QCOMPARE(list[5], QLatin1String("short"));
    QCOMPARE(list.last(), QLatin1String("string"));
}

QTEST_MAIN(TEST_cppwriter)
