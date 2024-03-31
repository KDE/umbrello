/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef BENCHMARKS_H
#define BENCHMARKS_H

#include <QObject>
#include <QFile>

#include "duchaintestbase.h"

namespace Php
{
class Benchmarks : public DUChainTestBase
{
    Q_OBJECT

public:
    Benchmarks();

private:
    /// benchmarks the parser
    Q_SLOT void parser();
    /// benchmarks the declaration builder
    Q_SLOT void declarationBuilder();
    /// benchmarks the use-builder
    Q_SLOT void useBuilder();
};

}

#endif // BENCHMARKS_H
