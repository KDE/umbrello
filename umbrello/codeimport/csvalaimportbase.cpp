/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "csvalaimportbase.h"

// app includes
#include "umlattribute.h"
#include "umlclassifier.h"
#include "codeimpthread.h"
#define DBG_SRC QStringLiteral("CsValaImportBase")
#include "debug_utils.h"
#include "umlenum.h"
#include "import_utils.h"
#include "operation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

DEBUG_REGISTER(CsValaImportBase)

/**
 * Override operation from NativeImportBase.
 */
bool CsValaImportBase::preprocess(QString& line)
{
    if (NativeImportBase::preprocess(line))
        return true;  // done

    // Quick and dirty: Ignore preprocessor lines.
    // TODO: Implement this properly. For example of hooking up an external
    //       preprocessor see idlimport.cpp constructor IDLImport(CodeImpThread*)
    QString trimmed = line.trimmed();
    if (trimmed.startsWith(QLatin1Char('#')))
        return true;  // done

    return false;
}

/**
 * Implement abstract operation from NativeImportBase.
 * compilation-unit:
 *     using-directives?  global-attributes?  namespace-member-declarations?
 * @return success status of operation
 */
bool CsValaImportBase::parseStmt()
{
    QString keyword = m_source[m_srcIndex];
    //uDebug() << '"' << keyword << '"';

    if (keyword == QStringLiteral("using")) {
        return parseUsingDirectives();
    }

    if (keyword == QStringLiteral("namespace")) {
        return parseNamespaceDeclaration();
    }

    if (keyword == QStringLiteral("[")) {
        return parseAnnotation();
    }

    while (isClassModifier(keyword)) {
        keyword = advance();
    }

    // type-declaration - class, interface, struct, enum, delegate
    if (isTypeDeclaration(keyword)) {
        if (keyword == QStringLiteral("struct")) {
            return parseStructDeclaration();
        }
        if (keyword == QStringLiteral("enum")) {
            return parseEnumDeclaration();
        }
        if (keyword == QStringLiteral("delegate")) {
            return parseDelegateDeclaration();
        }
        // "class" or "interface"
        return parseClassDeclaration(keyword);
    }

    if (keyword == QStringLiteral("[")) {   // ...
        advance();
        skipToClosing(QLatin1Char('['));
        return true;
    }

    if (keyword == QStringLiteral("}")) {
        if (scopeIndex())
            m_klass = popScope()->asUMLClassifier();
        else
            logError0("CsValaImportBase::parseStmt: too many }");
        return true;
    }

    // At this point, we expect to encounter a class/interface member
    // or property or method declaration.
    // These may be preceded by the visibility (public, private etc)
    // and/or other modifiers.
    while (isCommonModifier(keyword)) {
        keyword = advance();
    }

    // At this point, we expect `keyword` to be a type name
    // (of a member of class or interface, or return type
    // of an operation.) Up next is the name of the attribute
    // or operation.
    if (! keyword.contains(QRegularExpression(QStringLiteral("^\\w")))) {
        if (m_klass)
            logError4("CsValaImportBase::parseStmt: ignoring keyword %1 at index %2 of %3 (%4)",
                      keyword, m_srcIndex, m_source.count(), m_klass->name());
        else
            logError3("CsValaImportBase::parseStmt: ignoring keyword %1 at index %2 of %3",
                      keyword, m_srcIndex, m_source.count());
        return false;
    }

    QString typeName = joinTypename(keyword);
    // At this point we need a class.
    if (m_klass == nullptr) {
        logError1("CsValaImportBase::parseStmt: no class set for %1", typeName);
        return false;
    }
    QString name = advance();
    QString nextToken;
    if (typeName == m_klass->name() && name == QStringLiteral("(")) {
        // Constructor.
        nextToken = name;
        name = typeName;
        typeName.clear();
    } else {
        nextToken = advance();
    }
    if (name.contains(QRegularExpression(QStringLiteral("\\W")))) {
        logError1("CsValaImportBase::parseStmt: expecting name at %1", name);
        return false;
    }
    if (nextToken == QStringLiteral("(")) {
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        m_srcIndex++;
        while (m_srcIndex < m_source.count() && m_source[m_srcIndex] != QStringLiteral(")")) {
            QString typeName = m_source[m_srcIndex];
            if (typeName == QStringLiteral("final") || typeName.startsWith(QStringLiteral("//"))) {
                // ignore the "final" keyword and any comments in method args
                typeName = advance();
            }
            typeName = joinTypename(typeName);
            QString parName = advance();
            // the Class might not be resolved yet so resolve it if necessary
            UMLObject *obj = resolveClass(typeName);
            if (obj) {
                // by prepending the package, unwanted placeholder types will not get created
                typeName = obj->fullyQualifiedName(QStringLiteral("."));
            }
            /* UMLAttribute *att = */ Import_Utils::addMethodParameter(op, typeName, parName);
            if (advance() != QStringLiteral(","))
                break;
            m_srcIndex++;
        }
        // before adding the method, try resolving the return type
        if (typeName == QStringLiteral("void")) {
            typeName.clear();
        } else {
            UMLObject *obj = resolveClass(typeName);
            if (obj) {
                // using the fully qualified name means that a placeholder type will not be created.
                typeName = obj->fullyQualifiedName(QStringLiteral("."));
            }
        }
        Import_Utils::insertMethod(m_klass, op, m_currentAccess, typeName,
                                   m_isStatic, m_isAbstract, false /*isFriend*/,
                                   false /*isConstructor*/, false, m_comment);
        m_isAbstract = m_isStatic = false;
        // reset the default visibility
        m_currentAccess = m_defaultCurrentAccess;
        // At this point we do not know whether the method has a body or not.
        do {
            nextToken = advance();
        } while (nextToken != QStringLiteral("{") && nextToken != QStringLiteral(";"));
        if (nextToken == QStringLiteral(";")) {
            // No body (interface or abstract)
            return true;
        } else {
            return skipToClosing(QLatin1Char('{'));
        }
    }
    // At this point it should be some kind of data member or property declaration.
    if (nextToken == QStringLiteral("{")) {   // property
        // try to resolve the class type, or create a placeholder if that fails
        UMLObject *type = resolveClass(typeName);
        if (type) {
            Import_Utils::insertAttribute(
                        m_klass, m_currentAccess, name,
                        type->asUMLClassifier(), m_comment, m_isStatic);
        } else {
            Import_Utils::insertAttribute(
                        m_klass, m_currentAccess, name,
                        typeName, m_comment, m_isStatic);
        }
        skipToClosing(QLatin1Char('{'));
        // reset visibility to default
        m_currentAccess = m_defaultCurrentAccess;
        return true;
    }
    while (1) {
        while (nextToken != QStringLiteral(",") && nextToken != QStringLiteral(";")) {
            if (nextToken == QStringLiteral("=")) {
                if ((nextToken = advance()) == QStringLiteral("new")) {
                    advance();
                    if ((nextToken = advance()) == QStringLiteral("(")) {
                        skipToClosing(QLatin1Char('('));
                        if ((nextToken = advance()) == QStringLiteral("{")) {
                            skipToClosing(QLatin1Char('{'));
                        } else {
                            skipStmt();
                            break;
                        }
                    } else {
                        skipStmt();
                        break;
                    }
                } else {
                    skipStmt();
                    break;
                }
            } else {
                name += nextToken;  // add possible array dimensions to `name`
            }
            nextToken = advance();
            if (nextToken.isEmpty()) {
                break;
            }
        }
        // try to resolve the class type, or create a placeholder if that fails
        UMLObject *type = resolveClass(typeName);
        if (type) {
            Import_Utils::insertAttribute(
                        m_klass, m_currentAccess, name,
                        type->asUMLClassifier(), m_comment, m_isStatic);
        } else {
            Import_Utils::insertAttribute(
                        m_klass, m_currentAccess, name,
                        typeName, m_comment, m_isStatic);
        }
        // UMLAttribute *attr = o->asUMLAttribute();
        if (nextToken != QStringLiteral(",")) {
            // reset the modifiers
            m_isStatic = m_isAbstract = false;
            break;
        }
        name = advance();
        nextToken = advance();
    }
    // reset visibility to default
    m_currentAccess = m_defaultCurrentAccess;
    if (m_srcIndex < m_source.count()) {
        if (m_source[m_srcIndex] != QStringLiteral(";")) {
            logError1("CsValaImportBase::parseStmt: ignoring trailing items at %1", name);
            skipStmt();
        }
    } else {
        logError1("CsValaImportBase::parseStmt index out of range: ignoring statement %1", name);
        skipStmt();
    }
    return true;
}

/**
 * Parsing the statement 'using'.
 * Keep track of imports so we can resolve classes we are dependent on.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseUsingDirectives()
{
    QString import = advance();
    log(QStringLiteral("using ") + import);
    if (import.contains(QLatin1Char('='))) {
        //this is an alias to represent the namespace name
        //:TODO: import = import + advance();
    }
    m_imports.append(import);

    // move past ;
    skipStmt();
    return true;
}

/**
 * Parsing global attributes.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseGlobalAttributes()
{
    //:TODO:
    return true;
}

/**
 * Parsing the statement 'namespace'.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseNamespaceDeclaration()
{
    m_currentPackage = advance();
    const QString& qualifiedName = m_currentPackage;
    const QStringList names = qualifiedName.split(QLatin1Char('.'));
    for (QStringList::ConstIterator it = names.begin(); it != names.end(); ++it) {
        QString name = (*it);
        log(QLatin1String("namespace ") + name);
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                        name, currentScope(), m_comment, QString(), true);
        pushScope(ns->asUMLPackage());
    }
    log(QStringLiteral("namespace ") + m_currentPackage);
    const QString& next = advance();
    if (next != QStringLiteral("{") && next != QStringLiteral(";")) {
        logError2("parseNamespaceDeclaration(%1): Expecting ';' or '{', found %2",
                  m_currentPackage, next);
        return false;
    }
    return true;
}

/**
 * Parsing attributes.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseAnnotation()
{
    QString token, annotation;
    while ((token = advance()) != QStringLiteral("]") && !token.isEmpty()) {
        annotation.append(token);
    }
    log(QStringLiteral("attribute ") + annotation);
    m_comment.append(QStringLiteral("\n") + annotation);
    return true;
}

/**
 * Check if keyword is belonging to a type-declaration.
 * @return   result of check
 */
bool CsValaImportBase::isTypeDeclaration(const QString& keyword)
{
    if (keyword == QStringLiteral("class")     ||
        keyword == QStringLiteral("struct")    ||
        keyword == QStringLiteral("interface") ||
        keyword == QStringLiteral("enum")      ||
        keyword == QStringLiteral("delegate")) {
        // log("type-declaration: " + keyword);
        return true;
    }
    else {
        return false;
    }
}

/**
 * Check if keyword is a class-modifier.
 * @return   result of check
 */
bool CsValaImportBase::isClassModifier(const QString& keyword)
{
    if (isCommonModifier(keyword) ||
        keyword == QStringLiteral("sealed")) {
        return true;
    }
    if (keyword == QStringLiteral("abstract")) {
        m_isAbstract = true;
        return true;
    }
    return false;
}

/**
 * Check if keyword is an interface, struct, enum or delegate modifier.
 * @return   result of check
 */
bool CsValaImportBase::isCommonModifier(const QString& keyword)
{
    if (keyword == QStringLiteral("public")) {
        m_currentAccess = Uml::Visibility::Public;
        return true;
    }
    if (keyword == QStringLiteral("protected")) {
        m_currentAccess = Uml::Visibility::Protected;
        return true;
    }
    if (keyword == QStringLiteral("private")) {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == QStringLiteral("static")) {
        m_isStatic = true;
        return true;
    }
    if (keyword == QStringLiteral("new")       ||
        keyword == QStringLiteral("internal")  ||
        keyword == QStringLiteral("readonly")  ||
        keyword == QStringLiteral("volatile")  ||
        keyword == QStringLiteral("virtual")   ||
        keyword == QStringLiteral("override")  ||
        keyword == QStringLiteral("unsafe")    ||
        keyword == QStringLiteral("extern")    ||
        keyword == QStringLiteral("partial")   ||
        keyword == QStringLiteral("async")) {
        return true;
    }
    return false;
}

/**
 * Parsing the statement 'enum'.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseEnumDeclaration()
{
    const QString& name = advance();
    log(QStringLiteral("enum ") + name);
    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                        name, currentScope(), m_comment);
    UMLEnum *enumType = ns->asUMLEnum();
    if (enumType == nullptr)
        enumType = Import_Utils::remapUMLEnum(ns, currentScope());
    if (!enumType) {
       logError1("CsValaImportBase::parseEnumDeclaration: error on creating %1", name);
       skipStmt(QStringLiteral("}"));
       return false;
    }
    skipStmt(QStringLiteral("{"));
    while (m_srcIndex < m_source.count() - 1 && advance() != QStringLiteral("}")) {
        if (m_source[m_srcIndex] == QStringLiteral("[")) {
            parseAnnotation();
            continue;
        }
        QString enumValue = m_source[m_srcIndex];
        QString representation;
        QString next = advance();
        if (next == QStringLiteral("=")) {
            representation = advance();
            next = advance();
        }
        Import_Utils::addEnumLiteral(enumType, enumValue, m_comment, representation);
        m_comment.clear();
        if (next == QStringLiteral("{") || next == QStringLiteral("(")) {
            if (! skipToClosing(next[0]))
                return false;
            next = advance();
        }
        if (next != QStringLiteral(",")) {
            if (next == QStringLiteral(";")) {
                // @todo handle methods in enum
                // For now, we cheat (skip them)
                m_source[m_srcIndex] = QLatin1Char('{');
                if (! skipToClosing(QLatin1Char('{')))
                    return false;
            }
            break;
        }
    }
    return true;
}

/**
 * Parsing a struct-declaration.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseStructDeclaration()
{
    const QString& name = advance();
    log(QStringLiteral("struct ") + name + QStringLiteral(" --> parsing not yet implemented!"));
    return true;
}

/**
 * Parsing a delegate-declaration.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseDelegateDeclaration()
{
    // return-type identifier (formal-parameter-list?) ;
    const QString& returnType = advance();
    const QString& name = advance();
    log(QStringLiteral("delegate ") + name + QStringLiteral("with return-type ") + returnType);
    skipStmt(QStringLiteral(";"));
    return true;
}

/**
 * Parsing the statement 'class' or 'interface'.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseClassDeclaration(const QString& keyword)
{
    const QString& name = advance();
    const UMLObject::ObjectType ot = (keyword == QStringLiteral("class") ? UMLObject::ot_Class
                                                                        : UMLObject::ot_Interface);
    log(keyword + QLatin1Char(' ') + name);
    UMLObject *ns = Import_Utils::createUMLObject(ot, name, currentScope(), m_comment);
    pushScope(m_klass = ns->asUMLClassifier());
    m_klass->setStatic(m_isStatic);
    m_klass->setVisibilityCmd(m_currentAccess);
    // The UMLObject found by createUMLObject might originally have been created as a
    // placeholder with a type of class but if is really an interface, then we need to
    // change it.
    m_klass->setBaseType(ot);
    // TODO: UMLClassifier::setBaseType() resets abstract flag
    m_klass->setAbstract(m_isAbstract);
    m_isAbstract = m_isStatic = false;
    // if no modifier is specified in an interface, then it means public
    if (m_klass->isInterface()) {
        m_defaultCurrentAccess =  Uml::Visibility::Public;
    }

    if (advance() == QStringLiteral(";"))    // forward declaration
        return true;

    if (m_source[m_srcIndex] == QStringLiteral("<")) {
        // template args - preliminary, rudimentary implementation
        // @todo implement all template arg syntax
        uint start = m_srcIndex;
        if (! skipToClosing(QLatin1Char('<'))) {
            logError1("CsValaImportBase::parseClassDeclaration (%1): template syntax error", name);
            return false;
        }
        while(1) {
            const QString arg = m_source[++start];
            if (! arg.contains(QRegularExpression(QStringLiteral("^[A-Za-z_]")))) {
                logDebug2("import C# (%1): cannot handle template syntax (%2)", name, arg);
                break;
            }
            /* UMLTemplate *tmpl = */ m_klass->addTemplate(arg);
            const QString next = m_source[++start];
            if (next == QStringLiteral(">"))
                break;
            if (next != QStringLiteral(",")) {
                logDebug2("import C# (%1): cannot handle template syntax (%2)", name, next);
                break;
            }
        }
        advance();  // skip over ">"
    }

    if (m_source[m_srcIndex] == QStringLiteral(":")) {   // derivation
        while (m_srcIndex < m_source.count() - 1 && advance() != QStringLiteral("{")) {
            const QString& baseName = m_source[m_srcIndex];
            // try to resolve the interface we are implementing, if this fails
            // create a placeholder
            UMLObject *interface = resolveClass(baseName);
            if (interface) {
                Import_Utils::createGeneralization(m_klass, interface->asUMLClassifier());
            } else {
                logDebug1("CsValaImportBase::parseClassDeclaration: implementing interface %1 "
                          "is not resolvable. Creating placeholder", baseName);
                Import_Utils::createGeneralization(m_klass, baseName);
            }
            if (advance() != QStringLiteral(","))
                break;
        }
    }

    if (m_source[m_srcIndex] != QStringLiteral("{")) {
        logError2("CsValaImportBase::parseClassDeclaration ignoring excess chars at %1 (index %2)",
                  name, m_source[m_srcIndex]);
        skipStmt(QStringLiteral("{"));
    }
    return true;
}
