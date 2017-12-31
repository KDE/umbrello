/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Milian Wolff <mail@milianw.de>                         *
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

#ifndef PHP_NAMESPACEALIASDECLARATION_H
#define PHP_NAMESPACEALIASDECLARATION_H

#include <language/duchain/namespacealiasdeclaration.h>

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <serialization/indexedstring.h>

#include "phpduchainexport.h"
namespace Php
{

class KDEVPHPDUCHAIN_EXPORT NamespaceAliasDeclarationData : public KDevelop::NamespaceAliasDeclarationData
{
public:
    NamespaceAliasDeclarationData()
            : KDevelop::NamespaceAliasDeclarationData() {}

    NamespaceAliasDeclarationData(const NamespaceAliasDeclarationData& rhs)
            : KDevelop::NamespaceAliasDeclarationData(rhs)
    {
        prettyName = rhs.prettyName;
    }

    ~NamespaceAliasDeclarationData() {}

    KDevelop::IndexedString prettyName;
};

class KDEVPHPDUCHAIN_EXPORT NamespaceAliasDeclaration : public KDevelop::NamespaceAliasDeclaration
{
public:
    NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs);
    NamespaceAliasDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    NamespaceAliasDeclaration(NamespaceAliasDeclarationData& data);
    virtual ~NamespaceAliasDeclaration();

    virtual QString toString() const;

    void setPrettyName(const KDevelop::IndexedString& name);
    KDevelop::IndexedString prettyName() const;

    enum {
        Identity = 88
    };

    virtual KDevelop::Declaration* clonePrivate() const;
private:
    DUCHAIN_DECLARE_DATA(NamespaceAliasDeclaration)
};

}

#endif // PHP_NAMESPACEALIASDECLARATION_H
