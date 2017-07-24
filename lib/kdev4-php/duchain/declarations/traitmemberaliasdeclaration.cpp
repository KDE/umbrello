/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2014 Heinz Wiesinger <pprkut@liwjatan.at>                   *
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

#include "traitmemberaliasdeclaration.h"

#include <language/duchain/duchainregister.h>
using namespace KDevelop;

namespace Php
{

REGISTER_DUCHAIN_ITEM(TraitMemberAliasDeclaration);

TraitMemberAliasDeclaration::TraitMemberAliasDeclaration(const TraitMemberAliasDeclaration& rhs)
        : KDevelop::ClassMemberDeclaration(*new TraitMemberAliasDeclarationData(*rhs.d_func()))
{
}

TraitMemberAliasDeclaration::TraitMemberAliasDeclaration(const RangeInRevision& range, DUContext* context)
        : KDevelop::ClassMemberDeclaration(*new TraitMemberAliasDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

TraitMemberAliasDeclaration::TraitMemberAliasDeclaration(TraitMemberAliasDeclarationData& dd)
        : KDevelop::ClassMemberDeclaration(dd)
{
}

TraitMemberAliasDeclaration::TraitMemberAliasDeclaration(TraitMemberAliasDeclarationData& dd, const RangeInRevision& range)
        : KDevelop::ClassMemberDeclaration(dd, range)
{
}

TraitMemberAliasDeclaration::~TraitMemberAliasDeclaration()
{
}

Declaration* TraitMemberAliasDeclaration::clonePrivate() const
{
    return new TraitMemberAliasDeclaration(*this);
}

QString TraitMemberAliasDeclaration::toString() const
{
    if( aliasedDeclaration().isValid() )
        return aliasedDeclaration().declaration()->toString();
    else
        return i18n("Lost trait alias %1").arg(identifier().toString());
}

void TraitMemberAliasDeclaration::setAliasedDeclaration(const IndexedDeclaration& decl)
{
  d_func_dynamic()->m_aliasedDeclaration = decl;
  Declaration* aliased = decl.data();
  if(aliased)
    Declaration::setAbstractType(aliased->abstractType());
}

IndexedDeclaration TraitMemberAliasDeclaration::aliasedDeclaration() const
{
  return d_func()->m_aliasedDeclaration;
}

}
