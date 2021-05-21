/* This file is part of KDevelop
    SPDX-FileCopyrightText: 2002, 2003 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __tree_parser_h
#define __tree_parser_h

#include "ast.h"
class ParsedFile;

class TreeParser
{
public:
    TreeParser();
    virtual ~TreeParser();

    // translation-unit
    virtual void parseTranslationUnit(const ParsedFile&);

    // declarations
    virtual void parseDeclaration(DeclarationAST*);
    virtual void parseLinkageSpecification(LinkageSpecificationAST*);
    virtual void parseNamespace(NamespaceAST*);
    virtual void parseNamespaceAlias(NamespaceAliasAST*);
    virtual void parseUsing(UsingAST*);
    virtual void parseUsingDirective(UsingDirectiveAST*);
    virtual void parseTypedef(TypedefAST*);
    virtual void parseTemplateDeclaration(TemplateDeclarationAST*);
    virtual void parseSimpleDeclaration(SimpleDeclarationAST*);
    virtual void parseFunctionDefinition(FunctionDefinitionAST*);
    virtual void parseLinkageBody(LinkageBodyAST*);
    virtual void parseAccessDeclaration(AccessDeclarationAST*);

    // type-specifier
    virtual void parseTypeSpecifier(TypeSpecifierAST*);
    virtual void parseClassSpecifier(ClassSpecifierAST*);
    virtual void parseEnumSpecifier(EnumSpecifierAST*);
    virtual void parseElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST*);

private:
    TreeParser(const TreeParser& source);
    void operator = (const TreeParser& source);
};

#endif // __tree_parser_h
