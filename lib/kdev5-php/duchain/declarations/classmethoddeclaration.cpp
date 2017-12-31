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

#include "../duchaindebug.h"

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
    static const auto constructId = IndexedIdentifier(Identifier(QStringLiteral("__construct")));
    const auto indexed = indexedIdentifier();
    return indexed == constructId
           || indexed == context()->indexedLocalScopeIdentifier().identifier().indexedFirst();
}

bool ClassMethodDeclaration::isDestructor() const
{
    //TODO: register_shutdown_function
    static const auto destructId = IndexedIdentifier(Identifier(QStringLiteral("__destruct")));
    const auto indexed = indexedIdentifier();
    return indexed == destructId;
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
        return QStringLiteral("%1 %2 %3").arg(function->partToString( FunctionType::SignatureReturn ),
                                       prettyName().str(),
                                       function->partToString( FunctionType::SignatureArguments ));
    } else {
        QString type = abstractType() ? abstractType()->toString() : QStringLiteral("<notype>");
        qCDebug(DUCHAIN) << "A function has a bad type attached:" << type;
        return QStringLiteral("invalid member-function %1 type %2").arg(prettyName().str(),type);
    }
}


}
