/***************************************************************************
 *   Based on kdevelop-3.0 languages/cpp/store_walker.cpp by Roberto Raggi *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "cpptree2uml.h"

// app includes
#include "debug_utils.h"
#include "ast_utils.h"
#include "codeimpthread.h"
#include "import_utils.h"
// FIXME: The sole reason for the next 2 includes is parseTypedef().
// Make capsule methods in ClassImport, and remove these includes.
#include "classifier.h"
// FIXME The next include is motivated by template params
#include "template.h"

// qt includes
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QList>

CppTree2Uml::CppTree2Uml(const QString& fileName, CodeImpThread* thread)
    : m_anon( 0 ), m_nsCnt( 0 ), m_clsCnt( 0 ), m_thread( thread )
{
    QDir dir(fileName);
    m_fileName = dir.canonicalPath();
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

void CppTree2Uml::parseFile( FileAST* ast )
{
    Import_Utils::createUMLObject(UMLObject::ot_Artifact, ast->fileName(),
                                  0,
                                  ast->comment());
}

void CppTree2Uml::parseNamespace( NamespaceAST* ast )
{
    if (m_clsCnt > 0) {
        uDebug() << "error - cannot nest namespace inside class";
        return;
    }

    QString nsName;
    if( !ast->namespaceName() || ast->namespaceName()->text().isEmpty() ){
        QFileInfo fileInfo( m_fileName );
        QString shortFileName = fileInfo.baseName();

        nsName.sprintf( "(%s_%d)", shortFileName.toLocal8Bit().constData(), m_anon++ );
    } else {
        nsName = ast->namespaceName()->text();
    }
    uDebug() << nsName;
    if (m_thread) {
        m_thread->emitMessageToLog("", "namespace " + nsName);
    }
    UMLObject * o = Import_Utils::createUMLObject(UMLObject::ot_Package, nsName,
                                                  m_currentNamespace[m_nsCnt],
                                                  ast->comment());
    UMLPackage *ns = (UMLPackage *)o;
    m_currentScope.push_back( nsName );
    if (++m_nsCnt > STACKSIZE) {
        uError() << "excessive namespace nesting";
        m_nsCnt = STACKSIZE;
    }
    m_currentNamespace[m_nsCnt] = ns;

    TreeParser::parseNamespace( ast );

    --m_nsCnt;
    m_currentScope.pop_back();
}

void CppTree2Uml::parseTypedef( TypedefAST* ast )
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    InitDeclaratorListAST* declarators = ast->initDeclaratorList();

    if( typeSpec && declarators ){
        QString typeId;

        if( typeSpec->name() )
            typeId = typeSpec->name()->text();

        QList<InitDeclaratorAST*> l( declarators->initDeclaratorList() );
        InitDeclaratorAST* initDecl = 0;
        for( int i = 0; i < l.size(); ++i ) {
            initDecl = l.at(i);
            if (initDecl==0) break;
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
                Import_Utils::createUMLObject( UMLObject::ot_Class, typeId,
                                               m_currentNamespace[m_nsCnt] );
                UMLObject *typedefObj =
                Import_Utils::createUMLObject( UMLObject::ot_Datatype, id,
                                               m_currentNamespace[m_nsCnt] );
                UMLClassifier *dt = static_cast<UMLClassifier*>(typedefObj);
                dt->setIsReference();
                dt->setOriginType(static_cast<UMLClassifier*>(inner));
            } else {
                Import_Utils::createUMLObject( UMLObject::ot_Class, id,
                                               m_currentNamespace[m_nsCnt],
                                               "" /* doc */,
                                               "typedef" /* stereotype */);
            }
        }

    }
}

void CppTree2Uml::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
    TemplateParameterListAST* parmListAST = ast->templateParameterList();
    if (parmListAST == NULL)
        return;
    QList<TemplateParameterAST*> parmList = parmListAST->templateParameterList();
    for( int i = 0; i < parmList.size(); ++i ) {
        // The template is either a typeParameter or a typeValueParameter.
        TemplateParameterAST* tmplParmNode = parmList.at(i);
        TypeParameterAST* typeParmNode = tmplParmNode->typeParameter();
        if (typeParmNode) {
            NameAST* nameNode = typeParmNode->name();
            if (nameNode) {
                QString typeName = nameNode->unqualifiedName()->text();
                Model_Utils::NameAndType nt(typeName, NULL);
                m_templateParams.append(nt);
            } else {
                uError() << "nameNode is NULL";
            }
        }

        ParameterDeclarationAST* valueNode = tmplParmNode->typeValueParameter();
        if (valueNode) {
            TypeSpecifierAST* typeSpec = valueNode->typeSpec();
            if (typeSpec == NULL) {
                uError() << "typeSpec is NULL";
                continue;
            }
            QString typeName = typeSpec->name()->text();
            UMLObject *t = Import_Utils::createUMLObject( UMLObject::ot_UMLObject, typeName,
                                                          m_currentNamespace[m_nsCnt] );
            DeclaratorAST* declNode = valueNode->declarator();
            NameAST* nameNode = declNode->declaratorId();
            if (nameNode == NULL) {
                uError() << "CppTree2Uml::parseTemplateDeclaration(value):"
                         << " nameNode is NULL";
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
        QList<InitDeclaratorAST*> l = declarators->initDeclaratorList();
        for( int i = 0; i < l.size(); ++i) {
            parseDeclaration2(  ast->functionSpecifier(), ast->storageSpecifier(), typeSpec, l.at(i) );
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
        QList<AST*> l = funSpec->nodeList();
        for( int i = 0; i < l.size(); ++i ) {
            QString text = l.at(i)->text();
            if( text == "virtual" ) isVirtual = true;
            else if( text == "inline" ) isInline = true;
        }
    }

    if( storageSpec ){
        QList<AST*> l = storageSpec->nodeList();
        for( int i = 0; i < l.size(); ++i ) {
            QString text = l.at(i)->text();
            if( text == "friend" ) isFriend = true;
            else if( text == "static" ) isStatic = true;
        }
    }

    QString id = d->declaratorId()->unqualifiedName()->text().trimmed();
    if (m_thread) {
        m_thread->emitMessageToLog("", "method " + id);
    }
    uDebug() << id;

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        uDebug() << id << ": need a surrounding class.";
        return;
    }

    QString returnType = typeOfDeclaration( typeSpec, d );
    UMLOperation *m = Import_Utils::makeOperation(c, id);
    // if a class has no return type, it could be a constructor or
    // a destructor
    if (d && returnType.isEmpty() && id.indexOf('~') == -1)
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
        className = m_currentDeclarator->declaratorId()->text().trimmed();
    } else if( !ast->name() ){
        QFileInfo fileInfo( m_fileName );
        QString shortFileName = fileInfo.baseName();
        className.sprintf( "(%s_%d)", shortFileName.toLocal8Bit().constData(), m_anon++ );
    } else {
        className = ast->name()->unqualifiedName()->text().trimmed();
    }
    uDebug() << "name=" << className;
    if (m_thread) {
        m_thread->emitMessageToLog("", "class " + className);
    }
    if( !scopeOfName( ast->name(), QStringList() ).isEmpty() ){
        uDebug() << "skip private class declarations";
        return;
    }

    if (className.isEmpty()) {
        className = "anon_" + QString::number(m_anon);
        m_anon++;
    }
    UMLObject * o = Import_Utils::createUMLObject( UMLObject::ot_Class, className,
                                                   m_currentNamespace[m_nsCnt],
                                                   ast->comment() );
    UMLClassifier *klass = static_cast<UMLClassifier*>(o);
    flushTemplateParams(klass);
    if ( ast->baseClause() )
        parseBaseClause( ast->baseClause(), klass );

    m_currentScope.push_back( className );
    if (++m_clsCnt > STACKSIZE) {
        uError() << "excessive class nesting";
        m_clsCnt = STACKSIZE;
    }
    m_currentClass[m_clsCnt] = klass;
    if (++m_nsCnt > STACKSIZE) {
        uError() << "excessive namespace nesting";
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
    QString typeName = nameNode->unqualifiedName()->text().trimmed();
    if (typeName.isEmpty())
        return;  // skip constants
    UMLObject *o = Import_Utils::createUMLObject( UMLObject::ot_Enum, typeName,
                                                  m_currentNamespace[m_nsCnt],
                                                  ast->comment() );

    QList<EnumeratorAST*> l = ast->enumeratorList();
    for( int i = 0; i < l.size(); ++i ) {
        QString enumLiteral = l.at(i)->id()->text();
        Import_Utils::addEnumLiteral( (UMLEnum*)o, enumLiteral );
    }
}

void CppTree2Uml::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* typeSpec )
{
    // This is invoked for forward declarations.
    /// @todo Refine - Currently only handles class forward declarations.
    ///              - Using typeSpec->text() is probably not good, decode
    ///                the kind() instead.
    QString text = typeSpec->text();
    uDebug() << "forward declaration of " << text;
    if (m_thread) {
        m_thread->emitMessageToLog("", "forward declaration of " + text);
    }
    text.remove(QRegExp("^class\\s+"));
#if 0
    if (m_thread) {  //:TODO: for testing only
        int answer;
        m_thread->emitAskQuestion("Soll CppTree2Uml::parseElaboratedTypeSpecifier ausgeführt werden?");
        uDebug() << "Antwort: " << answer;
    }
#endif
    UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_Class, text, m_currentNamespace[m_nsCnt]);
#if 0
    if (m_thread) {  //:TODO: for testing only
        m_thread->emitAskQuestion("Soll nach CppTree2Uml::parseElaboratedTypeSpecifier weiter gemacht werden?");
    }
#endif
    flushTemplateParams( static_cast<UMLClassifier*>(o) );
}

void CppTree2Uml::parseDeclaration2( GroupAST* funSpec, GroupAST* storageSpec,
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
        uDebug() << id << ": skipping.";
        return;
    }

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        uDebug() << id << ": need a surrounding class.";
        return;
    }

    QString typeName = typeOfDeclaration( typeSpec, d );
    bool isFriend = false;
    bool isStatic = false;
    //bool isInitialized = decl->initializer() != 0;

    if( storageSpec ){
        QList<AST*> l = storageSpec->nodeList();
        for( int i = 0; i < l.size(); ++i ) {
            QString text = l.at(i)->text();
            if( text == "friend" ) isFriend = true;
            else if( text == "static" ) isStatic = true;
        }
    }

    Import_Utils::insertAttribute( c, m_currentAccess, id, typeName,
                                 m_comment, isStatic);
    m_comment = "";
}

void CppTree2Uml::parseAccessDeclaration( AccessDeclarationAST * access )
{
    QList<AST*> l = access->accessList();

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
        QList<AST*> l = funSpec->nodeList();
        for( int i = 0; i < l.size(); ++i ) {
            QString text = l.at(i)->text();
            if( text == "virtual" ) isVirtual = true;
            else if( text == "inline" ) isInline = true;
        }
    }

    if( storageSpec ){
        QList<AST*> l = storageSpec->nodeList();
        for( int i = 0; i < l.size(); ++i ) {
            QString text = l.at(i)->text();
            if( text == "friend" ) isFriend = true;
            else if( text == "static" ) isStatic = true;
        }
    }

    DeclaratorAST* d = decl->declarator();
    QString id = d->declaratorId()->unqualifiedName()->text();

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == NULL) {
        uDebug() << id << ": need a surrounding class.";
        return;
    }

    QString returnType = typeOfDeclaration( typeSpec, d );
    UMLOperation *m = Import_Utils::makeOperation(c, id);
    // if a class has no return type, it could de a constructor or
    // a destructor
    if (d && returnType.isEmpty() && id.indexOf('~') == -1)
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
        QList<ParameterDeclarationAST*> l( params->parameterList() );
        for( int i = 0; i < l.size(); ++i) {
            ParameterDeclarationAST* param = l.at(i);

            QString name;
            if (param->declarator())
                name = declaratorToString(param->declarator(), QString(), true );

            QString tp = typeOfDeclaration( param->typeSpec(), param->declarator() );

            if (tp != "void")
                Import_Utils::addMethodParameter( method, tp, name );
        }
    }
}

QString CppTree2Uml::typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator )
{
    if( !typeSpec || !declarator )
        return QString();

    QString text;

    text += typeSpec->text();

    QList<AST*> ptrOpList = declarator->ptrOpList();
    for( int i = 0; i < ptrOpList.size(); ++i ) {
        text += ptrOpList.at(i)->text();
    }

    return text;
}

void CppTree2Uml::parseBaseClause( BaseClauseAST * baseClause, UMLClassifier* klass )
{
    QList<BaseSpecifierAST*> l = baseClause->baseSpecifierList();
    for( int i = 0; i < l.size(); ++i ) {
        BaseSpecifierAST* baseSpecifier = l.at(i);

        if (baseSpecifier->name() == NULL) {
                uDebug() << "baseSpecifier->name() is NULL";
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
        QList<ClassOrNamespaceNameAST*> l = id->classOrNamespaceNameList();
        for( int i = 0; i < l.size(); ++i ) {
            if( l.at(i)->name() ){
               scope << l.at(i)->name()->text();
            }
        }
    }

    return scope;
}

QStringList CppTree2Uml::scopeOfDeclarator( DeclaratorAST* d, const QStringList& startScope )
{
    return scopeOfName( d->declaratorId(), startScope );
}

/**
 * Flush template parameters pending in m_templateParams to the klass.
 */
void CppTree2Uml::flushTemplateParams(UMLClassifier *klass)
{
    if (m_templateParams.count()) {
        Model_Utils::NameAndType_ListIt it;
        for (it = m_templateParams.begin(); it != m_templateParams.end(); ++it) {
            const Model_Utils::NameAndType &nt = *it;
            uDebug() << "adding template param: " << nt.m_name;
            UMLTemplate *tmpl = klass->addTemplate(nt.m_name);
            tmpl->setType(nt.m_type);
        }
        m_templateParams.clear();
    }
}
