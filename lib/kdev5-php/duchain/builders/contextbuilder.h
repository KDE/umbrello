/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Niko Sams <niko.sams@gmail.com>                        *
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

#ifndef CONTEXTBUILDER_H
#define CONTEXTBUILDER_H

#include <language/duchain/builders/abstractcontextbuilder.h>
#include <language/duchain/problem.h>

#include "phpdefaultvisitor.h"
#include "phpduchainexport.h"
#include "editorintegrator.h"
#include "helper.h"

namespace Php
{
class EditorIntegrator;
class ParseSession;

typedef KDevelop::AbstractContextBuilder<AstNode, IdentifierAst> ContextBuilderBase;

/// first is the "pretty" identifier used for printing
/// second comes the all-lowercase identifier used for storage
typedef QPair<KDevelop::IndexedString, KDevelop::QualifiedIdentifier> IdentifierPair;

class KDEVPHPDUCHAIN_EXPORT ContextBuilder: public ContextBuilderBase, public DefaultVisitor
{

public:
    ContextBuilder();
    virtual ~ContextBuilder();

    virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url, AstNode* node,
            KDevelop::ReferencedTopDUContext updateContext
            = KDevelop::ReferencedTopDUContext());

    bool hadUnresolvedIdentifiers() const;

    EditorIntegrator* editor() const;

protected:
    virtual KDevelop::DUContext* newContext(const KDevelop::RangeInRevision& range);
    virtual KDevelop::TopDUContext* newTopContext(const KDevelop::RangeInRevision& range, KDevelop::ParsingEnvironmentFile* file = 0);

    virtual void startVisiting(AstNode* node);
    virtual void setContextOnNode(AstNode* node, KDevelop::DUContext* ctx);
    virtual KDevelop::DUContext* contextFromNode(AstNode* node);
    virtual KDevelop::RangeInRevision editorFindRange(AstNode* fromRange, AstNode* toRange = 0);
    /// Find Cursor for start of a node, useful to limit findLocalDeclarations() searches.
    KDevelop::CursorInRevision startPos( AstNode* node);

    virtual KDevelop::QualifiedIdentifier identifierForNode(IdentifierAst* id);
    KDevelop::QualifiedIdentifier identifierForNode(VariableIdentifierAst* id);
    IdentifierPair identifierPairForNode(IdentifierAst* id);
    QString stringForNode(IdentifierAst* node) const;
    QString stringForNode(VariableIdentifierAst* node) const;

    virtual void visitClassDeclarationStatement(ClassDeclarationStatementAst*);
    virtual void visitInterfaceDeclarationStatement(InterfaceDeclarationStatementAst* node);
    virtual void visitTraitDeclarationStatement(TraitDeclarationStatementAst* node);
    virtual void visitClassStatement(ClassStatementAst *node);
    virtual void visitFunctionDeclarationStatement(FunctionDeclarationStatementAst* node);
    virtual void visitClosure(ClosureAst* node);
    virtual void visitUnaryExpression(UnaryExpressionAst* node);
    /**
     * don't overload in other builders, use @c openNamespace and @c closeNamespace instead.
     */
    virtual void visitNamespaceDeclarationStatement(NamespaceDeclarationStatementAst* node);
    virtual void openNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier, const KDevelop::RangeInRevision& range);
    virtual void closeNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier);

    virtual void addBaseType(NamespacedIdentifierAst * identifier);

    virtual void classContextOpened(KDevelop::DUContext* context);

    /// Report @p errorMsg with the range of @p node
    /// @see void reportError(const QString& errorMsg, KDevelop::SimpleRange range);
    void reportError(const QString& errorMsg, AstNode* node,
                        KDevelop::IProblem::Severity severity = KDevelop::IProblem::Error);
    /// Report @p errorMsg with the range encompassing all nodes in @p nodes
    /// @see void reportError(const QString& errorMsg, KDevelop::SimpleRange range);
    void reportError(const QString& errorMsg, QList<AstNode*> nodes,
                        KDevelop::IProblem::Severity severity = KDevelop::IProblem::Error);
    /// Report @p errorMsg with range @p range
    void reportError(const QString& errorMsg, KDevelop::RangeInRevision range,
                        KDevelop::IProblem::Severity severity = KDevelop::IProblem::Error);

    KDevelop::DeclarationPointer findDeclarationImport(DeclarationType declarationType, IdentifierAst* node);
    KDevelop::DeclarationPointer findDeclarationImport(DeclarationType declarationType, VariableIdentifierAst* node);
    KDevelop::DeclarationPointer findDeclarationImport(DeclarationType declarationType,
                                                       const KDevelop::QualifiedIdentifier &identifier);

    /// internal functions file should not be checked for errors and can get some optimizations
    bool m_isInternalFunctions;
    /// Whether semantic problems should get reported
    bool m_reportErrors;
    ///TODO: push this into kdevplatform
    bool m_mapAst;
    bool m_hadUnresolvedIdentifiers;

    EditorIntegrator* m_editor;

private:
    void closeNamespaces(NamespaceDeclarationStatementAst* namespaces);
    NamespaceDeclarationStatementAst* m_openNamespaces;

};

}

#endif
