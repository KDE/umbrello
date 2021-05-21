/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "classdeclaration.h"

#include <language/duchain/duchainregister.h>
#include <completioncodemodel.h>

#include "helper.h"

namespace Php {
REGISTER_DUCHAIN_ITEM(ClassDeclaration);

ClassDeclaration::ClassDeclaration(const ClassDeclaration& rhs)
        : KDevelop::ClassDeclaration(*new ClassDeclarationData(*rhs.d_func()))
{
}

ClassDeclaration::ClassDeclaration(const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::ClassDeclaration(*new ClassDeclarationData, range, context)
{
    d_func_dynamic()->setClassId(this);
    if (context) {
        setContext(context);
    }
}

ClassDeclaration::ClassDeclaration(ClassDeclarationData& data)
        : KDevelop::ClassDeclaration(data)
{
}

ClassDeclaration::ClassDeclaration(ClassDeclarationData& data, const KDevelop::RangeInRevision& range, KDevelop::DUContext* context)
        : KDevelop::ClassDeclaration(data, range, context)
{
}

ClassDeclaration::~ClassDeclaration()
{
}

KDevelop::Declaration* ClassDeclaration::clonePrivate() const
{
    return new ClassDeclaration(*this);
}

KDevelop::IndexedString ClassDeclaration::prettyName() const
{
    return d_func()->prettyName;
}

void ClassDeclaration::setPrettyName( const KDevelop::IndexedString& name )
{
    d_func_dynamic()->prettyName = name;
}

QString ClassDeclaration::toString() const
{
  QString ret;
  switch ( classModifier() ) {
    case ClassDeclarationData::None:
      //nothing
      break;
    case ClassDeclarationData::Abstract:
      ret += QLatin1String("abstract ");
      break;
    case ClassDeclarationData::Final:
      ret += QLatin1String("final ");
      break;
  }
  switch ( classType() ) {
    case ClassDeclarationData::Class:
      ret += QLatin1String("class ");
      break;
    case ClassDeclarationData::Interface:
      ret += QLatin1String("interface ");
      break;
    case ClassDeclarationData::Trait:
      ret += QLatin1String("trait ");
      break;
    case ClassDeclarationData::Union:
      ret += QLatin1String("union ");
      break;
    case ClassDeclarationData::Struct:
      ret += QLatin1String("struct ");
      break;
  }
  return ret + prettyName().str();
}

void ClassDeclaration::updateCompletionCodeModelItem()
{
    if (d_func()->prettyName.isEmpty()) {
        return;
    }
    if (d_func()->m_inSymbolTable) {
        CompletionCodeModelItem::Kind flags = CompletionCodeModelItem::Unknown;
        static const KDevelop::QualifiedIdentifier exceptionQId(QStringLiteral("exception"));
        if (qualifiedIdentifier() == exceptionQId) {
            flags = (CompletionCodeModelItem::Kind)(flags | CompletionCodeModelItem::Exception);
        } else {
            static KDevelop::DUChainPointer<ClassDeclaration> exceptionDecl;
            if (!exceptionDecl) {
                QList<Declaration*> decs = context()->topContext()->findDeclarations(exceptionQId);
                Q_ASSERT(decs.count());
                exceptionDecl = dynamic_cast<ClassDeclaration*>(decs.first());
                Q_ASSERT(exceptionDecl);
            }
            if (equalQualifiedIdentifier(exceptionDecl.data())
                || isPublicBaseClass(exceptionDecl.data(), context()->topContext())
            ) {
                flags = (CompletionCodeModelItem::Kind)(flags | CompletionCodeModelItem::Exception);
            }
        }
        CompletionCodeModel::self().addItem(url(), qualifiedIdentifier(), d_func_dynamic()->prettyName, flags);
    } else {
        CompletionCodeModel::self().removeItem(url(), qualifiedIdentifier());
    }
}

}
