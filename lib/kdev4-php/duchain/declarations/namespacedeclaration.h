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

#ifndef PHP_NAMESPACEDECLARATION_H
#define PHP_NAMESPACEDECLARATION_H

#include <language/duchain/declaration.h>

#include <language/duchain/declarationdata.h>
#include <language/duchain/indexedstring.h>

#include "phpduchainexport.h"
namespace Php
{

class KDEVPHPDUCHAIN_EXPORT NamespaceDeclarationData : public KDevelop::DeclarationData
{
public:
    NamespaceDeclarationData()
            : KDevelop::DeclarationData() {}

    NamespaceDeclarationData(const NamespaceDeclarationData& rhs)
            : KDevelop::DeclarationData(rhs)
    {
        prettyName = rhs.prettyName;
    }

    ~NamespaceDeclarationData() {}

    KDevelop::IndexedString prettyName;
};

class KDEVPHPDUCHAIN_EXPORT NamespaceDeclaration : public KDevelop::Declaration
{
public:
    NamespaceDeclaration(const NamespaceDeclaration &rhs);
    NamespaceDeclaration(const KDevelop::RangeInRevision &range, KDevelop::DUContext *parentContext);
    NamespaceDeclaration(NamespaceDeclarationData &data);
    virtual ~NamespaceDeclaration();

    virtual QString toString() const;

    void setPrettyName(const KDevelop::IndexedString& name);
    KDevelop::IndexedString prettyName() const;

    enum {
        Identity = 87
    };

    virtual KDevelop::Declaration* clonePrivate() const;
private:
    DUCHAIN_DECLARE_DATA(NamespaceDeclaration)
};

}

#endif // PHP_NAMESPACEDECLARATION_H
