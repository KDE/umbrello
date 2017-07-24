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

