/***************************************************************************
 *   cpptree2uml.h - based on kdevelop/languages/cpp/store_walker.h,       *
 *                   Copyright (C) 2003 by Roberto Raggi                   *
 *   Copyright (C) 2004 by Oliver Kellogg <okellogg@users.sourceforge.net> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CPPTREE2UML_H
#define CPPTREE2UML_H

#include "tree_parser.h"
#include "../package.h"
#include "../class.h"
#include "../umlattributelist.h"
#include <qstringlist.h>
#include <qptrstack.h>

// fwd decls
class ClassImport;
class UMLClass;

class CppTree2Uml: public TreeParser
{
public:
    CppTree2Uml( const QString& fileName, ClassImport* store );
    virtual ~CppTree2Uml();

    //FileDom file() { return m_file; }

    // translation-unit
    virtual void parseTranslationUnit( TranslationUnitAST* );

    // declarations
    //virtual void parseDeclaration( DeclarationAST* );  // use parent method
    //virtual void parseLinkageSpecification( LinkageSpecificationAST* );  // use parent method
    virtual void parseNamespace( NamespaceAST* );
    //virtual void parseNamespaceAlias( NamespaceAliasAST* );  // use parent method
    //virtual void parseUsing( UsingAST* );  // use parent method
    //virtual void parseUsingDirective( UsingDirectiveAST* );  // use parent method
    virtual void parseTypedef( TypedefAST* );
    virtual void parseTemplateDeclaration( TemplateDeclarationAST* );
    virtual void parseSimpleDeclaration( SimpleDeclarationAST* );
    virtual void parseFunctionDefinition( FunctionDefinitionAST* );
    //virtual void parseLinkageBody( LinkageBodyAST* );  // use parent method
    virtual void parseAccessDeclaration( AccessDeclarationAST* );

    // type-specifier
    //virtual void parseTypeSpecifier( TypeSpecifierAST* );  // use parent method
    virtual void parseClassSpecifier( ClassSpecifierAST* );
    virtual void parseEnumSpecifier( EnumSpecifierAST* );
    //virtual void parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* );  // use parent method

    // non-overriding (locally added) methods

    virtual void parseDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    virtual void parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    void parseFunctionArguments( DeclaratorAST* declarator, UMLOperation* method);
    virtual void parseBaseClause( BaseClauseAST* baseClause, UMLClass* klass );

private:
    //NamespaceDom findOrInsertNamespace( NamespaceAST* ast, const QString& name );

    QString typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator );
    QStringList scopeOfName( NameAST* id, const QStringList& scope );
    QStringList scopeOfDeclarator( DeclaratorAST* d, const QStringList& scope );

private:
    //FileDom m_file;
    QString m_fileName;
    QStringList m_currentScope;
    ClassImport* m_importer;
    int m_currentAccess;
    bool m_inSlots;
    bool m_inSignals;
    int m_anon;
    bool m_inStorageSpec;
    bool m_inTypedef;

    DeclaratorAST* m_currentDeclarator;
    QPtrStack<UMLPackage> m_currentNamespace;
    QPtrStack<UMLClass> m_currentClass;

private:
    CppTree2Uml( const CppTree2Uml& source );
    void operator = ( const CppTree2Uml& source );
};

#endif // CPPTREE2UML
