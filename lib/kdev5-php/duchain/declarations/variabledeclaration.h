/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef VARIABLEDECLARATION_H
#define VARIABLEDECLARATION_H

#include <language/duchain/declaration.h>
#include <language/duchain/declarationdata.h>

#include "phpduchainexport.h"

namespace Php
{

class KDEVPHPDUCHAIN_EXPORT VariableDeclarationData : public KDevelop::DeclarationData
{
public:
    VariableDeclarationData()
            : KDevelop::DeclarationData(), m_isSuperglobal(false) {
    }

    VariableDeclarationData(const VariableDeclarationData& rhs)
            : KDevelop::DeclarationData(rhs) {
        m_isSuperglobal = rhs.m_isSuperglobal;
    }

    ~VariableDeclarationData() {
    }

bool m_isSuperglobal;
};

/**
 * Declaration used for Php variables eg. $a = 0;
 */
class KDEVPHPDUCHAIN_EXPORT VariableDeclaration : public KDevelop::Declaration
{
public:
    VariableDeclaration(const VariableDeclaration& rhs);
    VariableDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context);
    VariableDeclaration(VariableDeclarationData& data);
    VariableDeclaration(VariableDeclarationData& data, const KDevelop::RangeInRevision&);
    virtual ~VariableDeclaration();

    bool isSuperglobal() const;
    void setSuperglobal(bool superglobal);

    virtual uint additionalIdentity() const;
    virtual KDevelop::DeclarationId id(bool forceDirect = false) const;

    enum {
        Identity = 83
    };
    typedef KDevelop::Declaration Base;

private:
    DUCHAIN_DECLARE_DATA(VariableDeclaration)
};

}

#endif // VARIABLEDECLARATION_H
