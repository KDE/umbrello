/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef EXPRESSIONVISITOR_H
#define EXPRESSIONVISITOR_H

#include "phpdefaultvisitor.h"
#include "phpduchainexport.h"
#include "expressionevaluationresult.h"
#include "helper.h"

#include <language/duchain/types/abstracttype.h>
#include <language/duchain/identifier.h>

namespace KDevelop
{
class TopDUContext;
class Declaration;
}

namespace Php
{
class EditorIntegrator;

class KDEVPHPDUCHAIN_EXPORT ExpressionVisitor : public DefaultVisitor
{
public:
    ExpressionVisitor(EditorIntegrator* editor);
    ExpressionEvaluationResult result() {
        return m_result;
    }
    void setCreateProblems(bool v);
    void setOffset(const KDevelop::CursorInRevision& offset);

    virtual void visitNode(AstNode *node);

protected:
    KDevelop::DeclarationPointer processVariable( VariableIdentifierAst* variable);

    void visitAssignmentExpression(AssignmentExpressionAst *node);
    virtual void visitArrayIndexSpecifier(ArrayIndexSpecifierAst* node);
    void visitCompoundVariableWithSimpleIndirectReference(CompoundVariableWithSimpleIndirectReferenceAst *node);
    void visitVarExpressionNewObject(VarExpressionNewObjectAst *node);
    virtual void visitVarExpressionArray(VarExpressionArrayAst *node);
    virtual void visitClosure(ClosureAst* node);
    void visitFunctionCall(FunctionCallAst* node);
    void visitConstantOrClassConst(ConstantOrClassConstAst *node);
    void visitScalar(ScalarAst *node);
    void visitStaticScalar(StaticScalarAst *node);
    void visitEncapsVar(EncapsVarAst *node);
    void visitVariableProperty(VariablePropertyAst *node);
    void visitStaticMember(StaticMemberAst* node);
    void visitUnaryExpression(UnaryExpressionAst* node);
    void visitAdditiveExpressionRest(AdditiveExpressionRestAst* node);
    void visitVariable(VariableAst* node);
    void visitFunctionCallParameterList( FunctionCallParameterListAst* node );
    void visitFunctionCallParameterListElement(FunctionCallParameterListElementAst* node);
    void visitRelationalExpression(RelationalExpressionAst* node);

    QString stringForNode(AstNode* id);
    KDevelop::QualifiedIdentifier identifierForNode(IdentifierAst* id);
    QString stringForNode(VariableIdentifierAst* id);
    KDevelop::QualifiedIdentifier identifierForNode(VariableIdentifierAst* id);


    virtual void usingDeclaration(AstNode* node, const KDevelop::DeclarationPointer& decl) {
        Q_UNUSED(node) Q_UNUSED(decl)
    }

    KDevelop::DeclarationPointer findDeclarationImport(DeclarationType declarationType, IdentifierAst* node);
    KDevelop::DeclarationPointer findDeclarationImport(DeclarationType declarationType, VariableIdentifierAst* node);
    KDevelop::DeclarationPointer findDeclarationImport(DeclarationType declarationType,
                                                       const KDevelop::QualifiedIdentifier& identifier);
    KDevelop::Declaration* findVariableDeclaration(KDevelop::DUContext* context, KDevelop::Identifier identifier,
                                                   KDevelop::CursorInRevision position, KDevelop::DUContext::SearchFlag flag);
protected:
    EditorIntegrator* m_editor;

private:
    KDevelop::DUContext* findClassContext(NamespacedIdentifierAst* className);
    KDevelop::DUContext* findClassContext(IdentifierAst* className);
    void buildNamespaceUses(NamespacedIdentifierAst* namespaces, const KDevelop::QualifiedIdentifier& identifier);

    bool m_createProblems;
    KDevelop::CursorInRevision m_offset;
    KDevelop::DUContext* m_currentContext;

    ExpressionEvaluationResult m_result;

    bool m_isAssignmentExpressionEqual;
    bool m_inDefine;
};

}
#endif
