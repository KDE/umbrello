/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Niko Sams <niko.sams@gmail.com>                        *
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
