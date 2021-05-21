/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHP_CLASSDECLARATION_H
#define PHP_CLASSDECLARATION_H

#include <language/duchain/classdeclaration.h>

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/indexedstring.h>

#include "phpduchainexport.h"
namespace Php
{

class KDEVPHPDUCHAIN_EXPORT ClassDeclarationData : public KDevelop::ClassDeclarationData
{
public:
    ClassDeclarationData()
            : KDevelop::ClassDeclarationData() {}

    ClassDeclarationData(const ClassDeclarationData& rhs)
            : KDevelop::ClassDeclarationData(rhs)
    {
        prettyName = rhs.prettyName;
    }

    ~ClassDeclarationData() {}

    KDevelop::IndexedString prettyName;
};

class KDEVPHPDUCHAIN_EXPORT ClassDeclaration : public KDevelop::ClassDeclaration
{
public:
    ClassDeclaration(const ClassDeclaration &rhs);
    ClassDeclaration(const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    ClassDeclaration(ClassDeclarationData &data);
    ClassDeclaration(ClassDeclarationData &data, const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    ~ClassDeclaration();

    virtual QString toString() const;

    void setPrettyName(const KDevelop::IndexedString& name);
    KDevelop::IndexedString prettyName() const;

    virtual void setInSymbolTable(bool inSymbolTable);

    enum {
        Identity = 85
    };

    virtual KDevelop::Declaration* clonePrivate() const;
private:
    DUCHAIN_DECLARE_DATA(ClassDeclaration)
};

}

#endif // PHP_CLASSDECLARATION_H
