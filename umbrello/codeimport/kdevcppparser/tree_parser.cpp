/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "tree_parser.h"

#include "debug_utils.h"

#include <QtCore/QList>

TreeParser::TreeParser()
{
}

TreeParser::~TreeParser()
{
}

void TreeParser::parseTranslationUnit(TranslationUnitAST* translationUnit)
{
    QList<DeclarationAST*> declarations = translationUnit->declarationList();
    for( int i = 0; i < declarations.size(); ++i ) {
        parseDeclaration( declarations.at(i) );
    }
}

void TreeParser::parseDeclaration(DeclarationAST* declaration)
{
    //uDebug() << "TreeParser::parseDeclaration()";

    if (!declaration)
        return;

    switch (declaration->nodeType()) {
    case NodeType_File:
        parseFile(static_cast<FileAST*>(declaration));
        break;

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

void TreeParser::parseFile(FileAST* decl)
{
    //uDebug() << "TreeParser::parseFile()";
    Q_UNUSED(decl);
}

void TreeParser::parseLinkageSpecification(LinkageSpecificationAST* ast)
{
    //uDebug() << "TreeParser::parseLinkageSpecification()";
    if (ast->linkageBody())
        parseLinkageBody(ast->linkageBody());
    else if (ast->declaration())
        parseDeclaration(ast->declaration());
}

void TreeParser::parseNamespace(NamespaceAST* decl)
{
    //uDebug() << "TreeParser::parseNamespace()";
    if (decl->linkageBody())
        parseLinkageBody(decl->linkageBody());
}

void TreeParser::parseNamespaceAlias(NamespaceAliasAST* decl)
{
    //uDebug() << "TreeParser::parseNamespaceAlias()";
    Q_UNUSED(decl);
}

void TreeParser::parseUsing(UsingAST* decl)
{
    //uDebug() << "TreeParser::parseUsing()";
    Q_UNUSED(decl);
}

void TreeParser::parseUsingDirective(UsingDirectiveAST* decl)
{
    //uDebug() << "TreeParser::parseUsingDirective()";
    Q_UNUSED(decl);
}

void TreeParser::parseTypedef(TypedefAST* decl)
{
    //uDebug() << "TreeParser::parseTypedef()";
    if (decl->typeSpec())
        parseTypeSpecifier(decl->typeSpec());
}

void TreeParser::parseTemplateDeclaration(TemplateDeclarationAST* decl)
{
    //uDebug() << "TreeParser::parseTemplateDeclaration()";
    Q_UNUSED(decl);
}

void TreeParser::parseSimpleDeclaration(SimpleDeclarationAST* decl)
{
    //uDebug() << "TreeParser::parseSimpleDeclaration()";
    Q_UNUSED(decl);
}

void TreeParser::parseFunctionDefinition(FunctionDefinitionAST* def)
{
    //uDebug() << "TreeParser::parseFunctionDefinition()";
    Q_UNUSED(def);
}

void TreeParser::parseLinkageBody(LinkageBodyAST* linkageBody)
{
    //uDebug() << "TreeParser::parseLinkageBody()";
    QList<DeclarationAST*> declarations = linkageBody->declarationList();
    for( int i = 0; i < declarations.size(); ++i ) {
        parseDeclaration( declarations.at(i) );
    }
}

void TreeParser::parseTypeSpecifier(TypeSpecifierAST* typeSpec)
{
    //uDebug() << "TreeParser::parseTypeSpecifier()";
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
    //uDebug() << "TreeParser::parseClassSpecifier()";
    QList<DeclarationAST*> declarations = classSpec->declarationList();
    for( int i = 0; i < declarations.size(); ++i ) {
        parseDeclaration( declarations.at(i) );
    }
}

void TreeParser::parseEnumSpecifier(EnumSpecifierAST* enumSpec)
{
    //uDebug() << "TreeParser::parseEnumSpecifier()";
    Q_UNUSED(enumSpec);
}

void TreeParser::parseElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* typeSpec)
{
    //uDebug() << "TreeParser::parseElaboratedTypeSpecifier()";
    Q_UNUSED(typeSpec);
}

void TreeParser::parseAccessDeclaration(AccessDeclarationAST * access)
{
    //uDebug() << "TreeParser::parseAccessDeclaration()";
    Q_UNUSED(access);
}

