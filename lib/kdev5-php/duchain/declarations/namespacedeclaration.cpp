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

#include "namespacedeclaration.h"

#include <language/duchain/duchainregister.h>
#include <completioncodemodel.h>

#include "helper.h"

namespace Php {
REGISTER_DUCHAIN_ITEM(NamespaceDeclaration);

NamespaceDeclaration::NamespaceDeclaration(const NamespaceDeclaration& rhs)
        : KDevelop::Declaration(*new NamespaceDeclarationData(*rhs.d_func()))
{
}

NamespaceDeclaration::NamespaceDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::Declaration(*new NamespaceDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context) {
        setContext(context);
    }
}

NamespaceDeclaration::NamespaceDeclaration(NamespaceDeclarationData& data)
        : KDevelop::Declaration(data)
{
}

NamespaceDeclaration::~NamespaceDeclaration()
{
}

KDevelop::Declaration* NamespaceDeclaration::clonePrivate() const
{
    return new NamespaceDeclaration(*this);
}

KDevelop::IndexedString NamespaceDeclaration::prettyName() const
{
    return d_func()->prettyName;
}

void NamespaceDeclaration::setPrettyName( const KDevelop::IndexedString& name )
{
    bool wasInSymbolTable = d_func()->m_inSymbolTable;
    setInSymbolTable(false);
    d_func_dynamic()->prettyName = name;
    setInSymbolTable(wasInSymbolTable);
}

QString NamespaceDeclaration::toString() const
{
  QString ret(QStringLiteral("namespace "));
  return ret + prettyName().str();
}

}
