/* This file is part of KDevelop

    SPDX-FileCopyrightText: 2014 Miquel Sabaté <mikisabate@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


#include <QObject>
#include <codegen/refactoring.h>


namespace Php
{

Refactoring::Refactoring(QObject *parent)
    : BasicRefactoring(parent)
{
    /* There's nothing to do here. */
}

} // End of namespace Php
