/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHP_NAMESPACEDECLARATION_H
#define PHP_NAMESPACEDECLARATION_H

#include <language/duchain/declaration.h>

#include <language/duchain/declarationdata.h>
#include <serialization/indexedstring.h>

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
