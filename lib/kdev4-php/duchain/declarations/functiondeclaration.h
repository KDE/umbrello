/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHP_FUNCTIONDECLARATION_H
#define PHP_FUNCTIONDECLARATION_H

#include <language/duchain/functiondeclaration.h>

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/indexedstring.h>

#include "phpduchainexport.h"
namespace Php
{

class KDEVPHPDUCHAIN_EXPORT FunctionDeclarationData : public KDevelop::FunctionDeclarationData
{
public:
    FunctionDeclarationData()
            : KDevelop::FunctionDeclarationData() {}

    FunctionDeclarationData(const FunctionDeclarationData& rhs)
            : KDevelop::FunctionDeclarationData(rhs)
    {
        prettyName = rhs.prettyName;
    }

    ~FunctionDeclarationData() {}

    KDevelop::IndexedString prettyName;
};

class KDEVPHPDUCHAIN_EXPORT FunctionDeclaration : public KDevelop::FunctionDeclaration
{
public:
    FunctionDeclaration(const FunctionDeclaration &rhs);
    FunctionDeclaration(const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    FunctionDeclaration(FunctionDeclarationData &data);
    FunctionDeclaration(FunctionDeclarationData &data, const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    ~FunctionDeclaration();

    virtual QString toString() const;

    void setPrettyName(const KDevelop::IndexedString& name);
    KDevelop::IndexedString prettyName() const;

    enum {
        Identity = 86
    };

    virtual KDevelop::Declaration* clonePrivate() const;
private:
    DUCHAIN_DECLARE_DATA(FunctionDeclaration)
};

}

#endif // PHP_FUNCTIONDECLARATION_H
