/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
