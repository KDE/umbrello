/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "csvalaimportbase.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "codeimpthread.h"
#define DBG_SRC QStringLiteral("CsValaImportBase")
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "operation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QFile>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

DEBUG_REGISTER(CsValaImportBase)

QStringList CsValaImportBase::s_filesAlreadyParsed;
int CsValaImportBase::s_parseDepth = 0;

/**
 * Constructor.
 */
CsValaImportBase::CsValaImportBase(CodeImpThread* thread)
  : NativeImportBase(QStringLiteral("//"), thread),
    m_defaultCurrentAccess(Uml::Visibility::Public)
{
    m_language = Uml::ProgrammingLanguage::CSharp;
    setMultiLineComment(QStringLiteral("/*"), QStringLiteral("*/"));
    initVars();
}

/**
 * Destructor.
 */
CsValaImportBase::~CsValaImportBase()
{
}

QString CsValaImportBase::fileExtension()
{
    return QString();  // to be reimplemented by the derived classes
}

/**
 * Reimplement operation from NativeImportBase.
 */
void CsValaImportBase::initVars()
{
    m_isStatic = false;
}

/**
 * Figure out if the type is really an array or template of the given typeName.
 * Catenate possible template arguments/array dimensions to the end of the type name.
 * @param typeName   the type name
 * @return the type name with the additional information
 */
QString CsValaImportBase::joinTypename(const QString& typeName)
{
    QString typeNameRet(typeName);
    QString next = lookAhead();
    if (!next.isEmpty()) {
        if (next == QStringLiteral("<") || next == QStringLiteral("[")) {
            int start = ++m_srcIndex;
            if (! skipToClosing(m_source[start][0]))
                return typeNameRet;
            for (int i = start; i <= m_srcIndex; ++i) {
                typeNameRet += m_source[i];
            }
        }
    }
    // to handle multidimensional arrays, call recursively
    if (lookAhead() == QStringLiteral("[")) {
        typeNameRet = joinTypename(typeNameRet);
    }
    return typeNameRet;
}

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
 * @param word   whitespace delimited item
 */
void CsValaImportBase::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        const QChar& c = word[i];
        if (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.')) {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme.clear();
            }
            m_source.append(QString(c));
        }
    }
    if (!lexeme.isEmpty())
        m_source.append(lexeme);

    // Condense single dimension array into the type name as done for the
    // predefined types in the CSharpWriter and ValaWriter code generators.
    const int last = m_source.size() - 1;
    if (last > 1 && m_source.at(last-1) == QStringLiteral("[")
                   && m_source.at(last) == QStringLiteral("]")) {
        m_source.removeLast();
        m_source.removeLast();
        m_source.last() += QStringLiteral("[]");
    }
}

/**
 * Spawn off an import of the specified file.
 * @param file   the specified file
 */
void CsValaImportBase::spawnImport(const QString& file)
{
    // if the file is being parsed, don't bother
    if (s_filesAlreadyParsed.contains(file)) {
        return;
    }
    if (QFile::exists(file)) {
        CsValaImportBase importer;
        QStringList fileList;
        fileList.append(file);
        s_filesAlreadyParsed.append(file);
        importer.importFiles(fileList);
    }
}

/**
 * Returns the UML Object if found, or null otherwise.
 * @param name        name of the uml object
 * @param parentPkg   parent package
 * @return null or the uml objecct
 */
UMLObject* CsValaImportBase::findObject(const QString& name, UMLPackage *parentPkg)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLObject * o = umldoc->findUMLObject(name, UMLObject::ot_UMLObject, parentPkg);
    return o;
}

/**
 * Try to resolve the specified class the current class depends on.
 * @param className  the name of the class
 */
UMLObject* CsValaImportBase::resolveClass(const QString& className)
{
    UMLObject *existing = findObject(className, currentScope());
    if (existing)
        return existing;
    logDebug1("CsValaImportBase::resolveClass trying to resolve %1", className);
    // keep track if we are dealing with an array
    bool isArray = className.contains(QLatin1Char('['));
    // remove any [] so that the class itself can be resolved
    QString baseClassName = className;
    baseClassName.remove(QLatin1Char('['));
    baseClassName.remove(QLatin1Char(']'));

    // java has a few implicit imports.  Most relevant for this is the
    // current package, which is in the same directory as the current file
    // being parsed
    QStringList file = m_currentFileName.split(QLatin1Char('/'));
    // remove the filename.  This leaves the full path to the containing
    // dir which should also include the package hierarchy
    file.pop_back();

    // the file we're looking for might be in the same directory as the
    // current class
    QString myDir = file.join(QStringLiteral("/"));
    QString myFile = myDir + QLatin1Char('/') + baseClassName + fileExtension();
    if (QFile::exists(myFile)) {
        spawnImport(myFile);
        if (isArray) {
            // we have imported the type. For arrays we want to return
            // the array type
            return Import_Utils::createUMLObject(UMLObject::ot_Class, className, currentScope());
        }
        return findObject(baseClassName, currentScope());
    }

    // the class we want is not in the same package as the one being imported.
    // use the imports to find the one we want.
    QStringList package = m_currentPackage.split(QLatin1Char('.'));
    int dirsInPackageCount = package.size();

    for (int count = 0; count < dirsInPackageCount; ++count) {
        // pop off one by one the directories, until only the source root remains
        file.pop_back();
    }
    // this is now the root of any further source imports
    QString sourceRoot = file.join(QStringLiteral("/")) + QLatin1Char('/');

    for (QStringList::Iterator pathIt = m_imports.begin();
            pathIt != m_imports.end(); ++pathIt) {
        QString import = (*pathIt);
        QStringList split = import.split(QLatin1Char('.'));
        split.pop_back(); // remove the * or the classname
        if (import.endsWith(QLatin1Char('*')) || import.endsWith(baseClassName)) {
            // check if the file we want is in this imported package
            // convert the org.test type package into a filename
            QString aFile = sourceRoot + split.join(QStringLiteral("/")) + QLatin1Char('/') + baseClassName + fileExtension();
            if (QFile::exists(aFile)) {
                spawnImport(aFile);
                // we need to set the package for the class that will be resolved
                // start at the root package
                UMLPackage  *parent = nullptr;
                UMLPackage  *current = nullptr;

                for (QStringList::Iterator it = split.begin(); it != split.end(); ++it) {
                    QString name = (*it);
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                    name, parent);
                    current = ns->asUMLPackage();
                    parent = current;
                } // for
                if (isArray) {
                    // we have imported the type. For arrays we want to return
                    // the array type
                    return Import_Utils::createUMLObject(UMLObject::ot_Class, className, current);
                }
                // now that we have the right package, the class should be findable
                return findObject(baseClassName, current);
            } // if file exists
        } // if import matches
    } //foreach import
    return 0; // no match
}

/**
 * Keep track of the current file being parsed and reset the list of imports.
 * @param filename   the name of the file being parsed
 */
bool CsValaImportBase::parseFile(const QString& filename)
{
    logDebug1("CsValaImportBase::parseFile %1", filename);
    m_currentFileName = filename;
    m_imports.clear();
    // default visibility is Impl, unless we are an interface, then it is
    // public for member vars and methods
    m_defaultCurrentAccess = Uml::Visibility::Implementation;
    m_currentAccess = m_defaultCurrentAccess;
    s_parseDepth++;
    // in the case of self referencing types, we can avoid parsing the
    // file twice by adding it to the list
    s_filesAlreadyParsed.append(filename);
    NativeImportBase::parseFile(filename);
    s_parseDepth--;
    if (s_parseDepth <= 0) {
        // if the user decides to clear things out and reparse, we need
        // to honor the request, so reset things for next time.
        s_filesAlreadyParsed.clear();
        s_parseDepth = 0;
    }
    return true;
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
        return parseNamespaceMemberDeclarations();
    }

    if (keyword == QStringLiteral("[")) {
        return parseAttributes();  //:TODO: more than one
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
    if (m_klass == 0) {
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
bool CsValaImportBase::parseNamespaceMemberDeclarations()
{
    QString m_currentNamespace = advance();
    log(QStringLiteral("namespace ") + m_currentNamespace);
    // move past {
    skipStmt(QStringLiteral("{"));
    return true;
}

/**
 * Parsing attributes.
 * @return   success status of parsing
 */
bool CsValaImportBase::parseAttributes()
{
    QString attribute = advance();
    log(QStringLiteral("attribute ") + attribute);
    skipStmt(QStringLiteral("]"));
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
    if (enumType == 0)
        enumType = Import_Utils::remapUMLEnum(ns, currentScope());
    skipStmt(QStringLiteral("{"));
    while (m_srcIndex < m_source.count() - 1 && advance() != QStringLiteral("}")) {
        QString next = advance();
        if (next == QStringLiteral("=")) {
            next = advance();
            if (enumType != 0)
                Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex - 2], QString(), m_source[m_srcIndex]);
            next = advance();
        } else {
            if (enumType != 0)
                Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex - 1]);
        }
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
