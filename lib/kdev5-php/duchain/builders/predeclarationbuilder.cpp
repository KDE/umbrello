/*
    This file is part of KDevelop
    SPDX-FileCopyrightText: 2008 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "predeclarationbuilder.h"

#include <QByteArray>

#include <language/duchain/stringhelpers.h>
#include <language/duchain/types/functiontype.h>

#include <klocalizedstring.h>

#include "phpast.h"
#include "parsesession.h"
#include "helper.h"
#include "../declarations/classdeclaration.h"
#include "../declarations/functiondeclaration.h"
#include "../declarations/variabledeclaration.h"
#include "../declarations/namespacedeclaration.h"
#include "../types/structuretype.h"
#include "../duchaindebug.h"

using namespace KDevelop;

namespace Php
{

PreDeclarationBuilder::~PreDeclarationBuilder()
{
}

void PreDeclarationBuilder::visitClassDeclarationStatement(ClassDeclarationStatementAst * node)
{
    setComment(formatComment(node, m_editor));
    {
        IdentifierPair ids = identifierPairForNode(node->className);
        DUChainWriteLocker lock(DUChain::lock());
        ClassDeclaration* dec = openDefinition<ClassDeclaration>(ids.second, editorFindRange(node->className, node->className));
        dec->setPrettyName(ids.first);
        dec->setKind(KDevelop::Declaration::Type);
        dec->clearBaseClasses();
        dec->setClassType(ClassDeclarationData::Class);
        if (node->modifier) {
            switch (node->modifier->modifier) {
            case NormalClass:
                dec->setClassModifier(ClassDeclarationData::None);
                break;
            case FinalClass:
                dec->setClassModifier(ClassDeclarationData::Final);
                break;
            case AbstractClass:
                dec->setClassModifier(ClassDeclarationData::Abstract);
                break;
            }
        } else {
            dec->setClassModifier(ClassDeclarationData::None);
        }

        // build the type as well, to make this declaration usable
        StructureType::Ptr type(new StructureType());
        type->setPrettyName(ids.first);
        type->setDeclaration(dec);
        dec->setType(type);

        m_types->insert(node->className->string, dec);
    }

    PreDeclarationBuilderBase::visitClassDeclarationStatement(node);

    closeDeclaration();
}

void PreDeclarationBuilder::visitInterfaceDeclarationStatement(InterfaceDeclarationStatementAst *node)
{
    setComment(formatComment(node, m_editor));
    {
        IdentifierPair ids = identifierPairForNode(node->interfaceName);
        DUChainWriteLocker lock(DUChain::lock());
        ClassDeclaration* dec = openDefinition<ClassDeclaration>(ids.second, editorFindRange(node->interfaceName, node->interfaceName));
        dec->setPrettyName(ids.first);
        dec->setKind(KDevelop::Declaration::Type);
        dec->clearBaseClasses();
        dec->setClassType(ClassDeclarationData::Interface);

        // build the type as well, to make this declaration usable
        StructureType::Ptr type(new StructureType());
        type->setPrettyName(ids.first);
        type->setDeclaration(dec);
        dec->setType(type);

        m_types->insert(node->interfaceName->string, dec);
    }

    PreDeclarationBuilderBase::visitInterfaceDeclarationStatement(node);

    closeDeclaration();
}

void PreDeclarationBuilder::visitTraitDeclarationStatement(TraitDeclarationStatementAst *node)
{
    setComment(formatComment(node, m_editor));
    {
        IdentifierPair ids = identifierPairForNode(node->traitName);
        StructureType::Ptr type(new StructureType());
        type->setPrettyName(ids.first);

        DUChainWriteLocker lock;

        ClassDeclaration* dec = openDefinition<ClassDeclaration>(ids.second, editorFindRange(node->traitName, node->traitName));
        dec->setPrettyName(ids.first);
        dec->setKind(KDevelop::Declaration::Type);
        dec->clearBaseClasses();
        dec->setClassType(ClassDeclarationData::Trait);

        type->setDeclaration(dec);
        dec->setType(type);

        m_types->insert(node->traitName->string, dec);
    }

    PreDeclarationBuilderBase::visitTraitDeclarationStatement(node);

    closeDeclaration();
}

void PreDeclarationBuilder::visitClassVariable(ClassVariableAst* node)
{
    m_upcomingClassVariables->append(identifierForNode(node->variable));
}

void PreDeclarationBuilder::visitFunctionDeclarationStatement(FunctionDeclarationStatementAst* node)
{
    setComment(formatComment(node, m_editor));
    {
        IdentifierPair ids = identifierPairForNode(node->functionName);
        DUChainWriteLocker lock(DUChain::lock());
        FunctionDeclaration *dec = openDefinition<FunctionDeclaration>(ids.second, editorFindRange(node->functionName, node->functionName));
        dec->setPrettyName(ids.first);
        dec->setKind(Declaration::Type);
        dec->clearDefaultParameters();

        FunctionType::Ptr type = FunctionType::Ptr(new FunctionType());

        dec->setType(type);

        m_functions->insert(node->functionName->string, dec);
    }

    PreDeclarationBuilderBase::visitFunctionDeclarationStatement(node);

    closeDeclaration();
}

void PreDeclarationBuilder::openNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier, const RangeInRevision& range)
{
    if ( node == parent->namespaceNameSequence->back()->element ) {
        setComment(formatComment(parent, m_editor));
    }

    {
    DUChainWriteLocker lock;
    NamespaceDeclaration *dec = openDefinition<NamespaceDeclaration>(identifier.second, editorFindRange(node, node) );
    dec->setKind(Declaration::Namespace);
    dec->setPrettyName(identifier.first);
    m_namespaces->insert(node->string, dec);
    }

    PreDeclarationBuilderBase::openNamespace(parent, node, identifier, range);
}

void PreDeclarationBuilder::closeNamespace(NamespaceDeclarationStatementAst* parent, IdentifierAst* node, const IdentifierPair& identifier)
{
    PreDeclarationBuilderBase::closeNamespace(parent, node, identifier);

    closeDeclaration();
}

void PreDeclarationBuilder::closeDeclaration()
{
    eventuallyAssignInternalContext();
    PreDeclarationBuilderBase::closeDeclaration();
}

void PreDeclarationBuilder::closeContext()
{
    // we don't want to cleanup here, see DeclarationBuilder::closeContext()
    setCompilingContexts(false);
    PreDeclarationBuilderBase::closeContext();
    setCompilingContexts(true);
}

}
