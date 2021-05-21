/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PREDECLARATIONBUILDER_H
#define PREDECLARATIONBUILDER_H

#include "contextbuilder.h"
#include "helper.h"
#include <language/duchain/builders/abstractdeclarationbuilder.h>

namespace KDvelop
{
class Declaration;
}
namespace Php
{
class ParseSession;
class EditorIntegrator;
class FunctionDeclaration;
class ClassDeclaration;
class NamespaceDeclaration;

typedef KDevelop::AbstractDeclarationBuilder<AstNode, IdentifierAst, ContextBuilder> PreDeclarationBuilderBase;

/**
 * The PreDeclarationBuilder builds usable declarations for classes, interfaces and functions.
 *
 * \todo constants should probably be handled here as well
 */
class KDEVPHPDUCHAIN_EXPORT PreDeclarationBuilder : public PreDeclarationBuilderBase
{
public:
    PreDeclarationBuilder(QHash<qint64, ClassDeclaration*>* types,
                          QHash<qint64, FunctionDeclaration*>* functions,
                          QHash<qint64, NamespaceDeclaration*>* namespaces,
                          QList<KDevelop::QualifiedIdentifier>* upcomingClassVariables,
                          EditorIntegrator* editor )
            : m_types(types), m_functions(functions), m_namespaces(namespaces),
              m_upcomingClassVariables(upcomingClassVariables)
    {
        m_editor = editor;
    }

    virtual ~PreDeclarationBuilder();

    /// make it accessible to the declaration builder
    bool didRecompile() { return recompiling(); }

protected:
//     virtual void visitNode(AstNode* node);
    virtual void visitClassDeclarationStatement(ClassDeclarationStatementAst *node);
    virtual void visitInterfaceDeclarationStatement(InterfaceDeclarationStatementAst *node);
    virtual void visitTraitDeclarationStatement(TraitDeclarationStatementAst *node);
    virtual void visitFunctionDeclarationStatement(FunctionDeclarationStatementAst *node);
    virtual void visitClassVariable(ClassVariableAst* node);
    virtual void openNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier, const KDevelop::RangeInRevision& range);
    virtual void closeNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier);

    virtual void closeDeclaration();
    virtual void closeContext();
private:
    QHash<qint64, ClassDeclaration*>* m_types;
    QHash<qint64, FunctionDeclaration*>* m_functions;
    QHash<qint64, NamespaceDeclaration*>* m_namespaces;
    QList<KDevelop::QualifiedIdentifier>* m_upcomingClassVariables;
};

}

#endif // PREDECLARATIONBUILDER_H

