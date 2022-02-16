/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DECLARATIONBUILDER_H
#define DECLARATIONBUILDER_H

#include "typebuilder.h"
#include "helper.h"
#include <language/duchain/classdeclaration.h>
#include <language/duchain/builders/abstractdeclarationbuilder.h>

namespace KDvelop
{
class Declaration;
}
namespace Php
{
class ParseSession;
class EditorIntegrator;
class ClassDeclaration;
class FunctionDeclaration;
class NamespaceDeclaration;

typedef KDevelop::AbstractDeclarationBuilder<AstNode, IdentifierAst, TypeBuilder> DeclarationBuilderBase;

/**
 * The DeclarationBuilder builds declarations, types and contexts for everything in an AST.
 *
 * \note Since PHP allows the usage of functions, classes and interfaces before definition,
 *       a \see PreDeclarationBuilder is used to get the declarations _and_ types for those.
 *       Thus type- and declaratoinbuilding for these is skipped in this class.
 */
class KDEVPHPDUCHAIN_EXPORT DeclarationBuilder : public DeclarationBuilderBase
{
public:
    DeclarationBuilder(EditorIntegrator* editor)
        : m_currentModifers(0)
    {
        m_editor = editor;
        m_findVariable.find = false;
    }
    virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url, AstNode* node,
            KDevelop::ReferencedTopDUContext updateContext
            = KDevelop::ReferencedTopDUContext());

    virtual void startVisiting(AstNode* node);

protected:
    virtual void visitClassDeclarationStatement(ClassDeclarationStatementAst *node);
    virtual void visitInterfaceDeclarationStatement(InterfaceDeclarationStatementAst *node);
    virtual void visitTraitDeclarationStatement(TraitDeclarationStatementAst *node);
    virtual void visitClassStatement(ClassStatementAst *node);
    virtual void importTraitMethods(ClassStatementAst *node);
    virtual void visitClassExtends(ClassExtendsAst *node);
    virtual void visitClassImplements(ClassImplementsAst *node);
    virtual void visitParameter(ParameterAst *node);
    virtual void visitFunctionDeclarationStatement(FunctionDeclarationStatementAst *node);
    virtual void visitClassVariable(ClassVariableAst *node);
    virtual void visitConstantDeclaration(ConstantDeclarationAst *node);
    virtual void visitTraitAliasStatement(TraitAliasStatementAst *node);
    virtual void createTraitAliasDeclarations(TraitAliasStatementAst *node, KDevelop::DeclarationPointer dec);
    virtual void visitOuterTopStatement(OuterTopStatementAst* node);
    virtual void visitAssignmentExpression(AssignmentExpressionAst* node);
    virtual void visitAssignmentExpressionEqual(AssignmentExpressionEqualAst *node);
    virtual void visitVariable(VariableAst* node);
    virtual void visitFunctionCall(FunctionCallAst* node);
    virtual void visitFunctionCallParameterList(FunctionCallParameterListAst* node);
    virtual void visitFunctionCallParameterListElement(FunctionCallParameterListElementAst* node);
    virtual void visitStatement(StatementAst* node);
    virtual void visitStaticVar(StaticVarAst* node);
    virtual void visitGlobalVar(GlobalVarAst* node);
    virtual void visitCatchItem(CatchItemAst *node);
    virtual void visitUnaryExpression( UnaryExpressionAst* node );
    virtual void visitAssignmentListElement(AssignmentListElementAst* node);
    virtual void openNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier, const KDevelop::RangeInRevision& range);
    virtual void closeNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier);
    virtual void visitUseNamespace(UseNamespaceAst* node);
    virtual void visitClosure(ClosureAst* node);
    virtual void visitLexicalVar(LexicalVarAst* node);

    /// checks whether the body is empty (i.e. equals ";" instead of "{...}")
    bool isEmptyMethodBody(const MethodBodyAst* body) const {
        return !body || !body->statements;
    }

    virtual void closeDeclaration();
    void classContextOpened(KDevelop::DUContext* context);

    virtual void supportBuild(AstNode* node, KDevelop::DUContext* context = 0);
    virtual void closeContext();

    /// don't forget to closeDeclaration() afterwards
    /// set m_currentModifers to your likings and reset it afterwards
    void openClassMemberDeclaration(AstNode* node, const KDevelop::QualifiedIdentifier& name);

    virtual void updateCurrentType();

private:
    /// because the predeclarationbuilder runs before us,
    /// we always "think" that we are recompiling, while this is not necessarily true
    bool m_actuallyRecompiling;

    struct FindVariableResults {
        /// Set this to true if you want to catch any variable in the lower AST tree
        bool find;
        /// If the found variable is accessed as an array ($var[...]) this is set to true.
        /// @see m_findVariable
        bool isArray;
        /// The identifier for the found variable.
        /// @see m_findVariable
        KDevelop::QualifiedIdentifier identifier;
        /// The identifier for the parent of the found variable. Empty if
        /// the found variable is not a class member.
        /// @see m_findVariable
        KDevelop::QualifiedIdentifier parentIdentifier;
        /// The AstNode of the found variable. Use this for declarations.
        /// @see m_findVariable
        AstNode* node;

        FindVariableResults();
    };
    FindVariableResults m_findVariable;

    /// The position of the current argument, will only be set inside function calls.
    int m_functionCallParameterPos;
    /// Type of the current function, will only be set inside function calls.
    KDevelop::FunctionType::Ptr m_currentFunctionType;

    unsigned int m_currentModifers;
    QString m_lastTopStatementComment;

    QHash<qint64, ClassDeclaration*> m_types;
    QHash<qint64, FunctionDeclaration*> m_functions;
    QHash<qint64, NamespaceDeclaration*> m_namespaces;
    QList<KDevelop::QualifiedIdentifier> m_upcomingClassVariables;

    /// handles common stuff for both interfaces and classes
    ClassDeclaration* openTypeDeclaration(IdentifierAst *name, KDevelop::ClassDeclarationData::ClassType type);

    /// check if this declaration is already declared
    bool isGlobalRedeclaration(const KDevelop::QualifiedIdentifier &identifier, AstNode *node,
                               DeclarationType type);
    /// check if a non-abstract method declaration tries to overwrite a final base method
    /// or whether an abstract method is redeclared
    /// @param ids      The identifier for the current method
    /// @param curClass the current class we are in
    /// @param node     the node we are processing, used to access modifiers and for error reporting
    bool isBaseMethodRedeclaration(const IdentifierPair &ids, ClassDeclaration *curClass,
                                   ClassStatementAst *node);
    /// reports a redeclaration error for the given node
    /// @param declaration the old declaration
    /// @param node        the AstNode which resembles the redeclaration
    void reportRedeclarationError(KDevelop::Declaration* declaration, AstNode *node);

    /**
     * Get the interesting identifiers out of a VariableAst node:
     * $var yields @p id = 'var', @p parent = ''
     * $var->asdf yields @p id = 'asdf', @p parent = 'asdf'
     * $var->...->foo->bar yields @p id = 'bar', @p parent => 'foo'
     *
     * @note If the parent or the identifier itself end on an array access, e.g. $var[0] or
     *       $var->...->parent[0]->bar, @p arrayAccess will be set to true.
     *
     * @param id the last identifier
     * @param parent the parent of the last identifier
     * @param targetNode the node of the last identifier
     * @param arrayAccess the node actually ends on an array access, like $node->var->..->asdf[0]
     */
    void getVariableIdentifier(VariableAst *node,
                                    KDevelop::QualifiedIdentifier &id,
                                    KDevelop::QualifiedIdentifier &parent,
                                    AstNode* &targetNode,
                                    bool &arrayAccess);

    /**
     * Declare a class member in @p parentCtx. Validates whether the current context allows
     * redeclaration of private/protected members.
     *
     * @param parentCtx  The class context you want to add the member to.
     * @param type       The type of the member.
     * @param identifier The identifier of the member.
     * @param node       The node of the member.
     */
    void declareClassMember(KDevelop::DUContext *parentCtx, KDevelop::AbstractType::Ptr type,
                            const KDevelop::QualifiedIdentifier& identifier, AstNode* node );

    /**
     * Declare a variable in @p parentCtx. If the variable is already defined in the
     * context and it's last type equals @p type, don't do anything.
     *
     * @param parentCtx  The context you want to declare the variable in.
     * @param type       The type of the variable
     * @param identifier The identifier for the variable.
     * @param node       The node for the variable.
     */
    void declareVariable(KDevelop::DUContext *parentCtx, KDevelop::AbstractType::Ptr type,
                            const KDevelop::QualifiedIdentifier& identifier, AstNode* node );

    /**
     * Wrapper that operates declares the found variable. It will declare it
     * either as a class member or as a variable, depending whether a parent was found.
     *
     * It will also check whether that var also exists and if so, won't do anything.
     *
     * @param type When the var gets declared, this will be it's type.
     *
     * @see m_findVariable
     * @see declareClassMeember
     * @see declareVariable
     */
    void declareFoundVariable(KDevelop::AbstractType::Ptr type);

    /**
     * Sets encountered and updates the comment when we are recompiling.
     */
    void encounter(KDevelop::Declaration* dec);
};

}

#endif // DECLARATIONBUILDER_H

