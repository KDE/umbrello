/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef BENCHMARKS_H
#define BENCHMARKS_H

#include <QtCore/QObject>
#include <QtCore/QFile>

#include "duchaintestbase.h"

namespace Php
{
class Benchmarks : public DUChainTestBase
{
    Q_OBJECT

public:
    Benchmarks();

private slots:
    /// benchmarks the parser
    void parser();
    /// benchmarks the declaration builder
    void declarationBuilder();
    /// benchmarks the use-builder
    void useBuilder();
};

}

#endif // BENCHMARKS_H
