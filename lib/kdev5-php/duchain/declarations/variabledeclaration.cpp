/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "variabledeclaration.h"

#include <language/duchain/ducontext.h>
#include <language/duchain/duchainregister.h>

using namespace KDevelop;
namespace Php
{
REGISTER_DUCHAIN_ITEM(VariableDeclaration);

VariableDeclaration::VariableDeclaration(VariableDeclarationData& data) : KDevelop::Declaration(data)
{
}

VariableDeclaration::VariableDeclaration(VariableDeclarationData& data, const KDevelop::RangeInRevision& range)
        : KDevelop::Declaration(data, range)
{
}

VariableDeclaration::VariableDeclaration(const VariableDeclaration& rhs)
        : KDevelop::Declaration(*new VariableDeclarationData(*rhs.d_func()))
{
}

VariableDeclaration::VariableDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::Declaration(*new VariableDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}
VariableDeclaration::~VariableDeclaration()
{
}

uint VariableDeclaration::additionalIdentity() const
{
    return 2;
}

KDevelop::DeclarationId VariableDeclaration::id(bool forceDirect) const
{
    Q_UNUSED(forceDirect);
    //always forceDirect, because there can exist multiple declarations with the
    //same identifier within one context, and uses wouldn't work correctly else
    return KDevelop::Declaration::id(true);
}

bool VariableDeclaration::isSuperglobal() const
{
    DUCHAIN_D(VariableDeclaration);
    return d->m_isSuperglobal;
}

void VariableDeclaration::setSuperglobal(bool superglobal)
{
    DUCHAIN_D_DYNAMIC(VariableDeclaration);
    d->m_isSuperglobal = superglobal;
}

}
