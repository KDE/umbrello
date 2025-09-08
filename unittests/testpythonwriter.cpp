/*
    SPDX-FileCopyrightText: 2011 Andi Fischer <andi.fischer@hispeed.ch>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testpythonwriter.h"

// app includes
#include "umlclassifier.h"
#include "pythonwriter.h"

const bool IS_NOT_IMPL = false;

//-----------------------------------------------------------------------------

class PythonWriterTest : public PythonWriter
{
public:
    QString findFileName(UMLPackage* classifier, const QString &ext)
    {
       return PythonWriter::findFileName(classifier, ext);
    }
};

void TestPythonWriter::test_language()
{
    PythonWriter* py = new PythonWriter();
    Uml::ProgrammingLanguage::Enum lang = py->language();
    QVERIFY(lang == Uml::ProgrammingLanguage::Python);
}

void TestPythonWriter::test_writeClass()
{
    PythonWriterTest* py = new PythonWriterTest();
    UMLClassifier* c = new UMLClassifier(QStringLiteral("Customer"), "12345678");
    UMLAttribute* attr;
    attr = c->createAttribute(QStringLiteral("name_"));
    attr = c->createAttribute(QStringLiteral("address_"));
    c->addAttribute(attr);

    py->writeClass(c);
    // does the just created file exist?
    QFile file(temporaryPath() + py->findFileName(c, QStringLiteral(".py")));
    QCOMPARE(file.exists(), true);
}

void TestPythonWriter::test_reservedKeywords()
{
    PythonWriter* py = new PythonWriter();
    QStringList list = py->reservedKeywords();
    QCOMPARE(list.empty(), false);
    QCOMPARE(list[0], QStringLiteral("abs"));
    QCOMPARE(list[11], QStringLiteral("class"));
    QCOMPARE(list.last(), QStringLiteral("zip"));
}

QTEST_MAIN(TestPythonWriter)
