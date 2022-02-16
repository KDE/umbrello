/*
    KDevelop Php Code Completion Support

    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "item.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <language/codecompletion/codecompletionmodel.h>
#include <language/codecompletion/codecompletionhelper.h>
#include <language/duchain/declaration.h>
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/types/functiontype.h>
#include "declarations/classdeclaration.h"
#include "declarations/classmethoddeclaration.h"
#include "types/structuretype.h"

#include "completion/helpers.h"
#include "completion/context.h"

#include "../duchain/navigation/navigationwidget.h"
#include "../duchain/declarations/variabledeclaration.h"
#include "../duchain/helper.h"

using namespace KDevelop;

namespace Php
{

QString NormalDeclarationCompletionItem::declarationName() const
{
    QString ret = prettyName(m_declaration.data());
    if ( ret.isEmpty() ) {
        return "<unknown>";
    }
    bool isStatic = false;
    if (!m_declaration->isFunctionDeclaration()) {
        if (dynamic_cast<VariableDeclaration*>(m_declaration.data())) {
            ret = '$' + ret;
        } else if (ClassMemberDeclaration* memberDec = dynamic_cast<ClassMemberDeclaration*>(m_declaration.data())) {
            isStatic = memberDec->isStatic();
            if (memberDec->isStatic() && memberDec->abstractType() && ! memberDec->abstractType()->modifiers() & AbstractType::ConstModifier) {
                // PHP is strange, $obj->asdf, class::const but class::$static ...
                ret = '$' + ret;
            }
        }
    } else if ( ClassFunctionDeclaration* funDec = dynamic_cast<ClassFunctionDeclaration*>(m_declaration.data()) ) {
        isStatic = funDec->isStatic();
    }

    const KSharedPtr<CodeCompletionContext>& ctx = completionContext();

    if ( ctx->memberAccessOperation() == CodeCompletionContext::NoMemberAccess ) {
        // if we complete a class member or method (inside a method)
        // we might have to add "self::", "parent::" or "$this->"
        if ( ctx->duContext() && ctx->duContext()->parentContext()
                && ctx->duContext()->parentContext()->type() == DUContext::Class )
        {
            if ( m_declaration->context() && m_declaration->context()->type() == DUContext::Class ) {
                if ( isStatic ) {
                    ret = "self::" + ret;
                } else {
                    ret = "$this->" + ret;
                }
            }
        }
    }
    return ret;
}

void NormalDeclarationCompletionItem::executed(KTextEditor::Document* document, const KTextEditor::Range& word)
{
    if (m_declaration && dynamic_cast<AbstractFunctionDeclaration*>(m_declaration.data())) {
        //Do some intelligent stuff for functions with the parens:
        insertFunctionParenText(document, word.end(), m_declaration);
    }
}

QVariant NormalDeclarationCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const
{

    DUChainReadLocker lock(DUChain::lock(), 500);
    if (!lock.locked()) {
        kDebug() << "Failed to lock the du-chain in time";
        return QVariant();
    }

    if (!m_declaration) {
        return QVariant();
    }

    Declaration* dec = const_cast<Declaration*>(m_declaration.data());

    switch (role) {
    case CodeCompletionModel::ItemSelected:
        return QVariant(NavigationWidget::shortDescription(dec));
    case Qt::DisplayRole:
        switch (index.column()) {
        case CodeCompletionModel::Postfix:
            return QVariant();
        case CodeCompletionModel::Prefix:
            if (dec->kind() == Declaration::Type && !dec->isTypeAlias()) {
                if (dec->isFunctionDeclaration()) {
                    FunctionType::Ptr funcType = dec->type<FunctionType>();
                    if ( funcType && funcType->returnType() ) {
                        return funcType->returnType()->toString();
                    } else {
                        return "<notype>";
                    }
                } else if (dec->internalContext() && dec->internalContext()->type() == DUContext::Class) {
                    ClassDeclaration* classDec = dynamic_cast<ClassDeclaration*>(dec);
                    if (classDec) {
                        if (classDec->classType() == ClassDeclarationData::Interface) {
                            return "interface";
                        } else {
                            return "class";
                        }
                    }
                }
                return QVariant();
            }  else if (dec->kind() == Declaration::Namespace) {
                return "namespace";
            }
        break;

        case CodeCompletionModel::Arguments:
            if (FunctionType::Ptr functionType = dec->type<FunctionType>()) {
                QString ret;
                createArgumentList(*this, ret, 0);
                return ret;
            }
            break;
        }
        break;
    case CodeCompletionModel::HighlightingMethod:
        if (index.column() == CodeCompletionModel::Arguments) {
            if (completionContext()->memberAccessOperation() == CodeCompletionContext::FunctionCallAccess) {
                return QVariant(CodeCompletionModel::CustomHighlighting);
            } else {
                return QVariant();
            }
            break;
        }

        break;

    case CodeCompletionModel::CustomHighlight:
        if (index.column() == CodeCompletionModel::Arguments && completionContext()->memberAccessOperation() == CodeCompletionContext::FunctionCallAccess) {
            QString ret;
            QList<QVariant> highlight;
            createArgumentList(*this, ret, &highlight);
            return QVariant(highlight);
        }
        break;
    }
    lock.unlock();

    return KDevelop::NormalDeclarationCompletionItem::data(index, role, model);
}

QWidget* NormalDeclarationCompletionItem::createExpandingWidget(const KDevelop::CodeCompletionModel* model) const
{
    return new NavigationWidget(m_declaration, model->currentTopContext());
}

bool NormalDeclarationCompletionItem::createsExpandingWidget() const
{
    return true;
}

KSharedPtr<CodeCompletionContext> NormalDeclarationCompletionItem::completionContext() const
{
    return KSharedPtr<CodeCompletionContext>::staticCast(m_completionContext);
}


}

