/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2011-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "csharpimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "codeimpthread.h"
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
#include <QRegExp>
#include <QStringList>
#include <QTextStream>

QStringList CSharpImport::s_filesAlreadyParsed;
int CSharpImport::s_parseDepth = 0;

/**
 * Constructor.
 */
CSharpImport::CSharpImport(CodeImpThread* thread)
  : NativeImportBase(QLatin1String("//"), thread),
    m_defaultCurrentAccess(Uml::Visibility::Public)
{
    setMultiLineComment(QLatin1String("/*"), QLatin1String("*/"));
    initVars();
}

/**
 * Destructor.
 */
CSharpImport::~CSharpImport()
{
}

/**
 * Reimplement operation from NativeImportBase.
 */
void CSharpImport::initVars()
{
    m_isStatic = false;
}

/**
 * Figure out if the type is really an array or template of the given typeName.
 * Catenate possible template arguments/array dimensions to the end of the type name.
 * @param typeName   the type name
 * @return the type name with the additional information
 */
QString CSharpImport::joinTypename(const QString& typeName)
{
    QString typeNameRet(typeName);
    if (m_srcIndex + 1 < m_source.size()) {
        if (m_source[m_srcIndex + 1] == QLatin1String("<") ||
            m_source[m_srcIndex + 1] == QLatin1String("[")) {
            int start = ++m_srcIndex;
            if (! skipToClosing(m_source[start][0]))
                return typeNameRet;
            for (int i = start; i <= m_srcIndex; ++i) {
                typeNameRet += m_source[i];
            }
        }
    }
    // to handle multidimensional arrays, call recursively
    if ((m_srcIndex + 1 < m_source.size()) && (m_source[m_srcIndex + 1] == QLatin1String("["))) {
        typeNameRet = joinTypename(typeNameRet);
    }
    return typeNameRet;
}

/**
 * Implement abstract operation from NativeImportBase.
 * @param word   whitespace delimited item
 */
void CSharpImport::fillSource(const QString& word)
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
}

/**
 * Spawn off an import of the specified file.
 * @param file   the specified file
 */
void CSharpImport::spawnImport(const QString& file)
{
    // if the file is being parsed, don't bother
    if (s_filesAlreadyParsed.contains(file)) {
        return;
    }
    if (QFile::exists(file)) {
        CSharpImport importer;
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
UMLObject* CSharpImport::findObject(const QString& name, UMLPackage *parentPkg)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLObject * o = umldoc->findUMLObject(name, UMLObject::ot_UMLObject, parentPkg);
    return o;
}

/**
 * Try to resolve the specified class the current class depends on.
 * @param className  the name of the class
 */
UMLObject* CSharpImport::resolveClass(const QString& className)
{
    uDebug() << "trying to resolve " << className;
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
    QString myDir = file.join(QLatin1String("/"));
    QString myFile = myDir + QLatin1Char('/') + baseClassName + QLatin1String(".cs");
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
    QString sourceRoot = file.join(QLatin1String("/")) + QLatin1Char('/');

    for (QStringList::Iterator pathIt = m_imports.begin();
            pathIt != m_imports.end(); ++pathIt) {
        QString import = (*pathIt);
        QStringList split = import.split(QLatin1Char('.'));
        split.pop_back(); // remove the * or the classname
        if (import.endsWith(QLatin1Char('*')) || import.endsWith(baseClassName)) {
            // check if the file we want is in this imported package
            // convert the org.test type package into a filename
            QString aFile = sourceRoot + split.join(QLatin1String("/")) + QLatin1Char('/') + baseClassName + QLatin1String(".cs");
            if (QFile::exists(aFile)) {
                spawnImport(aFile);
                // we need to set the package for the class that will be resolved
                // start at the root package
                UMLPackage *parent = 0;
                UMLPackage *current = 0;

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
bool CSharpImport::parseFile(const QString& filename)
{
    uDebug() << filename;
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
bool CSharpImport::parseStmt()
{
    const QString& keyword = m_source[m_srcIndex];
    //uDebug() << '"' << keyword << '"';

    if (keyword == QLatin1String("using")) {
        return parseUsingDirectives();
    }

    if (keyword == QLatin1String("namespace")) {
        return parseNamespaceMemberDeclarations();
    }

    if (keyword == QLatin1String("[")) {
        return parseAttributes();  //:TODO: more than one
    }

    // type-declaration - class, interface, struct, enum, delegate
    if (isClassModifier(keyword) || isTypeDeclaration(keyword)) {
        // more than one modifier possible
        QString nextKeyword = keyword;
        while (isClassModifier(nextKeyword)) {
            nextKeyword = advance();
        }
        if (nextKeyword == QLatin1String("class")) {
            return parseClassDeclaration(nextKeyword);
        }
        if (nextKeyword == QLatin1String("interface")) {
            return parseClassDeclaration(nextKeyword);
        }
        if (nextKeyword == QLatin1String("struct")) {
            return parseStructDeclaration();
        }
        if (nextKeyword == QLatin1String("enum")) {
            return parseEnumDeclaration();
        }
        if (nextKeyword == QLatin1String("delegate")) {
            return parseDelegateDeclaration();
        }
    }


/*
    if (keyword == QLatin1String("static")) {
        m_isStatic = true;
        return true;
    }

    // if we detected static previously and keyword is { then this is a static block
    if (m_isStatic && keyword == QLatin1String("{")) {
        // reset static flag and jump to end of static block
        m_isStatic = false;
        return skipToClosing(QLatin1Char('{'));
    }

    if ((keyword == QLatin1String("override")) ||
            (keyword == QLatin1String("virtual")) ||
            (keyword == QLatin1String("sealed"))) {
        //:TODO: anything to do here?
        return true;
    }
*/

    if (keyword == QLatin1String("#")) {   // preprocessor directives
        QString ppdKeyword = advance();
        uDebug() << "found preprocessor directive " << ppdKeyword;
        //:TODO: anything to do here?
        return true;
    }

//     if (keyword == QLatin1String("@")) {   // annotation
//         advance();
//         if (m_source[m_srcIndex + 1] == QLatin1String("(")) {
//             advance();
//             skipToClosing(QLatin1Char('('));
//         }
//         return true;
//     }

    if (keyword == QLatin1String("[")) {   // ...
        advance();
        skipToClosing(QLatin1Char('['));
        return true;
    }

    if (keyword == QLatin1String("}")) {
        if (scopeIndex())
            m_klass = popScope()->asUMLClassifier();
        else
            logError0("CSharpImport::parseStmt: too many }");
        return true;
    }

    // At this point, we expect `keyword' to be a type name
    // (of a member of class or interface, or return type
    // of an operation.) Up next is the name of the attribute
    // or operation.
    if (! keyword.contains(QRegExp(QLatin1String("^\\w")))) {
        if (m_klass)
            logError4("CSharpImport::parseStmt: ignoring keyword %1 at index %2 of %3 (%4)",
                      keyword, m_srcIndex, m_source.count(), m_klass->name());
        else
            logError3("CSharpImport::parseStmt: ignoring keyword %1 at index %2 of %3",
                      keyword, m_srcIndex, m_source.count());
        return false;
    }

    QString typeName = m_source[m_srcIndex];
    typeName = joinTypename(typeName);
    // At this point we need a class.
    if (m_klass == 0) {
        logError1("CSharpImport::parseStmt: no class set for %1", typeName);
        return false;
    }
    QString name = advance();
    QString nextToken;
    if (typeName == m_klass->name() && name == QLatin1String("(")) {
        // Constructor.
        nextToken = name;
        name = typeName;
        typeName.clear();
    } else {
        nextToken = advance();
    }
    if (name.contains(QRegExp(QLatin1String("\\W")))) {
        logError1("CSharpImport::parseStmt: expecting name at %1", name);
        return false;
    }
    if (nextToken == QLatin1String("(")) {
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        m_srcIndex++;
        while (m_srcIndex < m_source.count() && m_source[m_srcIndex] != QLatin1String(")")) {
            QString typeName = m_source[m_srcIndex];
            if (typeName == QLatin1String("final") || typeName.startsWith(QLatin1String("//"))) {
                // ignore the "final" keyword and any comments in method args
                typeName = advance();
            }
            typeName = joinTypename(typeName);
            QString parName = advance();
            // the Class might not be resolved yet so resolve it if necessary
            UMLObject *obj = resolveClass(typeName);
            if (obj) {
                // by prepending the package, unwanted placeholder types will not get created
                typeName = obj->fullyQualifiedName(QLatin1String("."));
            }
            /* UMLAttribute *att = */ Import_Utils::addMethodParameter(op, typeName, parName);
            if (advance() != QLatin1String(","))
                break;
            m_srcIndex++;
        }
        // before adding the method, try resolving the return type
        UMLObject *obj = resolveClass(typeName);
        if (obj) {
            // using the fully qualified name means that a placeholder type will not be created.
            typeName = obj->fullyQualifiedName(QLatin1String("."));
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
        } while (nextToken != QLatin1String("{") && nextToken != QLatin1String(";"));
        if (nextToken == QLatin1String(";")) {
            // No body (interface or abstract)
            return true;
        } else {
            return skipToClosing(QLatin1Char('{'));
        }
    }
    // At this point we know it's some kind of attribute declaration.
    while(1) {
        while (nextToken != QLatin1String(",") && nextToken != QLatin1String(";")) {
            if (nextToken == QLatin1String("=")) {
                if ((nextToken = advance()) == QLatin1String("new")) {
                    advance();
                    if ((nextToken = advance()) == QLatin1String("(")) {
                        skipToClosing(QLatin1Char('('));
                        if ((nextToken = advance()) == QLatin1String("{")) {
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
                name += nextToken;  // add possible array dimensions to `name'
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
        if (nextToken != QLatin1String(",")) {
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
        if (m_source[m_srcIndex] != QLatin1String(";")) {
            logError1("CSharpImport::parseStmt: ignoring trailing items at %1", name);
            skipStmt();
        }
    } else {
        logError1("CSharpImport::parseStmt index out of range: ignoring statement %1", name);
        skipStmt();
    }
    return true;
}

/**
 * Parsing the statement 'using'.
 * Keep track of imports so we can resolve classes we are dependent on.
 * @return   success status of parsing
 */
bool CSharpImport::parseUsingDirectives()
{
    QString import = advance();
    log(QLatin1String("using ") + import);
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
bool CSharpImport::parseGlobalAttributes()
{
    //:TODO:
    return true;
}

/**
 * Parsing the statement 'namespace'.
 * @return   success status of parsing
 */
bool CSharpImport::parseNamespaceMemberDeclarations()
{
    QString m_currentNamespace = advance();
    log(QLatin1String("namespace ") + m_currentNamespace);
    // move past {
    skipStmt(QLatin1String("{"));
    return true;
}

/**
 * Parsing attributes.
 * @return   success status of parsing
 */
bool CSharpImport::parseAttributes()
{
    QString attribute = advance();
    log(QLatin1String("attribute ") + attribute);
    skipStmt(QLatin1String("]"));
    return true;
}

/**
 * Check if keyword is belonging to a type-declaration.
 * @return   result of check
 */
bool CSharpImport::isTypeDeclaration(const QString& keyword)
{
    if (keyword == QLatin1String("class")     ||
        keyword == QLatin1String("struct")    ||
        keyword == QLatin1String("interface") ||
        keyword == QLatin1String("enum")      ||
        keyword == QLatin1String("delegate")) {
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
bool CSharpImport::isClassModifier(const QString& keyword)
{
    if (isCommonModifier(keyword)  ||
        keyword == QLatin1String("abstract")    ||
        keyword == QLatin1String("sealed")) {
        // log("class-modifier: " + keyword);
        if (keyword == QLatin1String("abstract")) {
            m_isAbstract = true;
        }
        return true;
    }
    else {
        return false;
    }
}

/**
 * Check if keyword is an interface, struct, enum or delegate modifier.
 * @return   result of check
 */
bool CSharpImport::isCommonModifier(const QString& keyword)
{
    if (keyword == QLatin1String("new")       ||
        keyword == QLatin1String("public")    ||
        keyword == QLatin1String("protected") ||
        keyword == QLatin1String("internal")  ||
        keyword == QLatin1String("private")) {
        if (keyword == QLatin1String("public")) {
            m_currentAccess = Uml::Visibility::Public;
        }
        if (keyword == QLatin1String("protected")) {
            m_currentAccess = Uml::Visibility::Protected;
        }
        if (keyword == QLatin1String("private")) {
            m_currentAccess = Uml::Visibility::Private;
        }
        return true;
    }
    else {
        return false;
    }
}

/**
 * Parsing the statement 'enum'.
 * @return   success status of parsing
 */
bool CSharpImport::parseEnumDeclaration()
{
    const QString& name = advance();
    log(QLatin1String("enum ") + name);
    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                        name, currentScope(), m_comment);
    UMLEnum *enumType = ns->asUMLEnum();
    if (enumType == 0)
        enumType = Import_Utils::remapUMLEnum(ns, currentScope());
    skipStmt(QLatin1String("{"));
    while (m_srcIndex < m_source.count() - 1 && advance() != QLatin1String("}")) {
        QString next = advance();
        if (next == QLatin1String("=")) {
            next = advance();
            if (enumType != 0)
                Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex - 2], QString(), m_source[m_srcIndex]);
            next = advance();
        } else {
            if (enumType != 0)
                Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex - 1]);
        }
        if (next == QLatin1String("{") || next == QLatin1String("(")) {
            if (! skipToClosing(next[0]))
                return false;
            next = advance();
        }
        if (next != QLatin1String(",")) {
            if (next == QLatin1String(";")) {
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
bool CSharpImport::parseStructDeclaration()
{
    const QString& name = advance();
    log(QLatin1String("struct ") + name + QLatin1String(" --> parsing not yet implemented!"));
    return true;
}

/**
 * Parsing a delegate-declaration.
 * @return   success status of parsing
 */
bool CSharpImport::parseDelegateDeclaration()
{
    // return-type identifier (formal-parameter-list?) ;
    const QString& returnType = advance();
    const QString& name = advance();
    log(QLatin1String("delegate ") + name + QLatin1String("with return-type ") + returnType);
    skipStmt(QLatin1String(";"));
    return true;
}

/**
 * Parsing the statement 'class' or 'interface'.
 * @return   success status of parsing
 */
bool CSharpImport::parseClassDeclaration(const QString& keyword)
{
    const QString& name = advance();
    const UMLObject::ObjectType ot = (keyword == QLatin1String("class") ? UMLObject::ot_Class
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

    if (advance() == QLatin1String(";"))    // forward declaration
        return true;

    if (m_source[m_srcIndex] == QLatin1String("<")) {
        // template args - preliminary, rudimentary implementation
        // @todo implement all template arg syntax
        uint start = m_srcIndex;
        if (! skipToClosing(QLatin1Char('<'))) {
            logError1("CSharpImport::parseClassDeclaration (%1): template syntax error", name);
            return false;
        }
        while(1) {
            const QString arg = m_source[++start];
            if (! arg.contains(QRegExp(QLatin1String("^[A-Za-z_]")))) {
                uDebug() << "import C# (" << name << "): "
                         << "cannot handle template syntax (" << arg << ")";
                break;
            }
            /* UMLTemplate *tmpl = */ m_klass->addTemplate(arg);
            const QString next = m_source[++start];
            if (next == QLatin1String(">"))
                break;
            if (next != QLatin1String(",")) {
                uDebug() << "import C# (" << name << "): "
                         << "cannot handle template syntax (" << next << ")";
                break;
            }
        }
        advance();  // skip over ">"
    }

    if (m_source[m_srcIndex] == QLatin1String(":")) {   // derivation
        while (m_srcIndex < m_source.count() - 1 && advance() != QLatin1String("{")) {
            const QString& baseName = m_source[m_srcIndex];
            // try to resolve the interface we are implementing, if this fails
            // create a placeholder
            UMLObject *interface = resolveClass(baseName);
            if (interface) {
                Import_Utils::createGeneralization(m_klass, interface->asUMLClassifier());
            } else {
                uDebug() << "implementing interface " << baseName
                         << " is not resolvable. Creating placeholder";
                Import_Utils::createGeneralization(m_klass, baseName);
            }
            if (advance() != QLatin1String(","))
                break;
        }
    }

    if (m_source[m_srcIndex] != QLatin1String("{")) {
        logError2("CSharpImport::parseClassDeclaration ignoring excess chars at %1 (index %2)",
                  name, m_source[m_srcIndex]);
        skipStmt(QLatin1String("{"));
    }
    return true;
}
