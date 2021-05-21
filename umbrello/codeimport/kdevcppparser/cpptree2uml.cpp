/*
    Based on kdevelop-3.0 languages/cpp/store_walker.cpp by Roberto Raggi

    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "cpptree2uml.h"

// app includes
#include "uml.h"
#include "umldoc.h"
#include "umllistview.h"
#include "datatype.h"
#include "operation.h"
#include "debug_utils.h"
#include "ast_utils.h"
#include "codeimpthread.h"
#include "driver.h"
#include "import_utils.h"

// FIXME: The sole reason for the next 2 includes is parseTypedef().
// Make capsule methods in ClassImport, and remove these includes.
#include "classifier.h"
// FIXME The next include is motivated by template params
#include "template.h"

// qt includes
#include <QDir>
#include <QFileInfo>
#include <QList>
#include <QRegExp>

CppTree2Uml::CppTree2Uml(const QString& fileName, CodeImpThread* thread)
  : m_thread(thread),
    m_rootFolder(0),
    m_doc(UMLApp::app()->document())
{
    clear();
    QDir dir(fileName);
    m_fileName = dir.canonicalPath();
}

CppTree2Uml::~CppTree2Uml()
{
}

void CppTree2Uml::clear()
{
    m_currentScope.clear();
    m_currentNamespace[0] = 0;  // index 0 is reserved (always 0)
    m_currentClass[0] = 0;  // index 0 is reserved (always 0)
    m_nsCnt = 0;
    m_clsCnt = 0;

    m_currentAccess = Uml::Visibility::Public;
    m_inSlots = false;
    m_inSignals = false;
    m_inStorageSpec = false;
    m_inTypedef = false;
    m_currentDeclarator = 0;
    m_anon = 0;
}

void CppTree2Uml::setRootPath(const QString &rootPath)
{
    m_rootPath = QDir::fromNativeSeparators(rootPath);
    if (Settings::optionState().codeImportState.createArtifacts) {
        if (!m_rootFolder) {
            UMLFolder *componentView = m_doc->rootFolder(Uml::ModelType::Component);
            if (!m_rootPath.isEmpty()) {
                UMLFolder *root = Import_Utils::createSubDir(m_rootPath, componentView);
                m_rootFolder = root;
            } else {
                m_rootFolder = componentView;
            }
        }
    }
}

void CppTree2Uml::parseTranslationUnit(const ParsedFile &file)
{
    clear();
    if (Settings::optionState().codeImportState.createArtifacts) {
        QFileInfo fi(file.fileName());

        UMLFolder *parent = m_rootFolder;
        QString path;
        if (!m_rootPath.isEmpty())
            path = fi.path().replace(m_rootPath + QLatin1String("/"), QLatin1String(""));
        else
            path = fi.absolutePath();
        if (!path.isEmpty())
            parent = Import_Utils::createSubDir(path, m_rootFolder);

        Import_Utils::createArtifact(fi.fileName(), parent, file->comment());
    }

    TreeParser::parseTranslationUnit(file);
}

void CppTree2Uml::parseNamespace(NamespaceAST* ast)
{
    if (m_clsCnt > 0) {
        uDebug() << "error - cannot nest namespace inside class";
        return;
    }

    QString nsName;
    if (!ast->namespaceName() || ast->namespaceName()->text().isEmpty()){
        QFileInfo fileInfo(m_fileName);
        QString shortFileName = fileInfo.baseName();

        nsName.sprintf("(%s_%d)", shortFileName.toLocal8Bit().constData(), m_anon++);
    } else {
        nsName = ast->namespaceName()->text();
    }
    uDebug() << nsName;
    if (m_thread) {
        m_thread->emitMessageToLog(QString(), QLatin1String("namespace ") + nsName);
    }
    UMLObject *o = m_doc->findUMLObject(nsName, UMLObject::ot_Package, m_currentNamespace[m_nsCnt]);
    if (!o)
        o = m_doc->findUMLObject(nsName, UMLObject::ot_Class, m_currentNamespace[m_nsCnt]);
    if (o && o->stereotype() == QLatin1String("class-or-package")) {
        o->setStereotype(QString());
        o->setBaseType(UMLObject::ot_Package);
    }
    // TODO reduce multiple finds
    else
        o = Import_Utils::createUMLObject(UMLObject::ot_Package, nsName,
                                          m_currentNamespace[m_nsCnt],
                                          ast->comment());
    UMLPackage *ns = (UMLPackage *)o;
    m_currentScope.push_back(nsName);
    if (++m_nsCnt > STACKSIZE) {
        uError() << "excessive namespace nesting";
        m_nsCnt = STACKSIZE;
    }
    m_currentNamespace[m_nsCnt] = ns;

    TreeParser::parseNamespace(ast);

    --m_nsCnt;
    m_currentScope.pop_back();
}

void CppTree2Uml::parseTypedef(TypedefAST* ast)
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    InitDeclaratorListAST* declarators = ast->initDeclaratorList();

    if (typeSpec && declarators){
        QString typeId;

        if (typeSpec->name())
            typeId = typeSpec->name()->text();

        QList<InitDeclaratorAST*> l(declarators->initDeclaratorList());
        InitDeclaratorAST* initDecl = 0;
        for (int i = 0; i < l.size(); ++i) {
            initDecl = l.at(i);
            if (initDecl==0) break;
            QString type, id;
            if (initDecl->declarator()){
               type = typeOfDeclaration(typeSpec, initDecl->declarator());

               DeclaratorAST* d = initDecl->declarator();
               while (d->subDeclarator()){
                   d = d->subDeclarator();
               }

               if (d->declaratorId())
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

            if (type.contains(QLatin1Char('*')) || isDatatype) {
                UMLObject *inner = 0;
                if (m_currentNamespace[m_nsCnt] &&
                        m_currentNamespace[m_nsCnt]->baseType() == UMLObject::ot_Class &&
                        typeId == m_currentNamespace[m_nsCnt]->name())
                    inner = m_currentNamespace[m_nsCnt];
                else
                    inner = Import_Utils::createUMLObject(UMLObject::ot_Class, type,
                                                          m_currentNamespace[m_nsCnt]);
                UMLObject *typedefObj =
                 Import_Utils::createUMLObject(UMLObject::ot_Datatype, id,
                                               m_currentNamespace[m_nsCnt]);
                UMLDatatype *dt = typedefObj->asUMLDatatype();
                if (dt) {
                    dt->setIsReference();
                    dt->setOriginType(inner->asUMLClassifier());
                }
                else {
                    uError() << "Could not create datatype from" << id;
                }
            } else {
                Import_Utils::createUMLObject(UMLObject::ot_Class, id,
                                               m_currentNamespace[m_nsCnt],
                                               QString() /* doc */,
                                               QLatin1String("typedef") /* stereotype */);
            }
        }

    }
}

void CppTree2Uml::parseTemplateDeclaration(TemplateDeclarationAST* ast)
{
    TemplateParameterListAST* parmListAST = ast->templateParameterList();
    if (parmListAST == 0)
        return;
    QList<TemplateParameterAST*> parmList = parmListAST->templateParameterList();
    for (int i = 0; i < parmList.size(); ++i) {
        // The template is either a typeParameter or a typeValueParameter.
        TemplateParameterAST* tmplParmNode = parmList.at(i);
        TypeParameterAST* typeParmNode = tmplParmNode->typeParameter();
        if (typeParmNode) {
            NameAST* nameNode = typeParmNode->name();
            if (nameNode) {
                QString typeName = nameNode->unqualifiedName()->text();
                Model_Utils::NameAndType nt(typeName, 0);
                m_templateParams.append(nt);
            } else {
                uError() << "nameNode is NULL";
            }
        }

        ParameterDeclarationAST* valueNode = tmplParmNode->typeValueParameter();
        if (valueNode) {
            TypeSpecifierAST* typeSpec = valueNode->typeSpec();
            if (typeSpec == 0) {
                uError() << "typeSpec is NULL";
                continue;
            }
            QString typeName = typeSpec->name()->text();
            UMLObject *t = Import_Utils::createUMLObject(UMLObject::ot_UMLObject, typeName,
                                                          m_currentNamespace[m_nsCnt]);
            DeclaratorAST* declNode = valueNode->declarator();
            NameAST* nameNode = declNode->declaratorId();
            if (nameNode == 0) {
                uError() << "CppTree2Uml::parseTemplateDeclaration(value):"
                         << " nameNode is NULL";
                continue;
            }
            QString paramName = nameNode->unqualifiedName()->text();
            Model_Utils::NameAndType nt(paramName, t);
            m_templateParams.append(nt);
        }
    }

    if (ast->declaration())
        TreeParser::parseDeclaration(ast->declaration());
}

void CppTree2Uml::parseSimpleDeclaration(SimpleDeclarationAST* ast)
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    InitDeclaratorListAST* declarators = ast->initDeclaratorList();
    GroupAST* storageSpec = ast->storageSpecifier();

    if (storageSpec && storageSpec->text() == QLatin1String("friend"))
        return;

    m_comment = ast->comment();

    if (typeSpec)
        parseTypeSpecifier(typeSpec);

    if (declarators){
        QList<InitDeclaratorAST*> l = declarators->initDeclaratorList();
        for (int i = 0; i < l.size(); ++i) {
            parseDeclaration2(ast->functionSpecifier(), ast->storageSpecifier(), typeSpec, l.at(i));
        }
    }
}

void CppTree2Uml::parseFunctionDefinition(FunctionDefinitionAST* ast)
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    GroupAST* funSpec = ast->functionSpecifier();
    GroupAST* storageSpec = ast->storageSpecifier();

    if (!ast->initDeclarator())
        return;

    DeclaratorAST* d = ast->initDeclarator()->declarator();

    if (!d->declaratorId())
        return;

    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;
    bool isConstructor = false;
    bool isDestructor = false;
    bool isExplicit = false;
    bool isConstExpression = false;

    if (funSpec){
        QList<AST*> l = funSpec->nodeList();
        for (int i = 0; i < l.size(); ++i) {
            QString text = l.at(i)->text();
            if (text == QLatin1String("virtual"))
                isVirtual = true;
            else if (text == QLatin1String("inline"))
                isInline = true;
            else if (text == QLatin1String("explicit"))
                isExplicit = true;
        }
    }

    if (storageSpec){
        QList<AST*> l = storageSpec->nodeList();
        for (int i = 0; i < l.size(); ++i) {
            QString text = l.at(i)->text();
            if (text == QLatin1String("friend")) isFriend = true;
            else if (text == QLatin1String("static")) isStatic = true;
            else if (text == QLatin1String("constexpr"))
                isConstExpression = true;
        }
    }

    QString id = d->declaratorId()->unqualifiedName()->text().trimmed();
    if (m_thread) {
        m_thread->emitMessageToLog(QString(), QLatin1String("method ") + id);
    }
    uDebug() << id;

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == 0) {
        uDebug() << id << ": need a surrounding class.";
        return;
    }

    QString returnType = typeOfDeclaration(typeSpec, d);
    UMLOperation *m = Import_Utils::makeOperation(c, id);
    if (isConstExpression)
        m->setStereotype(QLatin1String("constexpr"));
    if (isVirtual)
        m->setVirtual(true);
    if (isInline)
        m->setInline(true);
    if (d->override())
        m->setOverride(true);
    if (d->constant())
        m->setConst(true);
    // if a class has no return type, it could be a constructor or
    // a destructor
    if (d && returnType.isEmpty()) {
        if (id.indexOf(QLatin1Char('~')) == -1)
            isConstructor = true;
        else
            isDestructor = true;
    }

    parseFunctionArguments(d, m);
    Import_Utils::insertMethod(c, m, m_currentAccess, returnType,
                               isStatic, false /*isAbstract*/, isFriend, isConstructor,
                               isDestructor, m_comment);
    m_comment = QString();
    if  (isConstructor) {
        QString stereotype;
        if (isExplicit)
            stereotype.append(QLatin1String("explicit "));
        if (isConstExpression)
            stereotype.append(QLatin1String("constexpr "));
        stereotype.append(QLatin1String("constructor"));
        m->setStereotype(stereotype);
    } else if (isConstExpression)
        m->setStereotype(QLatin1String("constexpr"));

/* For reference, Kdevelop does some more:
    method->setFileName(m_fileName);
    if (m_inSignals)
        method->setSignal(true);
    if (m_inSlots)
        method->setSlot(true);
 */
}

void CppTree2Uml::parseClassSpecifier(ClassSpecifierAST* ast)
{
    Uml::Visibility::Enum oldAccess = m_currentAccess;
    bool oldInSlots = m_inSlots;
    bool oldInSignals = m_inSignals;

    QString kind = ast->classKey()->text();
    m_currentAccess = Uml::Visibility::fromString(kind);
    m_inSlots = false;
    m_inSignals = false;

    QString className;
    if (!ast->name() && m_currentDeclarator && m_currentDeclarator->declaratorId()) {
        className = m_currentDeclarator->declaratorId()->text().trimmed();
    } else if (!ast->name()){
        QFileInfo fileInfo(m_fileName);
        QString shortFileName = fileInfo.baseName();
        className.sprintf("(%s_%d)", shortFileName.toLocal8Bit().constData(), m_anon++);
    } else {
        className = ast->name()->unqualifiedName()->text().trimmed();
    }
    uDebug() << "name=" << className;
    if (m_thread) {
        m_thread->emitMessageToLog(QString(), QLatin1String("class ") + className);
    }
    QStringList scope = scopeOfName(ast->name(), QStringList());
    UMLObject *localParent = 0;
    if (!scope.isEmpty()) {
        localParent = m_doc->findUMLObject(scope.join(QLatin1String("::")),
                                           UMLObject::ot_Class, m_currentNamespace[m_nsCnt]);
        if (!localParent)
            localParent = m_doc->findUMLObject(scope.join(QLatin1String("::")),
                                               UMLObject::ot_Package, m_currentNamespace[m_nsCnt]);
        if (!localParent) {
            localParent = Import_Utils::createUMLObject(UMLObject::ot_Class, className,
                                                        m_currentNamespace[m_nsCnt],
                                                        ast->comment(), QString(), true);
            localParent->setStereotype(QLatin1String("class-or-package"));
        }
        m_currentNamespace[++m_nsCnt] = localParent->asUMLPackage();
    }

    if (className.isEmpty()) {
        className = QLatin1String("anon_") + QString::number(m_anon);
        m_anon++;
    }
    UMLObject *o = m_doc->findUMLObject(className, UMLObject::ot_Class, m_currentNamespace[m_nsCnt]);
    if (!o)
        o = m_doc->findUMLObject(className, UMLObject::ot_Datatype, m_currentNamespace[m_nsCnt]);
    if (o && o->stereotype() == QLatin1String("class-or-package")) {
        o->setStereotype(QString());
        o->setBaseType(UMLObject::ot_Class);
    }
    // TODO reduce multiple finds
    else
        o = Import_Utils::createUMLObject(UMLObject::ot_Class, className,
                                          m_currentNamespace[m_nsCnt],
                                          ast->comment(), QString(), true);

    UMLClassifier *klass = o->asUMLClassifier();
    flushTemplateParams(klass);
    if (ast->baseClause())
        parseBaseClause(ast->baseClause(), klass);

    m_currentScope.push_back(className);
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

    TreeParser::parseClassSpecifier(ast);

    --m_nsCnt;
    --m_clsCnt;

    m_currentScope.pop_back();

    // check is class is an interface
    bool isInterface = true;
    foreach(UMLOperation *op, klass->getOpList()) {
        if (!op->isDestructorOperation() && op->isAbstract() == false)
            isInterface = false;
    }

    foreach(UMLAttribute *attr, klass->getAttributeList()) {
        if (!(attr->isStatic() && attr->getTypeName().contains(QLatin1String("const"))))
            isInterface = false;
    }

    if (isInterface)
        klass->setBaseType(UMLObject::ot_Interface);

    m_currentAccess = oldAccess;
    m_inSlots = oldInSlots;
    m_inSignals = oldInSignals;
    if (localParent)
        m_currentNamespace[m_nsCnt--] = 0;
}

void CppTree2Uml::parseEnumSpecifier(EnumSpecifierAST* ast)
{
    NameAST *nameNode = ast->name();
    if (nameNode == 0)
        return;  // skip constants
    QString typeName = nameNode->unqualifiedName()->text().trimmed();
    if (typeName.isEmpty())
        return;  // skip constants
    UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_Enum, typeName,
                                                  m_currentNamespace[m_nsCnt],
                                                  ast->comment());

    QList<EnumeratorAST*> l = ast->enumeratorList();
    for (int i = 0; i < l.size(); ++i) {
        QString enumLiteral = l.at(i)->id()->text();
        QString enumLiteralValue = QString();
        if (l.at(i)->expr()) {
            enumLiteralValue = l.at(i)->expr()->text();
        }
        Import_Utils::addEnumLiteral((UMLEnum*)o, enumLiteral, QString(), enumLiteralValue);
    }
}

void CppTree2Uml::parseElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* typeSpec)
{
    // This is invoked for forward declarations.
    /// @todo Refine - Currently only handles class forward declarations.
    ///              - Using typeSpec->text() is probably not good, decode
    ///                the kind() instead.
    QString text = typeSpec->text();
    uDebug() << "forward declaration of " << text;
    if (m_thread) {
        m_thread->emitMessageToLog(QString(), QLatin1String("forward declaration of ") + text);
    }
    text.remove(QRegExp(QLatin1String("^class\\s+")));
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
    flushTemplateParams(o->asUMLClassifier());
}

void CppTree2Uml::parseDeclaration2(GroupAST* funSpec, GroupAST* storageSpec,
                                    TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl)
{
    if (m_inStorageSpec)
            return;

    DeclaratorAST* d = decl->declarator();

    if (!d)
        return;

    if (!d->subDeclarator() && d->parameterDeclarationClause())
        return parseFunctionDeclaration(funSpec, storageSpec, typeSpec, decl);

    DeclaratorAST* t = d;
    while (t && t->subDeclarator())
        t = t->subDeclarator();

    QString id;
    if (t && t->declaratorId() && t->declaratorId()->unqualifiedName())
        id = t->declaratorId()->unqualifiedName()->text();

    if (!scopeOfDeclarator(d, QStringList()).isEmpty()){
        uDebug() << id << ": skipping.";
        return;
    }

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == 0) {
        uDebug() << id << ": need a surrounding class.";
        return;
    }

    QString typeName = typeOfDeclaration(typeSpec, d);
    bool isFriend = false;
    bool isStatic = false;
//:unused:    bool isInitialized = decl->initializer() != 0;

    if (storageSpec){
        QList<AST*> l = storageSpec->nodeList();
        for (int i = 0; i < l.size(); ++i) {
            QString text = l.at(i)->text();
            if (text == QLatin1String("static"))
                isStatic = true;
            else if (text == QLatin1String("mutable"))
                typeName.prepend(text + QLatin1String(" "));
            else if (text == QLatin1String("friend"))
                isFriend = true;
        }
    }

    UMLAttribute *attribute = Import_Utils::insertAttribute(c, m_currentAccess, id, typeName, m_comment, isStatic);
    if (isFriend)
        attribute->setStereotype(QLatin1String("friend"));
    m_comment = QString();
}

void CppTree2Uml::parseAccessDeclaration(AccessDeclarationAST * access)
{
    QList<AST*> l = access->accessList();

    QString accessStr = l.at(0)->text();

    m_currentAccess=Uml::Visibility::fromString(accessStr);

    m_inSlots = l.count() > 1 ? l.at(1)->text() == QLatin1String("slots") : false;
    m_inSignals = l.count() >= 1 ? l.at(0)->text() == QLatin1String("signals") : false;
}

void CppTree2Uml::parseFunctionDeclaration(GroupAST* funSpec, GroupAST* storageSpec,
                                             TypeSpecifierAST * typeSpec, InitDeclaratorAST * decl)
{
    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;
    bool isPure = decl->initializer() != 0;
    bool isConstructor = false;
    bool isConstExpression = false;
    bool isDestructor = false;
    bool isExplicit = false;

    if (funSpec){
        QList<AST*> l = funSpec->nodeList();
        for (int i = 0; i < l.size(); ++i) {
            QString text = l.at(i)->text();
            if (text == QLatin1String("virtual"))
                isVirtual = true;
            else if (text == QLatin1String("inline"))
                isInline = true;
            else if (text == QLatin1String("explicit"))
                isExplicit = true;
        }
    }

    if (storageSpec){
        QList<AST*> l = storageSpec->nodeList();
        for (int i = 0; i < l.size(); ++i) {
            QString text = l.at(i)->text();
            if (text == QLatin1String("friend")) isFriend = true;
            else if (text == QLatin1String("static")) isStatic = true;
            else if (text == QLatin1String("constexpr"))
                isConstExpression = true;
        }
    }

    DeclaratorAST* d = decl->declarator();
    QString id = d->declaratorId()->unqualifiedName()->text();

    UMLClassifier *c = m_currentClass[m_clsCnt];
    if (c == 0) {
        uDebug() << id << ": need a surrounding class.";
        return;
    }

    QString returnType = typeOfDeclaration(typeSpec, d);
    // if a class has no return type, it could de a constructor or
    // a destructor
    if (d && returnType.isEmpty()) {
        if (id.indexOf(QLatin1Char('~')) == -1)
            isConstructor = true;
        else {
            isDestructor = true;
            id.remove(QLatin1String(" "));
        }
    }
    UMLOperation *m = Import_Utils::makeOperation(c, id);
    if (d->override())
        m->setOverride(true);
    if (d->constant())
        m->setConst(true);
    if (isConstExpression)
        m->setStereotype(QLatin1String("constexpr"));
    if (isVirtual)
        m->setVirtual(true);
    if (isInline)
        m->setInline(true);

    parseFunctionArguments(d, m);
    Import_Utils::insertMethod(c, m, m_currentAccess, returnType,
                               isStatic, isPure, isFriend, isConstructor, isDestructor, m_comment);
    if  (isPure)
        c->setAbstract(true);

    if  (isConstructor) {
        QString stereotype;
        if (isExplicit)
            stereotype.append(QLatin1String("explicit "));
        if (isConstExpression)
            stereotype.append(QLatin1String("constexpr "));
        stereotype.append(QLatin1String("constructor"));
        m->setStereotype(stereotype);
    } else if (isConstExpression)
        m->setStereotype(QLatin1String("constexpr"));

    m_comment = QString();
}

void CppTree2Uml::parseFunctionArguments(DeclaratorAST* declarator,
                                         UMLOperation* method)
{
    if (!declarator)
        return;
    ParameterDeclarationClauseAST* clause = declarator->parameterDeclarationClause();

    if (clause && clause->parameterDeclarationList()){
        ParameterDeclarationListAST* params = clause->parameterDeclarationList();
        QList<ParameterDeclarationAST*> l(params->parameterList());
        for (int i = 0; i < l.size(); ++i) {
            ParameterDeclarationAST* param = l.at(i);

            QString name;
            if (param->declarator())
                name = declaratorToString(param->declarator(), QString(), true);

            QString tp = typeOfDeclaration(param->typeSpec(), param->declarator());

            if (tp != QLatin1String("void"))
                Import_Utils::addMethodParameter(method, tp, name);
        }
    }
}

QString CppTree2Uml::typeOfDeclaration(TypeSpecifierAST* typeSpec, DeclaratorAST* declarator)
{
    if (!typeSpec || !declarator)
        return QString();

    QString text;

    text += typeSpec->text();

    QList<AST*> ptrOpList = declarator->ptrOpList();
    for (int i = 0; i < ptrOpList.size(); ++i) {
        text += ptrOpList.at(i)->text();
    }

    QList<AST*> arrays = declarator->arrayDimensionList();
    for(int i = 0; i < arrays.size(); ++i) {
        text += arrays.at(i)->text().replace(QLatin1String(" "), QLatin1String(""));
    }

    return text;
}

void CppTree2Uml::parseBaseClause(BaseClauseAST * baseClause, UMLClassifier* klass)
{
    QList<BaseSpecifierAST*> l = baseClause->baseSpecifierList();
    for (int i = 0; i < l.size(); ++i) {
        BaseSpecifierAST* baseSpecifier = l.at(i);

        NameAST *name = baseSpecifier->name();
        if (name == 0) {
                uDebug() << "baseSpecifier->name() is NULL";
                continue;
        }
        ClassOrNamespaceNameAST *cons = name->unqualifiedName();
        if (cons == 0) {
            uDebug() << "name->unqualifiedName() is NULL";
            continue;
        }
        QString baseName = cons->name()->text();
        Import_Utils::putAtGlobalScope(true);
        UMLObject *c = Import_Utils::createUMLObject(UMLObject::ot_Class, baseName,
                                                     m_currentNamespace[m_nsCnt],
                                                     baseSpecifier->comment());
        Import_Utils::putAtGlobalScope(false);
        Import_Utils::createGeneralization(klass, c->asUMLClassifier());
    }
}

QStringList CppTree2Uml::scopeOfName(NameAST* id, const QStringList& startScope)
{
    QStringList scope = startScope;
    if (id && id->classOrNamespaceNameList().count()){
        if (id->isGlobal())
            scope.clear();
        QList<ClassOrNamespaceNameAST*> l = id->classOrNamespaceNameList();
        for (int i = 0; i < l.size(); ++i) {
            if (l.at(i)->name()){
               scope << l.at(i)->name()->text();
            }
        }
    }

    return scope;
}

QStringList CppTree2Uml::scopeOfDeclarator(DeclaratorAST* d, const QStringList& startScope)
{
    return scopeOfName(d->declaratorId(), startScope);
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
