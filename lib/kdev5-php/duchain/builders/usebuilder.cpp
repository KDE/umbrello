/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "usebuilder.h"

#include <KLocalizedString>

#include "editorintegrator.h"
#include "expressionvisitor.h"
#include "parsesession.h"
#include "../duchaindebug.h"

using namespace KDevelop;

namespace Php
{

class UseExpressionVisitor : public ExpressionVisitor
{
public:
    UseExpressionVisitor(EditorIntegrator* editor, UseBuilder* useBuilder)
            : ExpressionVisitor(editor), m_builder(useBuilder) {
    }

protected:
    virtual void usingDeclaration(AstNode* node, const DeclarationPointer& decl) {
        m_builder->newCheckedUse(node, decl);
    }

private:
    UseBuilder* m_builder;
};

UseBuilder::UseBuilder( EditorIntegrator* editor )
{
    m_editor = editor;
}

ReferencedTopDUContext UseBuilder::build ( const IndexedString& url, AstNode* node, ReferencedTopDUContext updateContext )
{
    // just for safety purposes: running the UseBuilder on the internal function file
    // will lead to undefined behavior due to the amount of optimization it has received
    // (esp. in the contextbuilder)
    Q_ASSERT(url != internalFunctionFile());
    return UseBuilderBase::build ( url, node, updateContext );
}

void UseBuilder::visitParameter(ParameterAst *node)
{
    if (node->parameterType) {
        buildNamespaceUses(node->parameterType);
    }
}

void UseBuilder::visitClassImplements(ClassImplementsAst *node)
{
    if (node->implementsSequence) {
        const KDevPG::ListNode<NamespacedIdentifierAst*> *__it = node->implementsSequence->front(), *__end = __it;
        do {
            buildNamespaceUses(__it->element);
            __it = __it->next;
        } while (__it != __end);
    }
}

void UseBuilder::visitClassExtends(ClassExtendsAst *node)
{
    buildNamespaceUses(node->identifier);
}

void UseBuilder::visitClassStatement(ClassStatementAst *node)
{
     if (!node->traitsSequence) {
        UseBuilderBase::visitClassStatement(node);
        return;
     }

    const KDevPG::ListNode< NamespacedIdentifierAst* >* it = node->traitsSequence->front();
    forever {
        buildNamespaceUses(it->element, ClassDeclarationType);

        if ( it->hasNext() ) {
            it = it->next;
        } else {
            break;
        }
    }

    if (node->imports) {
        visitTraitAliasDeclaration(node->imports);
    }

    UseBuilderBase::visitClassStatement(node);
}

void UseBuilder::visitTraitAliasStatement(TraitAliasStatementAst *node)
{
    if (node->conflictIdentifierSequence) {
        const KDevPG::ListNode< NamespacedIdentifierAst* >* it = node->conflictIdentifierSequence->front();
        forever {
            buildNamespaceUses(it->element, ClassDeclarationType);

            if ( it->hasNext() ) {
                it = it->next;
            } else {
                break;
            }
        }
    }

    DUChainWriteLocker lock;
    DeclarationPointer dec = findDeclarationImport(ClassDeclarationType, identifierForNamespace(node->importIdentifier->identifier, m_editor));

    if (dec) {
        QualifiedIdentifier original = identifierPairForNode(node->importIdentifier->methodIdentifier).second;
        QList <Declaration*> list = dec.data()->internalContext()->findLocalDeclarations(original.last(), dec.data()->internalContext()->range().start);

        if (!list.isEmpty()) {
            UseBuilderBase::newUse(node->importIdentifier->methodIdentifier, DeclarationPointer(list.first()));
        }
    }

    lock.unlock();

    visitTraitAliasIdentifier(node->importIdentifier);
}

void UseBuilder::visitTraitAliasIdentifier(TraitAliasIdentifierAst *node)
{
    buildNamespaceUses(node->identifier, ClassDeclarationType);
}

void UseBuilder::visitExpr(ExprAst* node)
{
    visitNodeWithExprVisitor(node);
}

void UseBuilder::visitGlobalVar(GlobalVarAst* node)
{
    if (node->var) {
        DeclarationPointer dec = findDeclarationImport(GlobalVariableDeclarationType, node->var);
        if (dec) {
            newCheckedUse(node->var, dec);
        }
    }
}

void UseBuilder::visitStaticScalar(StaticScalarAst* node)
{
    if (currentContext()->type() == DUContext::Class) {
        visitNodeWithExprVisitor(node);
    }
}

void UseBuilder::visitStatement(StatementAst *node)
{
    if (node->foreachVar) {
        visitNodeWithExprVisitor(node->foreachVar);
    } else if (node->unsetVariablesSequence) {
        visitNodeWithExprVisitor(node);
    }

    if (node->foreachExprAsVar) {
        visitNodeWithExprVisitor(node->foreachExprAsVar);
    }
    if (node->foreachVarAsVar) {
        visitNodeWithExprVisitor(node->foreachVarAsVar);
    }
    if (node->foreachVariable) {
        visitNodeWithExprVisitor(node->foreachVariable);
    }

    UseBuilderBase::visitStatement(node);
}

void UseBuilder::visitCatchItem(CatchItemAst *node)
{
    if (node->catchClass) {
        buildNamespaceUses(node->catchClass, ClassDeclarationType);
    }
    UseBuilderBase::visitCatchItem(node);
}

void UseBuilder::newCheckedUse(AstNode* node, const DeclarationPointer& declaration, bool reportNotFound)
{
    if ( declaration && declaration->comment().contains("@deprecated") ) {
        reportError(i18n("Usage of %1 is deprecated.", declaration->toString()), node, IProblem::Hint);
    } else if ( !declaration && reportNotFound ) {
        reportError(i18n("Declaration not found: %1", m_editor->parseSession()->symbol(node)), node, IProblem::Hint);
    }
    UseBuilderBase::newUse(node, declaration);
}

void UseBuilder::visitUnaryExpression( UnaryExpressionAst* node )
{
    IndexedString includeFile = getIncludeFileForNode(node, m_editor);
    if ( !includeFile.isEmpty() ) {
        QualifiedIdentifier identifier(includeFile.str());

        DUChainWriteLocker lock(DUChain::lock());
        for( Declaration* dec : currentContext()->topContext()->findDeclarations(identifier) ) {
            if ( dec->kind() == Declaration::Import ) {
                newUse(node->includeExpression, DeclarationPointer(dec));
                return;
            }
        }
    }
}

void UseBuilder::visitUseNamespace(UseNamespaceAst* node)
{
    buildNamespaceUses(node->identifier, NamespaceDeclarationType);
}

void UseBuilder::buildNamespaceUses(NamespacedIdentifierAst* node, DeclarationType lastType)
{
    const QualifiedIdentifier identifier = identifierForNamespace(node, m_editor);
    QualifiedIdentifier curId;
    curId.setExplicitlyGlobal(identifier.explicitlyGlobal());
    Q_ASSERT(identifier.count() == node->namespaceNameSequence->count());
    for ( int i = 0; i < identifier.count() - 1; ++i ) {
        curId.push(identifier.at(i));
        AstNode* n = node->namespaceNameSequence->at(i)->element;
        DeclarationPointer dec = findDeclarationImport(NamespaceDeclarationType, curId);
        if (!dec || dec->range() != editorFindRange(n, n)) {
            newCheckedUse(n, dec, true);
        }
    }
    newCheckedUse(node->namespaceNameSequence->back()->element,
                  findDeclarationImport(lastType, identifier),
                  lastType == ClassDeclarationType || lastType == ConstantDeclarationType
                  || lastType == FunctionDeclarationType || lastType == NamespaceDeclarationType);
}

void UseBuilder::openNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node,
                               const IdentifierPair& identifier, const RangeInRevision& range)
{
    if (node != parent->namespaceNameSequence->back()->element) {
        DeclarationPointer dec = findDeclarationImport(NamespaceDeclarationType, identifier.second);
        if (!dec || dec->range() != editorFindRange(node, node)) {
            newCheckedUse(node, dec);
        }
    }
    UseBuilderBase::openNamespace(parent, node, identifier, range);
}

void UseBuilder::visitNodeWithExprVisitor(AstNode* node)
{
    UseExpressionVisitor v(m_editor, this);
    node->ducontext = currentContext();
    v.visitNode(node);

    if (v.result().hadUnresolvedIdentifiers()) {
        m_hadUnresolvedIdentifiers = true;
    }
}

}
