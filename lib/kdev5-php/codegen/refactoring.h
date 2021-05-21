/* This file is part of KDevelop

    SPDX-FileCopyrightText: 2014 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#ifndef REFACTORING_H
#define REFACTORING_H


#include <interfaces/context.h>
#include <language/codegen/basicrefactoring.h>


namespace Php
{

class Refactoring : public KDevelop::BasicRefactoring
{
public:
    explicit Refactoring(QObject *parent = NULL);
};

} // End of namespace Php


#endif /* REFACTORING_H */
