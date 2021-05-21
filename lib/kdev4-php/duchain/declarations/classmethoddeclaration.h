/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLASSMETHODDECLARATION_H
#define CLASSMETHODDECLARATION_H

#include <language/duchain/classfunctiondeclaration.h>

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>
#include <language/duchain/indexedstring.h>

#include "phpduchainexport.h"

namespace Php
{

class KDEVPHPDUCHAIN_EXPORT ClassMethodDeclarationData : public KDevelop::ClassFunctionDeclarationData
{
public:
    ClassMethodDeclarationData()
            : KDevelop::ClassFunctionDeclarationData() {}

    ClassMethodDeclarationData(const ClassMethodDeclarationData& rhs)
            : KDevelop::ClassFunctionDeclarationData(rhs)
    {
        prettyName = rhs.prettyName;
    }

    ~ClassMethodDeclarationData() {}

    KDevelop::IndexedString prettyName;
};

/**
 * inherits ClassFunctionDeclaration to overwrite some stuff for PHP specific behaviour
 */
class KDEVPHPDUCHAIN_EXPORT ClassMethodDeclaration : public KDevelop::ClassFunctionDeclaration
{
public:
    ClassMethodDeclaration(const ClassMethodDeclaration &rhs);
    ClassMethodDeclaration(const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    ClassMethodDeclaration(ClassMethodDeclarationData &data);
    ClassMethodDeclaration(ClassMethodDeclarationData &data, const KDevelop::RangeInRevision &range, KDevelop::DUContext *context);
    ~ClassMethodDeclaration();

    /// overwritten to check for __construct() method
    virtual bool isConstructor() const;
    /// overwritten to check for __destruct() method
    virtual bool isDestructor() const;

    virtual QString toString() const;

    void setPrettyName(const KDevelop::IndexedString& name);
    KDevelop::IndexedString prettyName() const;

    enum {
        Identity = 84
    };

    virtual KDevelop::Declaration* clonePrivate() const;
private:
    DUCHAIN_DECLARE_DATA(ClassMethodDeclaration)
};

}

#endif // CLASSMETHODDECLARATION_H

