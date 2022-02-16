/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
