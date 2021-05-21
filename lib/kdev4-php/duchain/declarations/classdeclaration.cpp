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
    bool wasInSymbolTable = d_func()->m_inSymbolTable;
    setInSymbolTable(false);
    d_func_dynamic()->prettyName = name;
    setInSymbolTable(wasInSymbolTable);
}

QString ClassDeclaration::toString() const
{
  QString ret;
  switch ( classModifier() ) {
    case ClassDeclarationData::None:
      //nothing
      break;
    case ClassDeclarationData::Abstract:
      ret += "abstract ";
      break;
    case ClassDeclarationData::Final:
      ret += "final ";
      break;
  }
  switch ( classType() ) {
    case ClassDeclarationData::Class:
      ret += "class ";
      break;
    case ClassDeclarationData::Interface:
      ret += "interface ";
      break;
    case ClassDeclarationData::Trait:
      ret += "trait ";
      break;
    case ClassDeclarationData::Union:
      ret += "union ";
      break;
    case ClassDeclarationData::Struct:
      ret += "struct ";
      break;
  }
  return ret + prettyName().str();
}

void ClassDeclaration::setInSymbolTable(bool inSymbolTable)
{
    if(!d_func()->prettyName.isEmpty()) {
        if(!d_func()->m_inSymbolTable && inSymbolTable) {
            CompletionCodeModelItem::Kind flags = CompletionCodeModelItem::Unknown;
            static const KDevelop::QualifiedIdentifier exceptionQId("exception");
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
        } else if(d_func()->m_inSymbolTable && !inSymbolTable) {
            CompletionCodeModel::self().removeItem(url(), qualifiedIdentifier());
        }
    }
    KDevelop::ClassDeclaration::setInSymbolTable(inSymbolTable);
}

}
