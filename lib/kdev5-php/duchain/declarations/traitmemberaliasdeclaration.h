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


#ifndef TRAITMEMBERALIASDECLARATION_H
#define TRAITMEMBERALIASDECLARATION_H

#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classmemberdeclarationdata.h>

#include "phpduchainexport.h"

namespace Php
{

class KDEVPHPDUCHAIN_EXPORT TraitMemberAliasDeclarationData : public KDevelop::ClassMemberDeclarationData
{
public:
    TraitMemberAliasDeclarationData()
            : KDevelop::ClassMemberDeclarationData() {}

    TraitMemberAliasDeclarationData( const TraitMemberAliasDeclarationData& rhs )
            : KDevelop::ClassMemberDeclarationData(rhs)
    {
        m_aliasedDeclaration = rhs.m_aliasedDeclaration;
    }

    KDevelop::IndexedDeclaration m_aliasedDeclaration;
};

/**
 * Represents a single class member definition in a definition-use chain.
 */
class KDEVPHPDUCHAIN_EXPORT TraitMemberAliasDeclaration : public KDevelop::ClassMemberDeclaration
{
public:
    TraitMemberAliasDeclaration(const TraitMemberAliasDeclaration& rhs);
    TraitMemberAliasDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    TraitMemberAliasDeclaration(TraitMemberAliasDeclarationData& dd);
    ~TraitMemberAliasDeclaration();

    QString toString() const;

    /**
     * Set the declaration that is aliased by this declaration.
     *
     * \param decl the declaration that this declaration references
     */
    void setAliasedDeclaration(const KDevelop::IndexedDeclaration& decl);

    /**
     * Access the declaration that is aliased by this declaration.
     *
     * \returns the aliased declaration
     */
    KDevelop::IndexedDeclaration aliasedDeclaration() const;

    enum {
        Identity = 130
    };

protected:
    TraitMemberAliasDeclaration(TraitMemberAliasDeclarationData& dd, const KDevelop::RangeInRevision& range);

private:
    DUCHAIN_DECLARE_DATA(TraitMemberAliasDeclaration)
    virtual KDevelop::Declaration* clonePrivate() const;

};
}

#endif // TRAITMEMBERALIASDECLARATION_H

