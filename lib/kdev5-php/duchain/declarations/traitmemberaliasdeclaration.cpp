/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2014 Heinz Wiesinger <pprkut@liwjatan.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
