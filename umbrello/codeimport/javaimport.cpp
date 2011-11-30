/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "javaimport.h"

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
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>

QStringList JavaImport::s_filesAlreadyParsed;
int JavaImport::s_parseDepth = 0;

/**
 * Constructor.
 */
JavaImport::JavaImport(CodeImpThread* thread)
  : NativeImportBase("//", thread)
{
    setMultiLineComment("/*", "*/");
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
        if (m_source[m_srcIndex + 1] == "<" ||
            m_source[m_srcIndex + 1] == "[") {
            int start = ++m_srcIndex;
            if (! skipToClosing(m_source[start][0]))
                return typeNameRet;
            for (int i = start; i <= m_srcIndex; ++i) {
                typeNameRet += m_source[i];
            }
        }
    }
    // to handle multidimensional arrays, call recursively
    if ((m_srcIndex + 1 < m_source.size()) && (m_source[m_srcIndex + 1] == "[")) {
        typeNameRet = joinTypename( typeNameRet );
    }
    return typeNameRet;
}

/**
 * Implement abstract operation from NativeImportBase.
 * @param word   whitespace delimited item
 */
void JavaImport::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        const QChar& c = word[i];
        if (c.isLetterOrNumber() || c == '_' || c == '.') {
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
void JavaImport::spawnImport(const QString& file )
{
    // if the file is being parsed, don't bother
    //
    if (s_filesAlreadyParsed.contains( file ) ) {
        return;
    }
    if (QFile::exists(file)) {
          JavaImport importer;
          QStringList fileList;
          fileList.append( file );
          s_filesAlreadyParsed.append( file );
          importer.importFiles( fileList );
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
    UMLObject * o = umldoc->findUMLObject(name, UMLObject::ot_UMLObject , parentPkg);
    return o;
}

/**
 * Try to resolve the specified class the current class depends on.
 * @param className  the name of the class
 */
UMLObject* JavaImport::resolveClass (const QString& className)
{
    uDebug() << "importJava trying to resolve " << className;
    // keep track if we are dealing with an array
    //
    bool isArray = className.contains('[');
    // remove any [] so that the class itself can be resolved
    //
    QString baseClassName = className;
    baseClassName.remove('[');
    baseClassName.remove(']');

    // java has a few implicit imports.  Most relevant for this is the
    // current package, which is in the same directory as the current file
    // being parsed
    //
    QStringList file = m_currentFileName.split( '/' );
    // remove the filename.  This leaves the full path to the containing
    // dir which should also include the package hierarchy
    //
    file.pop_back();

    // the file we're looking for might be in the same directory as the
    // current class
    //
    QString myDir = file.join( "/" );
    QString myFile = myDir + '/' + baseClassName + ".java";
    if ( QFile::exists(myFile) ) {
        spawnImport( myFile );
        if ( isArray ) {
            // we have imported the type. For arrays we want to return
            // the array type
            return Import_Utils::createUMLObject(UMLObject::ot_Class, className, m_scope[m_scopeIndex]);
        }
        return findObject(baseClassName, m_scope[m_scopeIndex]);
    }

    // the class we want is not in the same package as the one being imported.
    // use the imports to find the one we want.
    //
    QStringList package = m_currentPackage.split( '.' );
    int dirsInPackageCount = package.size();

    // in case the path does not fit into the package hierachy 
    // we cannot check the imports 
    if (dirsInPackageCount >= file.size())
        return NULL; 

    for (int count=0; count < dirsInPackageCount; ++count ) {
        // pop off one by one the directories, until only the source root remains
        //
        file.pop_back();
    }
    // this is now the root of any further source imports
    QString sourceRoot = file.join("/") + '/';

    for (QStringList::Iterator pathIt = m_imports.begin();
                                   pathIt != m_imports.end(); ++pathIt) {
        QString import = (*pathIt);
        QStringList split = import.split( '.' );
        split.pop_back(); // remove the * or the classname
        if ( import.endsWith( '*' ) || import.endsWith( baseClassName) ) {
            // check if the file we want is in this imported package
            // convert the org.test type package into a filename
            //
            QString aFile = sourceRoot + split.join("/") + '/' + baseClassName + ".java";
            if ( QFile::exists(aFile) ) {
                spawnImport( aFile );
                // we need to set the package for the class that will be resolved
                // start at the root package
                UMLPackage *parent = m_scope[0];
                UMLPackage *current = NULL;

                for (QStringList::Iterator it = split.begin(); it != split.end(); ++it) {
                    QString name = (*it);
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                                                  name, parent);
                    current = static_cast<UMLPackage*>(ns);
                    parent = current;
                } // for
                if ( isArray ) {
                    // we have imported the type. For arrays we want to return
                    // the array type
                    return Import_Utils::createUMLObject(UMLObject::ot_Class, className, current);
                }
                // now that we have the right package, the class should be findable
                return findObject(baseClassName, current);
            } // if file exists
        } // if import matches
    } //foreach import
    return NULL; // no match
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
    s_filesAlreadyParsed.append(filename);
    NativeImportBase::parseFile(filename);
    s_parseDepth--;
    if ( s_parseDepth <= 0 ) {
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
    const QString& keyword = m_source[m_srcIndex];
    //uDebug() << '"' << keyword << '"';
    if (keyword == "package") {
        m_currentPackage = advance();
        const QString& qualifiedName = m_currentPackage;
        const QStringList names = qualifiedName.split('.');
        for (QStringList::ConstIterator it = names.begin(); it != names.end(); ++it) {
            QString name = (*it);
            log(keyword + ' ' + name);
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                            name, m_scope[m_scopeIndex], m_comment);
            m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
        }
        if (advance() != ";") {
            uError() << "importJava: unexpected: " << m_source[m_srcIndex];
            skipStmt();
        }
        return true;
    }
    if (keyword == "class" || keyword == "interface") {
        const QString& name = advance();
        const UMLObject::ObjectType t = (keyword == "class" ? UMLObject::ot_Class : UMLObject::ot_Interface);
        log(keyword + ' ' + name);
        UMLObject *ns = Import_Utils::createUMLObject(t, name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        m_klass->setAbstract(m_isAbstract);
        m_klass->setStatic(m_isStatic);
        m_klass->setVisibility(m_currentAccess);
        // The UMLObject found by createUMLObject might originally have been created as a
        // placeholder with a type of class but if is really an interface, then we need to
        // change it.
        UMLObject::ObjectType ot = (keyword == "interface" ? UMLObject::ot_Interface : UMLObject::ot_Class);
        m_klass->setBaseType(ot);
        m_isAbstract = m_isStatic = false;
        // if no modifier is specified in an interface, then it means public
        if ( m_klass->isInterface() ) {
            m_defaultCurrentAccess =  Uml::Visibility::Public;
        }
        if (advance() == ";")   // forward declaration
            return true;
        if (m_source[m_srcIndex] == "<") {
            // template args - preliminary, rudimentary implementation
            // @todo implement all template arg syntax
            uint start = m_srcIndex;
            if (! skipToClosing('<')) {
                uError() << "importJava(" << name << "): template syntax error";
                return false;
            }
            while (1) {
                const QString arg = m_source[++start];
                if (! arg.contains( QRegExp("^[A-Za-z_]") )) {
                    uDebug() << "importJava(" << name << "): cannot handle template syntax ("
                        << arg << ")";
                    break;
                }
                /* UMLTemplate *tmpl = */ m_klass->addTemplate(arg);
                const QString next = m_source[++start];
                if (next == ">")
                    break;
                if (next != ",") {
                    uDebug() << "importJava(" << name << "): cannot handle template syntax ("
                        << next << ")";
                    break;
                }
            }
            advance();  // skip over ">"
        }
        if (m_source[m_srcIndex] == "extends") {
            const QString& baseName = advance();
            // try to resolve the class we are extending, or if impossible
            // create a placeholder
            UMLObject *parent = resolveClass( baseName );
            if ( parent ) {
                Import_Utils::createGeneralization(m_klass, static_cast<UMLClassifier*>(parent));
            } else {
                uDebug() << "importJava parentClass " << baseName
                    << " is not resolveable. Creating placeholder";
                Import_Utils::createGeneralization(m_klass, baseName);
            }
            advance();
        }
        if (m_source[m_srcIndex] == "implements") {
            while (m_srcIndex < srcLength - 1 && advance() != "{") {
                const QString& baseName = m_source[m_srcIndex];
                // try to resolve the interface we are implementing, if this fails
                // create a placeholder
                UMLObject *interface = resolveClass( baseName );
                if ( interface ) {
                     Import_Utils::createGeneralization(m_klass, static_cast<UMLClassifier*>(interface));
                } else {
                    uDebug() << "importJava implementing interface "<< baseName
                        <<" is not resolvable. Creating placeholder";
                    Import_Utils::createGeneralization(m_klass, baseName);
                }
                if (advance() != ",")
                    break;
            }
        }
        if (m_source[m_srcIndex] != "{") {
            uError() << "importJava: ignoring excess chars at " << name
                << " (" << m_source[m_srcIndex] << ")";
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "enum") {
        const QString& name = advance();
        log(keyword + ' ' + name);
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                        name, m_scope[m_scopeIndex], m_comment);
        UMLEnum *enumType = static_cast<UMLEnum*>(ns);
        skipStmt("{");
        while (m_srcIndex < srcLength - 1 && advance() != "}") {
            Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
            QString next = advance();
            if (next == "{" || next == "(") {
                if (! skipToClosing(next[0]))
                    return false;
                next = advance();
            }
            if (next != ",") {
                if (next == ";") {
                    // @todo handle methods in enum
                    // For now, we cheat (skip them)
                    m_source[m_srcIndex] = '{';
                    if (! skipToClosing('{'))
                        return false;
                }
                break;
            }
        }
        return true;
    }
    if (keyword == "static") {
        m_isStatic = true;
        return true;
    }
    // if we detected static previously and keyword is { then this is a static block
    if (m_isStatic && keyword == "{") {
        // reset static flag and jump to end of static block
        m_isStatic = false;
        return skipToClosing('{');
    }
    if (keyword == "abstract") {
        m_isAbstract = true;
        return true;
    }
    if (keyword == "public") {
        m_currentAccess = Uml::Visibility::Public;
        return true;
    }
    if (keyword == "protected") {
        m_currentAccess = Uml::Visibility::Protected;
        return true;
    }
    if (keyword == "private") {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == "final" ||
        keyword == "native" ||
        keyword == "synchronized" ||
        keyword == "transient" ||
        keyword == "volatile") {
        //@todo anything to do here?
        return true;
    }
    if (keyword == "import") {
        // keep track of imports so we can resolve classes we are dependent on
        QString import = advance();
        if ( import.endsWith('.') ) {
            //this most likely an import that ends with a *
            //
            import = import + advance();
        }
        m_imports.append( import );

        // move past ;
        skipStmt();
        return true;
    }
    if (keyword == "@") {  // annotation
        advance();
        if (m_source[m_srcIndex + 1] == "(") {
            advance();
            skipToClosing('(');
        }
        return true;
    }
    if (keyword == "}") {
        if (m_scopeIndex)
            m_klass = dynamic_cast<UMLClassifier*>(m_scope[--m_scopeIndex]);
        else
            uError() << "importJava: too many }";
        return true;
    }
    // At this point, we expect `keyword' to be a type name
    // (of a member of class or interface, or return type
    // of an operation.) Up next is the name of the attribute
    // or operation.
    if (! keyword.contains( QRegExp("^\\w") )) {
        uError() << "importJava: ignoring " << keyword <<
            " at " << m_srcIndex << ", " << m_source.count() << " in " << m_klass->name();
        return false;
    }
    QString typeName = m_source[m_srcIndex];
    typeName = joinTypename(typeName);
    // At this point we need a class.
    if (m_klass == NULL) {
        uError() << "importJava: no class set for " << typeName;
        return false;
    }
    QString name = advance();
    QString nextToken;
    if (typeName == m_klass->name() && name == "(") {
        // Constructor.
        nextToken = name;
        name = typeName;
        typeName.clear();
    } else {
        nextToken = advance();
    }
    if (name.contains( QRegExp("\\W") )) {
        uError() << "importJava: expecting name in " << name;
        return false;
    }
    if (nextToken == "(") {
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        m_srcIndex++;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
            QString typeName = m_source[m_srcIndex];
            if ( typeName == "final" || typeName.startsWith( "//") ) {
                // ignore the "final" keyword and any comments in method args
                typeName = advance();
            }
            typeName = joinTypename(typeName);
            QString parName = advance();
            // the Class might not be resolved yet so resolve it if necessary
            UMLObject *obj = resolveClass(typeName);
            if (obj) {
                // by prepending the package, unwanted placeholder types will not get created
                typeName = obj->fullyQualifiedName(".");
            }
            /* UMLAttribute *att = */ Import_Utils::addMethodParameter(op, typeName, parName);
            if (advance() != ",")
                break;
            m_srcIndex++;
        }
        // before adding the method, try resolving the return type
        UMLObject *obj = resolveClass(typeName);
        if (obj) {
            // using the fully qualified name means that a placeholder type will not be created.
            typeName = obj->fullyQualifiedName(".");
        }
        Import_Utils::insertMethod(m_klass, op, m_currentAccess, typeName,
                                   m_isStatic, m_isAbstract, false /*isFriend*/,
                                   false /*isConstructor*/, m_comment);
        m_isAbstract = m_isStatic = false;
        // reset the default visibility
        m_currentAccess = m_defaultCurrentAccess;
        // At this point we do not know whether the method has a body or not.
        do {
            nextToken = advance();
        } while (nextToken != "{" && nextToken != ";");
        if (nextToken == ";") {
            // No body (interface or abstract)
            return true;
        } else {
            return skipToClosing('{');
        }
    }
    // At this point we know it's some kind of attribute declaration.
    while (1) {
        while (nextToken != "," && nextToken != ";") {
            if (nextToken == "=") {
                if ((nextToken = advance()) == "new") {
                    advance();
                    if ((nextToken = advance()) == "(") {
                        skipToClosing('(');
                        if ((nextToken = advance()) == "{") {
                            skipToClosing('{');
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
        UMLObject *type = resolveClass( typeName );
        UMLObject *o;
        if (type) {
            o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                           static_cast<UMLClassifier*>(type), m_comment, m_isStatic);
        } else {
            o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                  typeName, m_comment, m_isStatic);
        }
        // UMLAttribute *attr = static_cast<UMLAttribute*>(o);
        if (nextToken != ",") {
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
        if (m_source[m_srcIndex] != ";") {
            uError() << "importJava: ignoring trailing items at " << name;
            skipStmt();
        }
    }
    else {
        uError() << "index out of range: ignoring statement " << name;
        skipStmt();
    }
    return true;
}
