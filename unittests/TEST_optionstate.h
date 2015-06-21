#ifndef TEST_OPTIONSTATE_H
#define TEST_OPTIONSTATE_H

#include "testbase.h"

class TEST_optionstate : public TestBase
{
    Q_OBJECT

private slots:
    void test_create();
    void test_saveAndLoad();
};

#endif // TEST_OPTIONSTATE_H
