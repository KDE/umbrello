/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CPPTREE2UML_H
#define CPPTREE2UML_H

#include "tree_parser.h"
#include "model_utils.h"

#include <QtCore/QStringList>

// fwd decls
class CodeImpThread;
class UMLClassifier;
class UMLOperation;
class UMLPackage;

class CppTree2Uml: public TreeParser
{
public:
    explicit CppTree2Uml( const QString& fileName, CodeImpThread* thread = 0);
    virtual ~CppTree2Uml();

    //FileDom file() { return m_file; }

    // translation-unit
    virtual void parseTranslationUnit( TranslationUnitAST* );

    // declarations
    //virtual void parseDeclaration( DeclarationAST* );  // use parent method
    //virtual void parseLinkageSpecification( LinkageSpecificationAST* );  // use parent method
    virtual void parseFile( FileAST* ast );
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
    virtual void parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* );

    // non-overriding (locally added) methods

    virtual void parseDeclaration2( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    virtual void parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
    void parseFunctionArguments( DeclaratorAST* declarator, UMLOperation* method);
    virtual void parseBaseClause( BaseClauseAST* baseClause, UMLClassifier* klass );

private:
    //NamespaceDom findOrInsertNamespace( NamespaceAST* ast, const QString& name );

    QString typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator );
    QStringList scopeOfName( NameAST* id, const QStringList& scope );
    QStringList scopeOfDeclarator( DeclaratorAST* d, const QStringList& scope );
    void flushTemplateParams(UMLClassifier *klass);

private:
    //FileDom m_file;
    QString m_fileName;
    QStringList m_currentScope;
    Uml::Visibility m_currentAccess;
    bool m_inSlots;
    bool m_inSignals;
    int m_anon;
    bool m_inStorageSpec;
    bool m_inTypedef;
    QString m_comment;
    Model_Utils::NameAndType_List m_templateParams;

    DeclaratorAST* m_currentDeclarator;
#   define STACKSIZE 30
    UMLPackage* m_currentNamespace[STACKSIZE+1];  ///< stack
    UMLClassifier* m_currentClass[STACKSIZE+1];  ///< stack
    int m_nsCnt;  ///< stack top for m_currentNamespace
    int m_clsCnt;  ///< stack top for m_currentClass
    CodeImpThread* m_thread;

private:
    CppTree2Uml( const CppTree2Uml& source );
    void operator = ( const CppTree2Uml& source );
};

#endif // CPPTREE2UML
