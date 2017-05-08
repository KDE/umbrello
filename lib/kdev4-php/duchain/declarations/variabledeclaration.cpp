/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Niko Sams <niko.sams@gmail.com>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

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
