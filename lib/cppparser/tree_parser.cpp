/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tree_parser.h"
#define DBG_SRC  QLatin1String("TreeParser")
#include "debug_utils.h"
#include "driver.h"

DEBUG_REGISTER_DISABLED(TreeParser)

TreeParser::TreeParser()
{
}

TreeParser::~TreeParser()
{
}

void TreeParser::parseTranslationUnit(const ParsedFile& translationUnit)
{
    DEBUG() << "TreeParser::parseTranslationUnit()" ;

    QList<DeclarationAST*> declarations = translationUnit->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); it++) {
        if (( *it) == nullptr) {
            DEBUG() << "declaration is zero" ;
            continue;
        }
        parseDeclaration(*it);
    }
}

void TreeParser::parseDeclaration(DeclarationAST* declaration)
{
    DEBUG() << "TreeParser::parseDeclaration()" ;

    if (!declaration)
        return;

    switch (declaration->nodeType()) {
    case NodeType_LinkageSpecification:
        parseLinkageSpecification(static_cast<LinkageSpecificationAST*>(declaration));
        break;

    case NodeType_Namespace:
        parseNamespace(static_cast<NamespaceAST*>(declaration));
        break;

    case NodeType_NamespaceAlias:
        parseNamespaceAlias(static_cast<NamespaceAliasAST*>(declaration));
        break;

    case NodeType_Using:
        parseUsing(static_cast<UsingAST*>(declaration));
        break;

    case NodeType_UsingDirective:
        parseUsingDirective(static_cast<UsingDirectiveAST*>(declaration));
        break;

    case NodeType_Typedef:
        parseTypedef(static_cast<TypedefAST*>(declaration));
        break;

    case NodeType_TemplateDeclaration:
        parseTemplateDeclaration(static_cast<TemplateDeclarationAST*>(declaration));
        break;

    case NodeType_SimpleDeclaration:
        parseSimpleDeclaration(static_cast<SimpleDeclarationAST*>(declaration));
        break;

    case NodeType_FunctionDefinition:
        parseFunctionDefinition(static_cast<FunctionDefinitionAST*>(declaration));
        break;

    case NodeType_AccessDeclaration:
        parseAccessDeclaration(static_cast<AccessDeclarationAST*>(declaration));
        break;
    }
}

void TreeParser::parseLinkageSpecification(LinkageSpecificationAST* ast)
{
    DEBUG() << "TreeParser::parseLinkageSpecification()" ;
    if (ast->linkageBody())
        parseLinkageBody(ast->linkageBody());
    else if (ast->declaration())
        parseDeclaration(ast->declaration());
}

void TreeParser::parseNamespace(NamespaceAST* decl)
{
    DEBUG() << "TreeParser::parseNamespace()" ;
    if (decl->linkageBody())
        parseLinkageBody(decl->linkageBody());
}

void TreeParser::parseNamespaceAlias(NamespaceAliasAST* decl)
{
    DEBUG() << "TreeParser::parseNamespaceAlias()" ;
    Q_UNUSED(decl);
}

void TreeParser::parseUsing(UsingAST* decl)
{
    DEBUG() << "TreeParser::parseUsing()" ;
    Q_UNUSED(decl);
}

void TreeParser::parseUsingDirective(UsingDirectiveAST* decl)
{
    DEBUG() << "TreeParser::parseUsingDirective()" ;
    Q_UNUSED(decl);
}

void TreeParser::parseTypedef(TypedefAST* decl)
{
    DEBUG() << "TreeParser::parseTypedef()" ;
    if (decl->typeSpec())
        parseTypeSpecifier(decl->typeSpec());
}

void TreeParser::parseTemplateDeclaration(TemplateDeclarationAST* decl)
{
    DEBUG() << "TreeParser::parseTemplateDeclaration()" ;
    Q_UNUSED(decl);
}

void TreeParser::parseSimpleDeclaration(SimpleDeclarationAST* decl)
{
    DEBUG() << "TreeParser::parseSimpleDeclaration()" ;
    Q_UNUSED(decl);
}

void TreeParser::parseFunctionDefinition(FunctionDefinitionAST* def)
{
    DEBUG() << "TreeParser::parseFunctionDefinition()" ;
    Q_UNUSED(def);
}

void TreeParser::parseLinkageBody(LinkageBodyAST* linkageBody)
{
    DEBUG() << "TreeParser::parseLinkageBody()" ;
    QList<DeclarationAST*> declarations = linkageBody->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); ++it) {
        parseDeclaration(*it);
    }
}

void TreeParser::parseTypeSpecifier(TypeSpecifierAST* typeSpec)
{
    DEBUG() << "TreeParser::parseTypeSpecifier()" ;
    switch (typeSpec->nodeType()) {
    case NodeType_ClassSpecifier:
        parseClassSpecifier(static_cast<ClassSpecifierAST*>(typeSpec));
        break;

    case NodeType_EnumSpecifier:
        parseEnumSpecifier(static_cast<EnumSpecifierAST*>(typeSpec));
        break;

    case NodeType_ElaboratedTypeSpecifier:
        parseElaboratedTypeSpecifier(static_cast<ElaboratedTypeSpecifierAST*>(typeSpec));
        break;
    }
}

void TreeParser::parseClassSpecifier(ClassSpecifierAST* classSpec)
{
    DEBUG() << "TreeParser::parseClassSpecifier()" ;
    QList<DeclarationAST*> declarations = classSpec->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); ++it) {
        parseDeclaration(*it);
    }
}

void TreeParser::parseEnumSpecifier(EnumSpecifierAST* enumSpec)
{
    DEBUG() << "TreeParser::parseEnumSpecifier()" ;
    Q_UNUSED(enumSpec);
}

void TreeParser::parseElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* typeSpec)
{
    DEBUG() << "TreeParser::parseElaboratedTypeSpecifier()" ;
    Q_UNUSED(typeSpec);
}

void TreeParser::parseAccessDeclaration (AccessDeclarationAST * access)
{
    DEBUG() << "TreeParser::parseAccessDeclaration()" ;
    Q_UNUSED(access);
}

