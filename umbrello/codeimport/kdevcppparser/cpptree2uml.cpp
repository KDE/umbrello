/***************************************************************************
 *   Based on kdevelop-3.0 languages/cpp/store_walker.cpp by Roberto Raggi *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cpptree2uml.h"
// qt/kde includes
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "ast_utils.h"
#include "urlutil.h"
#include "../import_utils.h"
// FIXME: The sole reason for the next 2 includes is parseTypedef().
// Make capsule methods in ClassImport, and remove these includes.
#include "../../classifier.h"
// FIXME The next include is motivated by template params
#include "../../template.h"

CppTree2Uml::CppTree2Uml( const QString& fileName)
    : m_anon( 0 ), m_nsCnt( 0 ), m_clsCnt( 0 )
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

    m_currentAccess = Uml::Visibility::Public;
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
        kDebug() << "CppTree2Uml::parseNamespace: error - cannot nest namespace inside class"
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
    kDebug() << "CppTree2Uml::parseNamespace: " << nsName << endl;
#endif
    UMLObject * o = Import_Utils::createUMLObject( Uml::ot_Package, nsName,
                                                 m_currentNamespace[m_nsCnt],
                                                 ast->comment());
    UMLPackage *ns = (UMLPackage *)o;
    m_currentScope.push_back( nsName );
    if (++m_nsCnt > STACKSIZE) {
        kdError() << "CppTree2Uml::parseNamespace: excessive namespace nesting" << endl;
        m_nsCnt = STACKSIZE;
    }
    m_currentNamespace[m_nsCnt] = ns;

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
            kDebug() << "CppTree2Uml::parseTypedef: name=" << id << ", type=" << type << endl;
//#endif
            /* @todo Trace typedefs back to their root type for deciding
                     whether to build a Datatype (for pointers.)  */
            /* check out if the ID type is a Datatype
               ex: typedef unsigned int uint;
               where unsigned int is a known datatype
               I'm not sure if setIsReference() should be run
             */
            bool isDatatype = Import_Utils::isDatatype(typeId, m_currentNamespace[m_nsCnt]);

            if (type.contains('*') || isDatatype) {
                UMLObject *inner =
                Import_Utils::createUMLObject( Uml::ot_Class, typeId,
                                             m_currentNamespace[m_nsCnt] );
                UMLObject *typedefObj =
                Import_Utils::createUMLObject( Uml::ot_Datatype, id,
                                             m_currentNamespace[m_nsCnt] );
                UMLClassifier *dt = static_cast<UMLClassifier*>(typedefObj);
                dt->setIsReference();
                dt->setOriginType(static_cast<UMLClassifier*>(inner));
            } else {
                Import_Utils::createUMLObject( Uml::ot_Class, id,
                                             m_currentNamespace[m_nsCnt],
                                             "" /* doc */,
                                             "typedef" /* stereotype */);
            }
            ++it;
        }

    }
#endif
}

void CppTree2Uml::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
    TemplateParameterListAST* parmListAST = ast->templateParameterList();
    if (parmListAST == NULL)
        return;
    QPtrList<TemplateParameterAST> parmList = parmListAST->templateParameterList();
    for (QPtrListIterator<TemplateParameterAST> it(parmList); it.current(); ++it) {
        // The template is either a typeParameter or a typeValueParameter.

        TemplateParameterAST* tmplParmNode = it.current();
        TypeParameterAST* typeParmNode = tmplParmNode->typeParameter();
        if (typeParmNode) {
            NameAST* nameNode = typeParmNode->name();
            if (nameNode) {
                QString typeName = nameNode->unqualifiedName()->text();
                Model_Utils::NameAndType nt(typeName, NULL);
                m_templateParams.append(nt);
            } else {
                kdError() << "CppTree2Uml::parseTemplateDeclaration(type):"
                          << " nameNode is NULL" << endl;
            }
        }

        ParameterDeclarationAST* valueNode = tmplParmNode->typeValueParameter();
        if (valueNode) {
            TypeSpecifierAST* typeSpec = valueNode->typeSpec();
            if (typeSpec == NULL) {
                kdError() << "CppTree2Uml::parseTemplateDeclaration(value):"
                          << " typeSpec is NULL" << endl;
                continue;
            }
            QString typeName = typeSpec->name()->text();
            UMLObject *t = Import_Utils::createUMLObject( Uml::ot_UMLObject, typeName,
                                                        m_currentNamespace[m_nsCnt] );
            DeclaratorAST* declNode = valueNode->declarator();
            NameAST* nameNode = declNode->declaratorId();
            if (nameNode == NULL) {
                kdError() << "CppTree2Uml::parseTemplateDeclaration(value):"
                          << " nameNode is NULL" << endl;
                continue;
            }
            QString paramName = nameNode->unqualifiedName()->text();
            Model_Utils::NameAndType nt(paramName, t);
            m_templateParams.append(nt);
        }
    }

    if( ast->declaration() )
        TreeParser::parseDeclaration( ast->declaration() );
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
    bool isConstructor = false;

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

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        kDebug() << "CppTree2Uml::parseFunctionDefinition (" << id
                  << "): need a surrounding class." << endl;
        return;
    }

    QString returnType = typeOfDeclaration( typeSpec, d );
    UMLOperation *m = Import_Utils::makeOperation(c, id);
    // if a class has no return type, it could be a constructor or
    // a destructor
    if (d && returnType.isEmpty() && id.find("~") == -1)
        isConstructor = true;

    parseFunctionArguments( d, m );
    Import_Utils::insertMethod( c, m, m_currentAccess, returnType,
                              isStatic, false /*isAbstract*/, isFriend, isConstructor, m_comment);
    m_comment = "";

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
    Uml::Visibility oldAccess = m_currentAccess;
    bool oldInSlots = m_inSlots;
    bool oldInSignals = m_inSignals;

    QString kind = ast->classKey()->text();
    m_currentAccess=Uml::Visibility::fromString(kind);
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
//#ifdef DEBUG_CPPTREE2UML
    kDebug() << "CppTree2Uml::parseClassSpecifier: name=" << className << endl;
//#endif
    if( !scopeOfName( ast->name(), QStringList() ).isEmpty() ){
        kDebug() << "skip private class declarations" << endl;
        return;
    }

    if (className.isEmpty()) {
        className = "anon_" + QString::number(m_anon);
        m_anon++;
    }
    UMLObject * o = Import_Utils::createUMLObject( Uml::ot_Class, className,
                                                 m_currentNamespace[m_nsCnt],
                                                 ast->comment() );
    UMLClassifier *klass = static_cast<UMLClassifier*>(o);
    flushTemplateParams(klass);
    if ( ast->baseClause() )
        parseBaseClause( ast->baseClause(), klass );

    m_currentScope.push_back( className );
    if (++m_clsCnt > STACKSIZE) {
        kdError() << "CppTree2Uml::parseNamespace: excessive class nesting" << endl;
        m_clsCnt = STACKSIZE;
    }
    m_currentClass[m_clsCnt] = klass;
    if (++m_nsCnt > STACKSIZE) {
        kdError() << "CppTree2Uml::parseNamespace: excessive namespace nesting" << endl;
        m_nsCnt = STACKSIZE;
    }
    m_currentNamespace[m_nsCnt] = (UMLPackage*)klass;

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
    NameAST *nameNode = ast->name();
    if (nameNode == NULL)
        return;  // skip constants
    QString typeName = nameNode->unqualifiedName()->text().stripWhiteSpace();
    if (typeName.isEmpty())
        return;  // skip constants
    UMLObject *o = Import_Utils::createUMLObject( Uml::ot_Enum, typeName,
                                                m_currentNamespace[m_nsCnt],
                                                ast->comment() );

    QPtrList<EnumeratorAST> l = ast->enumeratorList();
    QPtrListIterator<EnumeratorAST> it( l );
    while ( it.current() ) {
        QString enumLiteral = it.current()->id()->text();
        Import_Utils::addEnumLiteral( (UMLEnum*)o, enumLiteral );
        ++it;
    }
}

void CppTree2Uml::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* typeSpec )
{
    // This is invoked for forward declarations.
    /// @todo Refine - Currently only handles class forward declarations.
    ///              - Using typeSpec->text() is probably not good, decode
    ///                the kind() instead.
    QString text = typeSpec->text();
    kDebug() << "CppTree2Uml::parseElaboratedTypeSpecifier: text is " << text << endl;
    text.remove(QRegExp("^class\\s+"));
    UMLObject *o = Import_Utils::createUMLObject(Uml::ot_Class, text, m_currentNamespace[m_nsCnt]);
    flushTemplateParams( static_cast<UMLClassifier*>(o) );
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
        kDebug() << "CppTree2Uml::parseDeclaration (" << id << "): skipping."
                  << endl;
        return;
    }

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        kDebug() << "CppTree2Uml::parseDeclaration (" << id
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

    Import_Utils::insertAttribute( c, m_currentAccess, id, typeName,
                                 m_comment, isStatic);
    m_comment = "";
}

void CppTree2Uml::parseAccessDeclaration( AccessDeclarationAST * access )
{
    QPtrList<AST> l = access->accessList();

    QString accessStr = l.at( 0 )->text();

    m_currentAccess=Uml::Visibility::fromString(accessStr);

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
    bool isConstructor = false;

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

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        kDebug() << "CppTree2Uml::parseFunctionDeclaration (" << id
                  << "): need a surrounding class." << endl;
        return;
    }

    QString returnType = typeOfDeclaration( typeSpec, d );
    UMLOperation *m = Import_Utils::makeOperation(c, id);
    // if a class has no return type, it could be a constructor or
    // a destructor
    if (d && returnType.isEmpty() && id.find("~") == -1)
        isConstructor = true;

    parseFunctionArguments( d, m );
    Import_Utils::insertMethod( c, m, m_currentAccess, returnType,
                              isStatic, isPure, isFriend, isConstructor, m_comment);
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

            if (tp != "void")
                Import_Utils::addMethodParameter( method, tp, name );
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

void CppTree2Uml::parseBaseClause( BaseClauseAST * baseClause, UMLClassifier* klass )
{
    QPtrList<BaseSpecifierAST> l = baseClause->baseSpecifierList();
    QPtrListIterator<BaseSpecifierAST> it( l );
    while( it.current() ){
        BaseSpecifierAST* baseSpecifier = it.current();
        ++it;

        if (baseSpecifier->name() == NULL) {
                kDebug() << "CppTree2Uml::parseBaseClause: baseSpecifier->name() is NULL"
                          << endl;
                continue;
        }

        QString baseName = baseSpecifier->name()->text();
        Import_Utils::createGeneralization( klass, baseName );
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

void CppTree2Uml::flushTemplateParams(UMLClassifier *klass) {
    if (m_templateParams.count()) {
        Model_Utils::NameAndType_ListIt it;
        for (it = m_templateParams.begin(); it != m_templateParams.end(); ++it) {
            const Model_Utils::NameAndType &nt = *it;
            kDebug() << "CppTree2Uml::parseClassSpecifier: adding template param: "
                      << nt.m_name << endl;
            UMLTemplate *tmpl = klass->addTemplate(nt.m_name);
            tmpl->setType(nt.m_type);
        }
        m_templateParams.clear();
    }
}

