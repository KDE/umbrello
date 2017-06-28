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
