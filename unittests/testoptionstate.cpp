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

    QDomDocument doc;
    QDomElement element = doc.createElement(QLatin1String("test"));
    options.saveToXMI(element);

    Settings::OptionState optionsB;
    QCOMPARE(optionsB.loadFromXMI(element), true);
    QCOMPARE(optionsB.classState.showAtts, true);
    QCOMPARE(optionsB.classState.showOps, true);
    QCOMPARE(optionsB.uiState.useFillColor, true);
}

QTEST_MAIN(TestOptionState)
