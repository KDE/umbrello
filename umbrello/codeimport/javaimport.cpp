/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "javaimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "codeimpthread.h"
#define DBG_SRC QStringLiteral("JavaImport")
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "object_factory.h"
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

DEBUG_REGISTER_DISABLED(JavaImport)

QStringList JavaImport::s_filesAlreadyParsed;
int JavaImport::s_parseDepth = 0;

/**
 * Constructor.
 */
JavaImport::JavaImport(CodeImpThread* thread)
  : NativeImportBase(QStringLiteral("//"), thread),
    m_defaultCurrentAccess(Uml::Visibility::Public)
{
    setMultiLineComment(QStringLiteral("/*"), QStringLiteral("*/"));
    initVars();
}

/**
 * Destructor.
 */
JavaImport::~JavaImport()
{
}

/**
 * Reimplement operation from NativeImportBase.
 */
void JavaImport::initVars()
{
    m_isStatic = false;
}

/**
 * Figure out if the type is really an array or template of the given typeName.
 * Catenate possible template arguments/array dimensions to the end of the type name.
 * @param typeName   the type name
 * @return the type name with the additional information
 */
QString JavaImport::joinTypename(const QString& typeName)
{
    QString typeNameRet(typeName);
    if (m_srcIndex + 1 < m_source.size()) {
        if (m_source[m_srcIndex + 1] == QStringLiteral("<") ||
            m_source[m_srcIndex + 1] == QStringLiteral("[")) {
            int start = ++m_srcIndex;
            if (! skipToClosing(m_source[start][0]))
                return typeNameRet;
            for (int i = start; i <= m_srcIndex; ++i) {
                typeNameRet += m_source[i];
            }
        }
    }
    // to handle multidimensional arrays, call recursively
    if ((m_srcIndex + 1 < m_source.size()) && (m_source[m_srcIndex + 1] == QStringLiteral("["))) {
        typeNameRet = joinTypename(typeNameRet);
    }
    return typeNameRet;
}

/**
 * Implement abstract operation from NativeImportBase.
 * @param word   whitespace delimited item
 */
void JavaImport::fillSource(const QString& word)
{
    QString w(word);
    /* In Java, method varargs are represented by three dots following the
       type name.  For portability to other C family languages, we replace
       the "..." by "[]".
     */
    if (w.contains(QStringLiteral("..."))) {
        w.replace(QStringLiteral("..."), QStringLiteral("[]"));
    }
    QString lexeme;
    const uint len = w.length();
    for (uint i = 0; i < len; ++i) {
        const QChar& c = w[i];
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
void JavaImport::spawnImport(const QString& file)
{
    // if the file is being parsed, don't bother
    //
    if (s_filesAlreadyParsed.contains(file)) {
        return;
    }
    if (QFile::exists(file)) {
          JavaImport importer;
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
UMLObject* JavaImport::findObject(const QString& name, UMLPackage *parentPkg)
{
    UMLDoc *umldoc = UMLApp::app()->document();
    UMLObject * o = umldoc->findUMLObject(name, UMLObject::ot_UMLObject, parentPkg);
    return o;
}

/**
 * Try to resolve the specified class the current class depends on.
 * @param className  the name of the class
 */
UMLObject* JavaImport::resolveClass (const QString& className)
{
    logDebug1("importJava trying to resolve %1", className);
    // keep track if we are dealing with an array
    //
    bool isArray = className.contains(QLatin1Char('['));
    // remove any [] so that the class itself can be resolved
    //
    QString baseClassName = className;
    baseClassName.remove(QLatin1Char('['));
    baseClassName.remove(QLatin1Char(']'));

    // remove template class name so that the class itself can be resolved
    int index = baseClassName.indexOf(QLatin1Char('<'));
    if (index != -1) {
        baseClassName = baseClassName.remove(index, baseClassName.size()-index);
    }

    // java has a few implicit imports.  Most relevant for this is the
    // current package, which is in the same directory as the current file
    // being parsed
    //
    QStringList file = m_currentFileName.split(QLatin1Char('/'));
    // remove the filename.  This leaves the full path to the containing
    // dir which should also include the package hierarchy
    //
    file.pop_back();

    // the file we're looking for might be in the same directory as the
    // current class
    //
    QString myDir = file.join(QStringLiteral("/"));
    QString myFile = myDir + QLatin1Char('/') + baseClassName + QStringLiteral(".java");
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
    //
    QStringList package = m_currentPackage.split(QLatin1Char('.'));
    int dirsInPackageCount = package.size();

    // in case the path does not fit into the package hierarchy 
    // we cannot check the imports 
    if (dirsInPackageCount >= file.size())
        return nullptr;

    for (int count=0; count < dirsInPackageCount; ++count) {
        // pop off one by one the directories, until only the source root remains
        //
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
            //
            QString aFile = sourceRoot + split.join(QStringLiteral("/")) + QLatin1Char('/') + baseClassName + QStringLiteral(".java");
            if (QFile::exists(aFile)) {
                spawnImport(aFile);
                // we need to set the package for the class that will be resolved
                // start at the root package
                UMLPackage  *parent = nullptr;
                UMLPackage  *current = nullptr;

                for (QStringList::Iterator it = split.begin(); it != split.end(); ++it) {
                    QString name = (*it);
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                                                  name, parent,
                                                                  QString(), QString(),
                                                                  true, false);
                    current = ns->asUMLPackage();
                    parent = current;
                } // for
                if (isArray) {
                    // we have imported the type. For arrays we want to return
                    // the array type
                    return Import_Utils::createUMLObject(UMLObject::ot_Class, className, current,
                                                         QString(), QString(), true, false);
                }
                // now that we have the right package, the class should be findable
                return findObject(baseClassName, current);
            // imported class is specified but seems to be external
            } else if (import.endsWith(baseClassName)) {
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
                return Import_Utils::createUMLObject(UMLObject::ot_Class,
                                                              baseClassName, current);
            } // if file exists
        } // if import matches
    } //foreach import
    return nullptr; // no match
}

/**
 * Keep track of the current file being parsed and reset the list of imports.
 * @param filename   the name of the file being parsed
 */
bool JavaImport::parseFile(const QString& filename)
{
    m_currentFileName = filename;
    m_imports.clear();
    // default visibility is Impl, unless we are an interface, then it is
    // public for member vars and methods
    m_defaultCurrentAccess = Uml::Visibility::Implementation;
    m_currentAccess = m_defaultCurrentAccess;
    s_parseDepth++;
    // in the case of self referencing types, we can avoid parsing the
    // file twice by adding it to the list
    if (s_filesAlreadyParsed.contains(filename)) {
        s_parseDepth--;
        return true;
    }

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
 * @return success status of operation
 */
bool JavaImport::parseStmt()
{
    const int srcLength = m_source.count();
    QString keyword = m_source[m_srcIndex];
    //uDebug() << '"' << keyword << '"';
    if (keyword == QStringLiteral("package")) {
        m_currentPackage = advance();
        const QString& qualifiedName = m_currentPackage;
        const QStringList names = qualifiedName.split(QLatin1Char('.'));
        for (QStringList::ConstIterator it = names.begin(); it != names.end(); ++it) {
            QString name = (*it);
            log(keyword + QLatin1Char(' ') + name);
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                            name, currentScope(), m_comment, QString(), true);
            pushScope(ns->asUMLPackage());
        }
        if (advance() != QStringLiteral(";")) {
            logError1("JavaImport::parseStmt: unexpected: %1", m_source[m_srcIndex]);
            skipStmt();
        }
        return true;
    }
    if (keyword == QStringLiteral("class") || keyword == QStringLiteral("interface")) {
        const QString& name = advance();
        const UMLObject::ObjectType ot = (keyword == QStringLiteral("class") ? UMLObject::ot_Class
                                                                            : UMLObject::ot_Interface);
        log(keyword + QLatin1Char(' ') + name);
        UMLObject *ns = Import_Utils::createUMLObject(ot, name, currentScope(), m_comment);
        m_klass = ns->asUMLClassifier();
        // The name is already used but is package
        if (ns && ns->isUMLPackage() && !m_klass) {
            ns = Object_Factory::createNewUMLObject(ot, name, ns->asUMLPackage());
            ns->setDoc(m_comment);
            m_klass = ns->asUMLClassifier();
            m_klass->setUMLPackage(currentScope());
            currentScope()->addObject(m_klass, false);  // false => non interactively
        }
        pushScope(m_klass);
        m_klass->setStatic(m_isStatic);
        m_klass->setVisibilityCmd(m_currentAccess);
        // The UMLObject found by createUMLObject might originally have been created as a
        // placeholder with a type of class but if is really an interface then we need to
        // change it.
        m_klass->setBaseType(ot);
        // TODO: UMLClassifier::setBaseType() resets abstract flag
        m_klass->setAbstract(m_isAbstract);
        m_isAbstract = m_isStatic = false;
        // if no modifier is specified in an interface then it means public
        if (m_klass->isInterface()) {
            m_defaultCurrentAccess =  Uml::Visibility::Public;
        }
        if (advance() == QStringLiteral(";"))   // forward declaration
            return true;
        if (m_source[m_srcIndex] == QStringLiteral("<")) {
            // template args - preliminary, rudimentary implementation
            // @todo implement all template arg syntax
            uint start = m_srcIndex;
            if (! skipToClosing(QLatin1Char('<'))) {
                logError1("JavaImport::parseStmt(%1): template syntax error", name);
                return false;
            }
            while (1) {
                const QString arg = m_source[++start];
                if (! arg.contains(QRegularExpression(QStringLiteral("^[A-Za-z_]")))) {
                    logDebug2("JavaImport::parseStmt(%1): cannot handle template syntax (%2)",
                              name, arg);
                    break;
                }
                /* UMLTemplate *tmpl = */ m_klass->addTemplate(arg);
                const QString next = m_source[++start];
                if (next == QStringLiteral(">"))
                    break;
                if (next != QStringLiteral(",")) {
                    logDebug2("JavaImport::parseStmt(%1): cannot handle template syntax (%2)",
                              name, next);
                    break;
                }
            }
            advance();  // skip over ">"
        }
        if (m_source[m_srcIndex] == QStringLiteral("extends")) {
            const QString& baseName = advance();
            // try to resolve the class we are extending, or if impossible
            // create a placeholder
            UMLObject *parent = resolveClass(baseName);
            if (parent) {
                Import_Utils::createGeneralization(m_klass, parent->asUMLClassifier());
            } else {
                logDebug1("importJava parentClass %1 is not resolveable. Creating placeholder",
                          baseName);
                Import_Utils::createGeneralization(m_klass, baseName);
            }
            advance();
        }
        if (m_source[m_srcIndex] == QStringLiteral("implements")) {
            while (m_srcIndex < srcLength - 1 && advance() != QStringLiteral("{")) {
                const QString& baseName = m_source[m_srcIndex];
                // try to resolve the interface we are implementing, if this fails
                // create a placeholder
                UMLObject *interface = resolveClass(baseName);
                if (interface) {
                     Import_Utils::createGeneralization(m_klass, interface->asUMLClassifier());
                } else {
                    logDebug1("importJava implementing interface %1 is not resolvable. "
                              "Creating placeholder", baseName);
                    Import_Utils::createGeneralization(m_klass, baseName);
                }
                if (advance() != QStringLiteral(","))
                    break;
            }
        }
        if (m_source[m_srcIndex] != QStringLiteral("{")) {
            logError2("JavaImport::parseStmt: ignoring excess chars at %1 (%2)", name, m_source[m_srcIndex]);
            skipStmt(QStringLiteral("{"));
        }
        return true;
    }
    if (keyword == QStringLiteral("enum")) {
        const QString& name = advance();
        log(keyword + QLatin1Char(' ') + name);
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                        name, currentScope(), m_comment);
        UMLEnum *enumType = ns->asUMLEnum();
        if (enumType == nullptr)
            enumType = Import_Utils::remapUMLEnum(ns, currentScope());
        skipStmt(QStringLiteral("{"));
        while (m_srcIndex < srcLength - 1 && advance() != QStringLiteral("}")) {
            if (enumType != nullptr)
                Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
            QString next = advance();
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
    if (keyword == QStringLiteral("static")) {
        m_isStatic = true;
        return true;
    }
    // if we detected static previously and keyword is { then this is a static block
    if (m_isStatic && keyword == QStringLiteral("{")) {
        // reset static flag and jump to end of static block
        m_isStatic = false;
        return skipToClosing(QLatin1Char('{'));
    }
    if (keyword == QStringLiteral("abstract")) {
        m_isAbstract = true;
        return true;
    }
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
    if (keyword == QStringLiteral("final") ||
        keyword == QStringLiteral("native") ||
        keyword == QStringLiteral("synchronized") ||
        keyword == QStringLiteral("transient") ||
        keyword == QStringLiteral("volatile")) {
        //@todo anything to do here?
        return true;
    }
    if (keyword == QStringLiteral("import")) {
        // keep track of imports so we can resolve classes we are dependent on
        QString import = advance();
        if (import.endsWith(QLatin1Char('.'))) {
            //this most likely an import that ends with a *
            //
            import = import + advance();
        }
        m_imports.append(import);

        // move past ;
        skipStmt();
        return true;
    }
    if (keyword == QStringLiteral("@")) {  // annotation
        advance();
        if (m_source[m_srcIndex + 1] == QStringLiteral("(")) {
            advance();
            skipToClosing(QLatin1Char('('));
        }
        return true;
    }
    if (keyword == QStringLiteral("}")) {
        if (scopeIndex()) {
            m_klass = popScope()->asUMLClassifier();
        }
        else
            logError1("JavaImport::parseStmt: too many } at index %1", m_srcIndex);
        return true;
    }
    if (keyword == QStringLiteral("<")) {  // @todo generic parameters
        if (! skipToClosing(QLatin1Char('<'))) {
            logError1("JavaImport::parseStmt(%1): template syntax error", keyword);
            return false;
        }
        advance();
        if (m_srcIndex == srcLength)
            return false;
        keyword = m_source[m_srcIndex];
    }
    // At this point, we expect `keyword` to be a type name
    // (of a member of class or interface, or return type
    // of an operation.) Up next is the name of the attribute
    // or operation.
    if (! keyword.contains(QRegularExpression(QStringLiteral("^\\w")))) {
        if (m_klass) {
            logError4("JavaImport::parseStmt: ignoring %1 at index %2 of %3 in %4",
                      keyword, m_srcIndex, m_source.count(), m_klass->name());
        } else {
            logError3("JavaImport::parseStmt: ignoring %1 at index %2 of %3 (outside class)",
                      keyword, m_srcIndex, m_source.count());
        }
        return false;
    }
    QString typeName = m_source[m_srcIndex];
    typeName = joinTypename(typeName);
    // At this point we need a class.
    if (m_klass == nullptr) {
        logError1("JavaImport::parseStmt: no class set for %1", typeName);
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
        logError1("JavaImport::parseStmt: expecting name in %1", name);
        return false;
    }
    if (nextToken == QStringLiteral("(")) {
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        m_srcIndex++;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != QStringLiteral(")")) {
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
        if (!typeName.isEmpty() && typeName != QStringLiteral("void")) {
            // before adding the method, try resolving the return type
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
    // At this point we know it's some kind of attribute declaration.
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
            logError1("JavaImport::parseStmt: ignoring trailing items at %1", name);
            skipStmt();
        }
    }
    else {
        logError1("JavaImport::parseStmt index out of range: ignoring statement %1", name);
        skipStmt();
    }
    return true;
}
