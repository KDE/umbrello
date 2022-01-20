/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tree_parser.h"
#include "debug_utils.h"
#include "driver.h"

#if QT_VERSION >= 0x050000
#else
#include <kdebug.h>
#endif

#define DBG_TPAR  DEBUG_N(QLatin1String("TreeParser"))
DEBUG_REGISTER_DISABLED(TreeParser)

TreeParser::TreeParser()
{
}

TreeParser::~TreeParser()
{
}

void TreeParser::parseTranslationUnit(const ParsedFile& translationUnit)
{
    DBG_TPAR << "TreeParser::parseTranslationUnit()" << endl;

    QList<DeclarationAST*> declarations = translationUnit->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); it++) {
        if ((*it) == 0) {
            DBG_TPAR << "declaration is zero" << endl;
            continue;
        }
        parseDeclaration(*it);
    }
}

void TreeParser::parseDeclaration(DeclarationAST* declaration)
{
    DBG_TPAR << "TreeParser::parseDeclaration()" << endl;

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
    DBG_TPAR << "TreeParser::parseLinkageSpecification()" << endl;
    if (ast->linkageBody())
        parseLinkageBody(ast->linkageBody());
    else if (ast->declaration())
        parseDeclaration(ast->declaration());
}

void TreeParser::parseNamespace(NamespaceAST* decl)
{
    DBG_TPAR << "TreeParser::parseNamespace()" << endl;
    if (decl->linkageBody())
        parseLinkageBody(decl->linkageBody());
}

void TreeParser::parseNamespaceAlias(NamespaceAliasAST* decl)
{
    DBG_TPAR << "TreeParser::parseNamespaceAlias()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseUsing(UsingAST* decl)
{
    DBG_TPAR << "TreeParser::parseUsing()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseUsingDirective(UsingDirectiveAST* decl)
{
    DBG_TPAR << "TreeParser::parseUsingDirective()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseTypedef(TypedefAST* decl)
{
    DBG_TPAR << "TreeParser::parseTypedef()" << endl;
    if (decl->typeSpec())
        parseTypeSpecifier(decl->typeSpec());
}

void TreeParser::parseTemplateDeclaration(TemplateDeclarationAST* decl)
{
    DBG_TPAR << "TreeParser::parseTemplateDeclaration()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseSimpleDeclaration(SimpleDeclarationAST* decl)
{
    DBG_TPAR << "TreeParser::parseSimpleDeclaration()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseFunctionDefinition(FunctionDefinitionAST* def)
{
    DBG_TPAR << "TreeParser::parseFunctionDefinition()" << endl;
    Q_UNUSED(def);
}

void TreeParser::parseLinkageBody(LinkageBodyAST* linkageBody)
{
    DBG_TPAR << "TreeParser::parseLinkageBody()" << endl;
    QList<DeclarationAST*> declarations = linkageBody->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); ++it) {
        parseDeclaration(*it);
    }
}

void TreeParser::parseTypeSpecifier(TypeSpecifierAST* typeSpec)
{
    DBG_TPAR << "TreeParser::parseTypeSpecifier()" << endl;
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
    DBG_TPAR << "TreeParser::parseClassSpecifier()" << endl;
    QList<DeclarationAST*> declarations = classSpec->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); ++it) {
        parseDeclaration(*it);
    }
}

void TreeParser::parseEnumSpecifier(EnumSpecifierAST* enumSpec)
{
    DBG_TPAR << "TreeParser::parseEnumSpecifier()" << endl;
    Q_UNUSED(enumSpec);
}

void TreeParser::parseElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* typeSpec)
{
    DBG_TPAR << "TreeParser::parseElaboratedTypeSpecifier()" << endl;
    Q_UNUSED(typeSpec);
}

void TreeParser::parseAccessDeclaration (AccessDeclarationAST * access)
{
    DBG_TPAR << "TreeParser::parseAccessDeclaration()" << endl;
    Q_UNUSED(access);
}

