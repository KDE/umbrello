/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "testoptionstate.h"

#include "optionstate.h"

void TestOptionState::test_create()
{
    Settings::OptionState options;
    QCOMPARE(options.classState.showAtts, false);
    QCOMPARE(options.classState.showOps, false);
    QCOMPARE(options.uiState.useFillColor, false);

    QScopedPointer<Settings::OptionState> optionsA(new Settings::OptionState);
    QCOMPARE(optionsA->classState.showAtts, false);
    QCOMPARE(optionsA->classState.showOps, false);
    QCOMPARE(optionsA->uiState.useFillColor, false);
}

void TestOptionState::test_saveAndLoad()
{
    Settings::OptionState options;
    options.classState.showAtts = true;
    options.classState.showOps = true;
    options.uiState.useFillColor = true;

    // save
    QString xml;
    QXmlStreamWriter stream(&xml);
    stream.writeStartElement(QStringLiteral("test"));
    options.saveToXMI(stream);
    stream.writeEndElement();

    // convert XML string to QDomElement
    QDomDocument doc;
    QString error;
    int line;
    QVERIFY(doc.setContent(xml, &error, &line));
    QDomElement element = doc.firstChild().toElement();

    // load
    Settings::OptionState optionsB;
    QCOMPARE(optionsB.loadFromXMI(element), true);
    QCOMPARE(optionsB.classState.showAtts, true);
    QCOMPARE(optionsB.classState.showOps, true);
    QCOMPARE(optionsB.uiState.useFillColor, true);
}

QTEST_MAIN(TestOptionState)
