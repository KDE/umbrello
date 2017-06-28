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

#include "classmethoddeclaration.h"

#include <language/duchain/duchainregister.h>
#include <language/duchain/types/functiontype.h>
using namespace KDevelop;

namespace Php
{
REGISTER_DUCHAIN_ITEM(ClassMethodDeclaration);

ClassMethodDeclaration::ClassMethodDeclaration(const ClassMethodDeclaration& rhs)
        : KDevelop::ClassFunctionDeclaration(*new ClassMethodDeclarationData(*rhs.d_func()))
{
}

ClassMethodDeclaration::ClassMethodDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::ClassFunctionDeclaration(*new ClassMethodDeclarationData, range, context)
{
    d_func_dynamic()->setClassId(this);
    if (context)
        setContext(context);
}

ClassMethodDeclaration::ClassMethodDeclaration(ClassMethodDeclarationData& data)
        : KDevelop::ClassFunctionDeclaration(data)
{
}

ClassMethodDeclaration::ClassMethodDeclaration(ClassMethodDeclarationData& data, const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::ClassFunctionDeclaration(data, range, context)
{
}

ClassMethodDeclaration::~ClassMethodDeclaration()
{
}

bool ClassMethodDeclaration::isConstructor() const
{
    Identifier id = identifier();
    return id.nameEquals(Identifier("__construct"))
           || id.nameEquals(context()->indexedLocalScopeIdentifier().identifier().first());
}

bool ClassMethodDeclaration::isDestructor() const
{
    //TODO: register_shutdown_function
    return identifier().nameEquals(Identifier("__destruct"));
}

Declaration* ClassMethodDeclaration::clonePrivate() const
{
    return new ClassMethodDeclaration(*this);
}

KDevelop::IndexedString ClassMethodDeclaration::prettyName() const
{
    return d_func()->prettyName;
}

void ClassMethodDeclaration::setPrettyName( const KDevelop::IndexedString& name )
{
    d_func_dynamic()->prettyName = name;
}

QString ClassMethodDeclaration::toString() const
{
    if( !abstractType() )
        return ClassMemberDeclaration::toString();

    TypePtr<FunctionType> function = type<FunctionType>();
    if(function) {
        return QString("%1 %2 %3").arg(function->partToString( FunctionType::SignatureReturn ))
                                  .arg(prettyName().str())
                                  .arg(function->partToString( FunctionType::SignatureArguments ));
    } else {
        QString type = abstractType() ? abstractType()->toString() : QString("<notype>");
        kDebug(9505) << "A function has a bad type attached:" << type;
        return QString("invalid member-function %1 type %2").arg(prettyName().str()).arg(type);
    }
}


}
