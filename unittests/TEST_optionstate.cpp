#include "TEST_optionstate.h"

#include "optionstate.h"

void TEST_optionstate::test_create()
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

void TEST_optionstate::test_saveAndLoad()
{
    Settings::OptionState options;
    options.classState.showAtts = true;
    options.classState.showOps = true;
    options.uiState.useFillColor = true;

    QDomDocument doc;
    QDomElement element = doc.createElement(QLatin1String("test"));
    Settings::saveToXMI(element, options);

    Settings::OptionState optionsB;
    QCOMPARE(Settings::loadFromXMI(element, optionsB), true);
    QCOMPARE(optionsB.classState.showAtts, true);
    QCOMPARE(optionsB.classState.showOps, true);
    QCOMPARE(optionsB.uiState.useFillColor, true);
}

QTEST_MAIN(TEST_optionstate)
