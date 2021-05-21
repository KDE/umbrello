/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testcppwriter.h"

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

void TestCppWriter::test_language()
{
    CppWriter* cpp = new CppWriter();
    Uml::ProgrammingLanguage::Enum lang = cpp->language();
    QVERIFY(lang == Uml::ProgrammingLanguage::Cpp);
}

void TestCppWriter::test_writeClass()
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

void TestCppWriter::test_reservedKeywords()
{
    CppWriter* cpp = new CppWriter();
    QStringList list = cpp->reservedKeywords();
    QCOMPARE(list.empty(), false);
    QCOMPARE(list[0], QLatin1String("and"));
    QCOMPARE(list[11], QLatin1String("case"));
    QCOMPARE(list.last(), QLatin1String("xor_eq"));
}

void TestCppWriter::test_defaultDatatypes()
{
    CppWriter* cpp = new CppWriter();
    QStringList list = cpp->defaultDatatypes();
    QCOMPARE(list.empty(), false);
    QCOMPARE(list[1], QLatin1String("int"));
    QCOMPARE(list[10], QLatin1String("short int"));
    QCOMPARE(list[5], QLatin1String("string"));
}

QTEST_MAIN(TestCppWriter)
