/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef PHP_BENCHMARKCODECOMPLETION_H
#define PHP_BENCHMARKCODECOMPLETION_H

#include "../../duchain/tests/duchaintestbase.h"

namespace Php {

class BenchmarkCodeCompletion : public DUChainTestBase
{
    Q_OBJECT

public Q_SLOTS:
    // don't do the stuff we do in the DUChainTestBase
    void initTestCase();

private Q_SLOTS:
    void globalCompletion();
    void globalCompletionBigFile();
    void completionData();
};

}

#endif // PHP_BENCHMARKCODECOMPLETION_H
