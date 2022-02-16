/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
  QString ret("namespace ");
  return ret + prettyName().str();
}

}
