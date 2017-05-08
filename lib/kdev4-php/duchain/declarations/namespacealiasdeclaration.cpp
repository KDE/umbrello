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

#include "namespacealiasdeclaration.h"

#include <language/duchain/duchainregister.h>
#include <completioncodemodel.h>

#include "helper.h"

namespace Php {
REGISTER_DUCHAIN_ITEM(NamespaceAliasDeclaration);

NamespaceAliasDeclaration::NamespaceAliasDeclaration(const NamespaceAliasDeclaration& rhs)
        : KDevelop::NamespaceAliasDeclaration(*new NamespaceAliasDeclarationData(*rhs.d_func()))
{
}

NamespaceAliasDeclaration::NamespaceAliasDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::NamespaceAliasDeclaration(*new NamespaceAliasDeclarationData)
{
    setRange(range);
    d_func_dynamic()->setClassId(this);
    if (context) {
        setContext(context);
    }
}

NamespaceAliasDeclaration::NamespaceAliasDeclaration(NamespaceAliasDeclarationData& data)
        : KDevelop::NamespaceAliasDeclaration(data)
{
}

NamespaceAliasDeclaration::~NamespaceAliasDeclaration()
{
}

KDevelop::Declaration* NamespaceAliasDeclaration::clonePrivate() const
{
    return new NamespaceAliasDeclaration(*this);
}

KDevelop::IndexedString NamespaceAliasDeclaration::prettyName() const
{
    return d_func()->prettyName;
}

void NamespaceAliasDeclaration::setPrettyName( const KDevelop::IndexedString& name )
{
    bool wasInSymbolTable = d_func()->m_inSymbolTable;
    setInSymbolTable(false);
    d_func_dynamic()->prettyName = name;
    setInSymbolTable(wasInSymbolTable);
}

QString NamespaceAliasDeclaration::toString() const
{
  return QString("Import %1 as %2").arg(d_func()->m_importIdentifier.identifier().toString()).arg(prettyName().str());
}

}
