/* This file is part of KDevelop
    Copyright (C) 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "debug_utils.h"
#include "tree_parser.h"
#include "driver.h"

#include <kdebug.h>

DEBUG_REGISTER_DISABLED(TreeParser)

TreeParser::TreeParser()
{
}

TreeParser::~TreeParser()
{
}

void TreeParser::parseTranslationUnit(const ParsedFile& translationUnit)
{
    DEBUG("TreeParser") << "TreeParser::parseTranslationUnit()" << endl;

    QList<DeclarationAST*> declarations = translationUnit->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); it++) {
        if ((*it) == 0) {
            //kdDebug(9007) << "declaration is zero" << endl;
            continue;
        }
        parseDeclaration(*it);
    }
}

void TreeParser::parseDeclaration(DeclarationAST* declaration)
{
    DEBUG("TreeParser") << "TreeParser::parseDeclaration()" << endl;

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
    DEBUG("TreeParser") << "TreeParser::parseLinkageSpecification()" << endl;
    if (ast->linkageBody())
        parseLinkageBody(ast->linkageBody());
    else if (ast->declaration())
        parseDeclaration(ast->declaration());
}

void TreeParser::parseNamespace(NamespaceAST* decl)
{
    DEBUG("TreeParser") << "TreeParser::parseNamespace()" << endl;
    if (decl->linkageBody())
        parseLinkageBody(decl->linkageBody());
}

void TreeParser::parseNamespaceAlias(NamespaceAliasAST* decl)
{
    DEBUG("TreeParser") << "TreeParser::parseNamespaceAlias()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseUsing(UsingAST* decl)
{
    DEBUG("TreeParser") << "TreeParser::parseUsing()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseUsingDirective(UsingDirectiveAST* decl)
{
    DEBUG("TreeParser") << "TreeParser::parseUsingDirective()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseTypedef(TypedefAST* decl)
{
    DEBUG("TreeParser") << "TreeParser::parseTypedef()" << endl;
    if (decl->typeSpec())
        parseTypeSpecifier(decl->typeSpec());
}

void TreeParser::parseTemplateDeclaration(TemplateDeclarationAST* decl)
{
    DEBUG("TreeParser") << "TreeParser::parseTemplateDeclaration()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseSimpleDeclaration(SimpleDeclarationAST* decl)
{
    DEBUG("TreeParser") << "TreeParser::parseSimpleDeclaration()" << endl;
    Q_UNUSED(decl);
}

void TreeParser::parseFunctionDefinition(FunctionDefinitionAST* def)
{
    DEBUG("TreeParser") << "TreeParser::parseFunctionDefinition()" << endl;
    Q_UNUSED(def);
}

void TreeParser::parseLinkageBody(LinkageBodyAST* linkageBody)
{
    DEBUG("TreeParser") << "TreeParser::parseLinkageBody()" << endl;
    QList<DeclarationAST*> declarations = linkageBody->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); ++it) {
        parseDeclaration(*it);
    }
}

void TreeParser::parseTypeSpecifier(TypeSpecifierAST* typeSpec)
{
    DEBUG("TreeParser") << "TreeParser::parseTypeSpecifier()" << endl;
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
    DEBUG("TreeParser") << "TreeParser::parseClassSpecifier()" << endl;
    QList<DeclarationAST*> declarations = classSpec->declarationList();
    QList<DeclarationAST*>::const_iterator it = declarations.constBegin();
    for (; it != declarations.constEnd(); ++it) {
        parseDeclaration(*it);
    }
}

void TreeParser::parseEnumSpecifier(EnumSpecifierAST* enumSpec)
{
    DEBUG("TreeParser") << "TreeParser::parseEnumSpecifier()" << endl;
    Q_UNUSED(enumSpec);
}

void TreeParser::parseElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* typeSpec)
{
    DEBUG("TreeParser") << "TreeParser::parseElaboratedTypeSpecifier()" << endl;
    Q_UNUSED(typeSpec);
}

void TreeParser::parseAccessDeclaration (AccessDeclarationAST * access)
{
    DEBUG("TreeParser") << "TreeParser::parseAccessDeclaration()" << endl;
    Q_UNUSED(access);
}

