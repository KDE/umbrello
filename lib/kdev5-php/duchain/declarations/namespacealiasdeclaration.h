/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
