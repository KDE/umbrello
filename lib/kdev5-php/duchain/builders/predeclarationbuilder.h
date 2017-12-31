/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Milian Wolff <mail@milianw.de>                         *
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

