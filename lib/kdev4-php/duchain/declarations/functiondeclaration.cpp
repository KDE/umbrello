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

#include "functiondeclaration.h"

#include <language/duchain/duchainregister.h>
#include <language/duchain/types/functiontype.h>

namespace Php {
REGISTER_DUCHAIN_ITEM(FunctionDeclaration);

FunctionDeclaration::FunctionDeclaration(const FunctionDeclaration& rhs)
        : KDevelop::FunctionDeclaration(*new FunctionDeclarationData(*rhs.d_func()))
{
}

FunctionDeclaration::FunctionDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::FunctionDeclaration(*new FunctionDeclarationData, range)
{
    d_func_dynamic()->setClassId(this);
    if (context) {
        setContext(context);
    }
}

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data)
        : KDevelop::FunctionDeclaration(data)
{
}

FunctionDeclaration::FunctionDeclaration(FunctionDeclarationData& data, const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::FunctionDeclaration(data, range)
{
    if (context) {
        setContext(context);
    }
}

FunctionDeclaration::~FunctionDeclaration()
{
}

KDevelop::Declaration* FunctionDeclaration::clonePrivate() const
{
    return new FunctionDeclaration(*this);
}

KDevelop::IndexedString FunctionDeclaration::prettyName() const
{
    return d_func()->prettyName;
}

void FunctionDeclaration::setPrettyName( const KDevelop::IndexedString& name )
{
    d_func_dynamic()->prettyName = name;
}

QString FunctionDeclaration::toString() const
{
  if( !abstractType() )
    return Declaration::toString();

  TypePtr<KDevelop::FunctionType> function = type<KDevelop::FunctionType>();
  Q_ASSERT(function);

  return QString("%1 %2 %3").arg(function->partToString( KDevelop::FunctionType::SignatureReturn ))
                            .arg(prettyName().str())
                            .arg(function->partToString( KDevelop::FunctionType::SignatureArguments ));
}

}
