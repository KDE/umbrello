/***************************************************************************
 *   cpptree2uml.cpp - based on kdevelop/languages/cpp/store_walker.cpp,   *
 *		     Copyright (C) 2003 by Roberto Raggi		 *
 *   Copyright (C) 2004 by Oliver Kellogg <okellogg@users.sourceforge.net> *
 *									 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
 *									 *
 ***************************************************************************/

#include "cpptree2uml.h"
#include "ast_utils.h"
#include "urlutil.h"
#include "../classimport.h"
// FIXME: The sole reason for the next 2 includes is parseTypedef().
// Make capsule methods in ClassImport, and remove these includes.
#include "../classifier.h"
#include "../datatype.h"

#include <kdebug.h>
#include <qfileinfo.h>
#include <qdir.h>

CppTree2Uml::CppTree2Uml( const QString& fileName, ClassImport* store )
    : m_importer( store ), m_anon( 0 ), m_nsCnt( 0 ), m_clsCnt( 0 )
{
    m_fileName = URLUtil::canonicalPath(fileName);
}

CppTree2Uml::~CppTree2Uml()
{
}

void CppTree2Uml::parseTranslationUnit( TranslationUnitAST* ast )
{
    m_currentScope.clear();
    m_currentNamespace[0] = NULL;  // index 0 is reserved (always NULL)
    m_currentClass[0] = NULL;  // index 0 is reserved (always NULL)
    m_nsCnt = 0;
    m_clsCnt = 0;

    m_currentAccess = Uml::Public;
    m_inSlots = false;
    m_inSignals = false;
    m_inStorageSpec = false;
    m_inTypedef = false;
    m_currentDeclarator = 0;
    m_anon = 0;

    TreeParser::parseTranslationUnit( ast );
}

void CppTree2Uml::parseNamespace( NamespaceAST* ast )
{
    if (m_clsCnt > 0) {
	kdDebug() << "CppTree2Uml::parseNamespace: error - cannot nest namespace inside class"
		  << endl;
	return;
    }

    QString nsName;
    if( !ast->namespaceName() || ast->namespaceName()->text().isEmpty() ){
	QFileInfo fileInfo( m_fileName );
	QString shortFileName = fileInfo.baseName();

	nsName.sprintf( "(%s_%d)", shortFileName.local8Bit().data(), m_anon++ );
    } else {
	nsName = ast->namespaceName()->text();
    }

#ifdef DEBUG_CPPTREE2UML
    kdDebug() << "CppTree2Uml::parseNamespace: " << nsName << endl;
#endif
    UMLObject * o = m_importer->createUMLObject( Uml::ot_Package, nsName, "" /*stereotype*/,
						 ast->comment(),
						 m_currentNamespace[m_nsCnt] );
    UMLPackage *ns = (UMLPackage *)o;
    m_currentScope.push_back( nsName );
    m_currentNamespace[++m_nsCnt] = ns;

    TreeParser::parseNamespace( ast );

    --m_nsCnt;
    m_currentScope.pop_back();
}

void CppTree2Uml::parseTypedef( TypedefAST* ast )
{
#if 0
    DeclaratorAST* oldDeclarator = m_currentDeclarator;

    if( ast && ast->initDeclaratorList() && ast->initDeclaratorList()->initDeclaratorList().count() > 0 ) {
	    QPtrList<InitDeclaratorAST> lst( ast->initDeclaratorList()->initDeclaratorList() );
	    m_currentDeclarator = lst.at( 0 )->declarator();
    }

    m_inTypedef = true;

    TreeParser::parseTypedef( ast );

    m_inTypedef = false;
    m_currentDeclarator = oldDeclarator;
#else
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    InitDeclaratorListAST* declarators = ast->initDeclaratorList();

    if( typeSpec && declarators ){
	QString typeId;

	if( typeSpec->name() )
	    typeId = typeSpec->name()->text();

	QPtrList<InitDeclaratorAST> l( declarators->initDeclaratorList() );
	QPtrListIterator<InitDeclaratorAST> it( l );

	InitDeclaratorAST* initDecl = 0;
	while( 0 != (initDecl = it.current()) ){

	    QString type, id;
	    if( initDecl->declarator() ){
	       type = typeOfDeclaration( typeSpec, initDecl->declarator() );

	       DeclaratorAST* d = initDecl->declarator();
	       while( d->subDeclarator() ){
		   d = d->subDeclarator();
	       }

	       if( d->declaratorId() )
		  id = d->declaratorId()->text();
	    }
//#ifdef DEBUG_CPPTREE2UML
	    kdDebug() << "CppTree2Uml::parseTypedef: name=" << id << ", type=" << type << endl;
//#endif
	    if (type.contains('*')) {
		UMLObject *inner =
	        m_importer->createUMLObject( Uml::ot_Class, typeId,
					     "" /* stereotype */,
	 				     "" /* doc */,
					     m_currentNamespace[m_nsCnt] );
	        UMLObject *typedefObj =
		m_importer->createUMLObject( Uml::ot_Datatype, id,
					     "" /* stereotype */,
	 				     "" /* doc */,
					     m_currentNamespace[m_nsCnt] );
	        UMLDatatype *dt = static_cast<UMLDatatype*>(typedefObj);
		dt->setIsReference();
		dt->setOriginType(static_cast<UMLClassifier*>(inner));
	    } else {
	        m_importer->createUMLObject( Uml::ot_Class, id,
					     "typedef" /* stereotype */,
	 				     "" /* doc */,
					     m_currentNamespace[m_nsCnt] );
	    }
	    ++it;
	}

    }
#endif
}

void CppTree2Uml::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
    if( ast->declaration() )
	TreeParser::parseDeclaration( ast->declaration() );

    TreeParser::parseTemplateDeclaration( ast );
}

void CppTree2Uml::parseSimpleDeclaration( SimpleDeclarationAST* ast )
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    InitDeclaratorListAST* declarators = ast->initDeclaratorList();

    m_comment = ast->comment();

    if( typeSpec )
	parseTypeSpecifier( typeSpec );

    if( declarators ){
	QPtrList<InitDeclaratorAST> l = declarators->initDeclaratorList();

	QPtrListIterator<InitDeclaratorAST> it( l );
	while( it.current() ){
	    parseDeclaration(  ast->functionSpecifier(), ast->storageSpecifier(), typeSpec, it.current() );
	    ++it;
	}
    }
}

void CppTree2Uml::parseFunctionDefinition( FunctionDefinitionAST* ast )
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    GroupAST* funSpec = ast->functionSpecifier();
    GroupAST* storageSpec = ast->storageSpecifier();

    if( !ast->initDeclarator() )
	return;

    DeclaratorAST* d = ast->initDeclarator()->declarator();

    if( !d->declaratorId() )
	return;

    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;

    if( funSpec ){
	QPtrList<AST> l = funSpec->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    QString text = it.current()->text();
	    if( text == "virtual" ) isVirtual = true;
	    else if( text == "inline" ) isInline = true;
	    ++it;
	}
    }

    if( storageSpec ){
	QPtrList<AST> l = storageSpec->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    QString text = it.current()->text();
	    if( text == "friend" ) isFriend = true;
	    else if( text == "static" ) isStatic = true;
	    ++it;
	}
    }

    QString id = d->declaratorId()->unqualifiedName()->text().stripWhiteSpace();

    UMLClass *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        kdDebug() << "CppTree2Uml::parseFunctionDefinition (" << id
		  << "): need a surrounding class." << endl;
	return;
    }
    QString returnType = typeOfDeclaration( typeSpec, d );
    UMLOperation *m = m_importer->makeOperation(c, id);
    m_importer->insertMethod( c, m, (Uml::Scope)m_currentAccess, returnType,
			      isStatic, false /*isAbstract*/, m_comment );
    m_comment = "";
    parseFunctionArguments( d, m );

/* For reference, Kdevelop does some more:
    method->setFileName( m_fileName );
    if( m_inSignals )
	method->setSignal( true );
    if( m_inSlots )
	method->setSlot( true );
 */
}

void CppTree2Uml::parseClassSpecifier( ClassSpecifierAST* ast )
{
    int oldAccess = m_currentAccess;
    bool oldInSlots = m_inSlots;
    bool oldInSignals = m_inSignals;

    QString kind = ast->classKey()->text();
    if( kind == "class" )
	m_currentAccess = Uml::Private;
    else
	m_currentAccess = Uml::Public;
    m_inSlots = false;
    m_inSignals = false;

    QString className;
    if( !ast->name() && m_currentDeclarator && m_currentDeclarator->declaratorId() ) {
	className = m_currentDeclarator->declaratorId()->text().stripWhiteSpace();
    } else if( !ast->name() ){
	QFileInfo fileInfo( m_fileName );
	QString shortFileName = fileInfo.baseName();
	className.sprintf( "(%s_%d)", shortFileName.local8Bit().data(), m_anon++ );
    } else {
	className = ast->name()->unqualifiedName()->text().stripWhiteSpace();
    }
#ifdef DEBUG_CPPTREE2UML
    kdDebug() << "CppTree2Uml::parseClassSpecifier: name=" << className << endl;
#endif
    if( !scopeOfName( ast->name(), QStringList() ).isEmpty() ){
	kdDebug() << "skip private class declarations" << endl;
	return;
    }

    UMLObject * o = m_importer->createUMLObject( Uml::ot_Class, className, "" /*stereotype*/,
						 ast->comment(),
						 m_currentNamespace[m_nsCnt] );
    UMLClass *klass = (UMLClass *)o;

    if ( ast->baseClause() )
	parseBaseClause( ast->baseClause(), klass );

    m_currentScope.push_back( className );
    m_currentClass[++m_clsCnt] = klass;
    m_currentNamespace[++m_nsCnt] = (UMLPackage*)klass;

    TreeParser::parseClassSpecifier( ast );

    --m_nsCnt;
    --m_clsCnt;

    m_currentScope.pop_back();

    m_currentAccess = oldAccess;
    m_inSlots = oldInSlots;
    m_inSignals = oldInSignals;
}

void CppTree2Uml::parseEnumSpecifier( EnumSpecifierAST* ast )
{
    QString typeName = ast->name()->unqualifiedName()->text().stripWhiteSpace();
    UMLObject *o = m_importer->createUMLObject( Uml::ot_Enum, typeName,
						"" /* stereotype */,
						ast->comment(),
						m_currentNamespace[m_nsCnt] );

    QPtrList<EnumeratorAST> l = ast->enumeratorList();
    QPtrListIterator<EnumeratorAST> it( l );
    while ( it.current() ) {
	QString enumLiteral = it.current()->id()->text();
	m_importer->addEnumLiteral( (UMLEnum*)o, enumLiteral );
	++it;
    }
}

void CppTree2Uml::parseDeclaration( GroupAST* funSpec, GroupAST* storageSpec,
				    TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl )
{
    if( m_inStorageSpec )
	    return;

    DeclaratorAST* d = decl->declarator();

    if( !d )
	return;

    if( !d->subDeclarator() && d->parameterDeclarationClause() )
	return parseFunctionDeclaration( funSpec, storageSpec, typeSpec, decl );

    DeclaratorAST* t = d;
    while( t && t->subDeclarator() )
	t = t->subDeclarator();

    QString id;
    if( t && t->declaratorId() && t->declaratorId()->unqualifiedName() )
	id = t->declaratorId()->unqualifiedName()->text();

    if( !scopeOfDeclarator(d, QStringList()).isEmpty() ){
	kdDebug() << "CppTree2Uml::parseDeclaration (" << id << "): skipping."
		  << endl;
	return;
    }

    UMLClass *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        kdDebug() << "CppTree2Uml::parseDeclaration (" << id
		  << "): need a surrounding class." << endl;
	return;
    }

    QString typeName = typeOfDeclaration( typeSpec, d );
    bool isFriend = false;
    bool isStatic = false;
    //bool isInitialized = decl->initializer() != 0;

    if( storageSpec ){
	QPtrList<AST> l = storageSpec->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    QString text = it.current()->text();
	    if( text == "friend" ) isFriend = true;
	    else if( text == "static" ) isStatic = true;
	    ++it;
	}
    }

    m_importer->insertAttribute( c, (Uml::Scope)m_currentAccess, id, typeName,
				 m_comment, isStatic);
    m_comment = "";
}

void CppTree2Uml::parseAccessDeclaration( AccessDeclarationAST * access )
{
    QPtrList<AST> l = access->accessList();

    QString accessStr = l.at( 0 )->text();
    if( accessStr == "public" )
	m_currentAccess = Uml::Public;
    else if( accessStr == "protected" )
	m_currentAccess = Uml::Protected;
    else if( accessStr == "private" )
	m_currentAccess = Uml::Private;
    else if( accessStr == "signals" )
	m_currentAccess = Uml::Protected;
    else
	m_currentAccess = Uml::Public;

    m_inSlots = l.count() > 1 ? l.at( 1 )->text() == "slots" : false;
    m_inSignals = l.count() >= 1 ? l.at( 0 )->text() == "signals" : false;
}

void CppTree2Uml::parseFunctionDeclaration(  GroupAST* funSpec, GroupAST* storageSpec,
					     TypeSpecifierAST * typeSpec, InitDeclaratorAST * decl )
{
    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;
    bool isPure = decl->initializer() != 0;

    if( funSpec ){
	QPtrList<AST> l = funSpec->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    QString text = it.current()->text();
	    if( text == "virtual" ) isVirtual = true;
	    else if( text == "inline" ) isInline = true;
	    ++it;
	}
    }

    if( storageSpec ){
	QPtrList<AST> l = storageSpec->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    QString text = it.current()->text();
	    if( text == "friend" ) isFriend = true;
	    else if( text == "static" ) isStatic = true;
	    ++it;
	}
    }

    DeclaratorAST* d = decl->declarator();
    QString id = d->declaratorId()->unqualifiedName()->text();

    UMLClass *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        kdDebug() << "CppTree2Uml::parseFunctionDeclaration (" << id
		  << "): need a surrounding class." << endl;
	return;
    }

    QString returnType = typeOfDeclaration( typeSpec, d );
    UMLOperation *m = m_importer->makeOperation(c, id);
    parseFunctionArguments( d, m );
    m_importer->insertMethod( c, m, (Uml::Scope)m_currentAccess, returnType,
			      isStatic, isPure, m_comment );
    m_comment = "";
}

void CppTree2Uml::parseFunctionArguments(DeclaratorAST* declarator,
					 UMLOperation* method)
{
    ParameterDeclarationClauseAST* clause = declarator->parameterDeclarationClause();

    if( clause && clause->parameterDeclarationList() ){
	ParameterDeclarationListAST* params = clause->parameterDeclarationList();
	QPtrList<ParameterDeclarationAST> l( params->parameterList() );
	QPtrListIterator<ParameterDeclarationAST> it( l );
	while( it.current() ){
	    ParameterDeclarationAST* param = it.current();
	    ++it;

	    QString name;
	    if (param->declarator())
		name = declaratorToString(param->declarator(), QString::null, true );

	    QString tp = typeOfDeclaration( param->typeSpec(), param->declarator() );

	    m_importer->addMethodParameter( method, tp, name, "" /*initialValue*/,
	    							 "" /*doc*/);
	}
    }
}

QString CppTree2Uml::typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator )
{
    if( !typeSpec || !declarator )
	return QString::null;

    QString text;

    text += typeSpec->text();

    QPtrList<AST> ptrOpList = declarator->ptrOpList();
    for( QPtrListIterator<AST> it(ptrOpList); it.current(); ++it ){
	text += it.current()->text();
    }

    return text;
}

void CppTree2Uml::parseBaseClause( BaseClauseAST * baseClause, UMLClass* klass )
{
    QPtrList<BaseSpecifierAST> l = baseClause->baseSpecifierList();
    QPtrListIterator<BaseSpecifierAST> it( l );
    while( it.current() ){
	BaseSpecifierAST* baseSpecifier = it.current();
	++it;

	if (baseSpecifier->name() == NULL) {
		kdDebug() << "CppTree2Uml::parseBaseClause: baseSpecifier->name() is NULL"
			  << endl;
		continue;
	}

	QString baseName = baseSpecifier->name()->text();
	m_importer->createGeneralization( klass, baseName );
    }
}

QStringList CppTree2Uml::scopeOfName( NameAST* id, const QStringList& startScope )
{
    QStringList scope = startScope;
    if( id && id->classOrNamespaceNameList().count() ){
	if( id->isGlobal() )
	    scope.clear();
	QPtrList<ClassOrNamespaceNameAST> l = id->classOrNamespaceNameList();
	QPtrListIterator<ClassOrNamespaceNameAST> it( l );
	while( it.current() ){
	    if( it.current()->name() ){
	       scope << it.current()->name()->text();
	    }
	    ++it;
	}
    }

    return scope;
}

QStringList CppTree2Uml::scopeOfDeclarator( DeclaratorAST* d, const QStringList& startScope )
{
    return scopeOfName( d->declaratorId(), startScope );
}
