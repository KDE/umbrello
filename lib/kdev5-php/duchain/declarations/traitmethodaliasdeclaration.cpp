/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2014 Heinz Wiesinger <pprkut@liwjatan.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "traitmethodaliasdeclaration.h"

#include <language/duchain/duchainregister.h>
using namespace KDevelop;

namespace Php
{

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(TraitMethodAliasDeclarationData, items, IndexedQualifiedIdentifier)

class KDEVPHPDUCHAIN_EXPORT TraitMethodAliasDeclarationData : public ClassMethodDeclarationData
{
public:
    TraitMethodAliasDeclarationData()
            : ClassMethodDeclarationData()
    {
        initializeAppendedLists();
    }

    TraitMethodAliasDeclarationData(const TraitMethodAliasDeclarationData& rhs)
            : ClassMethodDeclarationData(rhs)
    {
        m_aliasedDeclaration = rhs.m_aliasedDeclaration;
        initializeAppendedLists();
        copyListsFrom(rhs);
    }

    ~TraitMethodAliasDeclarationData()
    {
        freeAppendedLists();
    }

    IndexedDeclaration m_aliasedDeclaration;
    START_APPENDED_LISTS_BASE(TraitMethodAliasDeclarationData, ClassMethodDeclarationData);
    APPENDED_LIST_FIRST(TraitMethodAliasDeclarationData, IndexedQualifiedIdentifier, items);
    END_APPENDED_LISTS(TraitMethodAliasDeclarationData, items);
};


DEFINE_LIST_MEMBER_HASH(TraitMethodAliasDeclarationData, items, IndexedQualifiedIdentifier)

REGISTER_DUCHAIN_ITEM(TraitMethodAliasDeclaration);

TraitMethodAliasDeclaration::TraitMethodAliasDeclaration(const TraitMethodAliasDeclaration& rhs)
        : ClassMethodDeclaration(*new TraitMethodAliasDeclarationData(*rhs.d_func()))
{
}

TraitMethodAliasDeclaration::TraitMethodAliasDeclaration(const RangeInRevision& range, DUContext* context)
        : ClassMethodDeclaration(*new TraitMethodAliasDeclarationData, range, context)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

TraitMethodAliasDeclaration::TraitMethodAliasDeclaration(TraitMethodAliasDeclarationData& data)
        : ClassMethodDeclaration(data)
{
}

TraitMethodAliasDeclaration::TraitMethodAliasDeclaration(TraitMethodAliasDeclarationData& data, const RangeInRevision& range, DUContext* context)
        : ClassMethodDeclaration(data, range, context)
{
}

TraitMethodAliasDeclaration::~TraitMethodAliasDeclaration()
{
}

Declaration* TraitMethodAliasDeclaration::clonePrivate() const
{
    return new TraitMethodAliasDeclaration(*this);
}

QString TraitMethodAliasDeclaration::toString() const
{
    if( aliasedDeclaration().isValid() )
        return aliasedDeclaration().declaration()->toString();
    else
        return i18n("Lost trait alias %1").arg(identifier().toString());
}

void TraitMethodAliasDeclaration::setAliasedDeclaration(const IndexedDeclaration& decl)
{
  d_func_dynamic()->m_aliasedDeclaration = decl;
  Declaration* aliased = decl.data();
  if(aliased)
    Declaration::setAbstractType(aliased->abstractType());
}

IndexedDeclaration TraitMethodAliasDeclaration::aliasedDeclaration() const
{
  return d_func()->m_aliasedDeclaration;
}

void TraitMethodAliasDeclaration::setOverrides(const QVector< IndexedQualifiedIdentifier>& ids)
{
    d_func_dynamic()->itemsList().clear();
    foreach (const IndexedQualifiedIdentifier& id, ids) {
        d_func_dynamic()->itemsList().append(id);
    }
}


bool TraitMethodAliasDeclaration::isOverriding(const IndexedQualifiedIdentifier& id) const
{
    FOREACH_FUNCTION(const IndexedQualifiedIdentifier& list, d_func()->items) {
        if (list == id) {
            return true;
        }
    }
    return false;
}

}
