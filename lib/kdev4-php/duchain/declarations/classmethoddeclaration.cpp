/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
