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

#ifndef TRAITMETHODALIASDECLARATION_H
#define TRAITMETHODALIASDECLARATION_H

#include "classmethoddeclaration.h"

namespace Php
{

struct TraitMethodAliasDeclarationData;

/**
 * inherits ClassMethodDeclaration to allow some trait specific functionality
 */
class KDEVPHPDUCHAIN_EXPORT TraitMethodAliasDeclaration : public ClassMethodDeclaration
{
public:
    TraitMethodAliasDeclaration(const TraitMethodAliasDeclaration &rhs);
    TraitMethodAliasDeclaration(const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    TraitMethodAliasDeclaration(TraitMethodAliasDeclarationData &data);
    TraitMethodAliasDeclaration(TraitMethodAliasDeclarationData &data, const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    ~TraitMethodAliasDeclaration();

    virtual QString toString() const;

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

    /**
     * Set the identifier for a context that contains a declaration overridden by this one.
     *
     * \param id the identifier of the context that contains an overridden declaration
     */
    void setOverrides(const QVector< KDevelop::IndexedQualifiedIdentifier >& ids);

    /**
     * Check whether a given identifier is registered as containing an overridden declaration
     *
     * \returns whether a given identifier is registered as containing an overridden declaration
     */
    bool isOverriding(const KDevelop::IndexedQualifiedIdentifier& id) const;

    enum {
        Identity = 131
    };

    virtual KDevelop::Declaration* clonePrivate() const;
private:
    DUCHAIN_DECLARE_DATA(TraitMethodAliasDeclaration)
};

}

#endif // TRAITMETHODALIASDECLARATION_H

