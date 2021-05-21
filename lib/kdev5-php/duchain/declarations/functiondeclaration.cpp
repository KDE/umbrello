/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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

  KDevelop::TypePtr<KDevelop::FunctionType> function = type<KDevelop::FunctionType>();
  Q_ASSERT(function);

  return QStringLiteral("%1 %2 %3").arg(function->partToString( KDevelop::FunctionType::SignatureReturn ),
                                 prettyName().str(),
                                 function->partToString( KDevelop::FunctionType::SignatureArguments ));
}

}
