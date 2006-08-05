/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "javaimport.h"

// qt/kde includes
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "import_utils.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../umlpackagelist.h"
#include "../package.h"
#include "../classifier.h"
#include "../enum.h"
#include "../operation.h"
#include "../attribute.h"

JavaImport::JavaImport() : NativeImportBase("//") {
    setMultiLineComment("/*", "*/");
    initVars();
}

JavaImport::~JavaImport() {
}

void JavaImport::initVars() {
    m_isStatic = false;
}

/// Catenate possible template arguments/array dimensions to the end of the type name.
QString JavaImport::joinTypename() {
    QString typeName = m_source[m_srcIndex];
    if (m_source[m_srcIndex + 1] == "<" ||
        m_source[m_srcIndex + 1] == "[") {
        uint start = ++m_srcIndex;
        if (! skipToClosing(m_source[start][0]))
            return typeName;
        for (uint i = start; i <= m_srcIndex; i++) {
            typeName += m_source[i];
        }
    }
    return typeName;
}

void JavaImport::fillSource(QString word) {
    QString lexeme;
    const uint len = word.length();
    bool inString = false;
    for (uint i = 0; i < len; i++) {
        const QChar& c = word[i];
        if (c == '"') {
            lexeme += c;
            if (i == 0 || word[i - 1] != '\\') {
                if (inString) {
                    m_source.append(lexeme);
                    lexeme = QString::null;
                }
                inString = !inString;
            }
        } else if (inString ||
                   c.isLetterOrNumber() || c == '_' || c == '.') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme = QString::null;
            }
            m_source.append(QString(c));
        }
    }
    if (!lexeme.isEmpty())
        m_source.append(lexeme);
}

bool JavaImport::skipToClosing(QChar opener) {
    QString closing;
    switch (opener) {
        case '{':
            closing = "}";
            break;
        case '[':
            closing = "]";
            break;
        case '(':
            closing = ")";
            break;
        case '<':
            closing = ">";
            break;
        default:
            kdError() << "JavaImport::skipToClosing(" << opener
                << "): " << "illegal input character" << endl;
            return false;
    }
    const QString opening(opener);
    skipStmt(opening);
    const uint srcLength = m_source.count();
    int nesting = 0;
    while (m_srcIndex < srcLength) {
        QString nextToken = advance();
        if (nextToken.isEmpty())
            break;
        if (nextToken == closing) {
            if (nesting <= 0)
                break;
            nesting--;
        } else if (nextToken == opening) {
            nesting++;
        }
    }
    if (m_srcIndex == srcLength)
        return false;
    return true;
}


///Spawn off an import of the specifed file
void JavaImport::spawnImport( QString file ) {
    static QStringList filesBeingParsed;
    // if the file is being parsed, don't bother
    //
    if (filesBeingParsed.contains( file ) ) {
        return;
    }
    if (QFile::exists(file)) {
          JavaImport importer;
          QStringList fileList;
          fileList.append( file );
          filesBeingParsed.append( file );
          importer.importFiles( fileList ); 
          filesBeingParsed.remove( file );
    }
}


///returns the UML Object if found, or null otherwise 
UMLObject* findObject( QString name,   UMLPackage *parentPkg ) {
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    UMLObject * o = umldoc->findUMLObject(name, Uml::ot_UMLObject , parentPkg); 
    return o;
}


///Resolve the specified className 
UMLObject* JavaImport::resolveClass (QString className) {
    // java has a few implicit imports.  Most relevant for this is the 
    // current package, which is in the same directory as the current file
    // being parsed
    //
    QStringList file = QStringList::split( '/', m_currentFileName);
    // remove the filename.  This leaves the full path to the containing
    // dir which should also include the package hierarchy
    //
    file.pop_back();

    // the file we're looking for might be in the same directory as the
    // current class
    //
    QString myDir = file.join( "/" );
    QString myFile = "/" + myDir + "/" + className + ".java";
    if ( QFile::exists(myFile) ) {
        spawnImport( myFile );
        return findObject ( className, m_scope[m_scopeIndex]);
    }

    // the class we want is not in the same package as the one being imported.
    // use the imports to find the one we want.
    //
    QStringList package = QStringList::split( '.', m_currentPackage);
    int dirsInPackageCount = package.size();

    for (int count=0; count < dirsInPackageCount; count ++ ) {
        // pop off one by one the directories, until only the source root remains
        //
        file.pop_back();
    }
    // this is now the root of any further source imports
    QString sourceRoot = "/" + file.join("/") + "/";

    for (QStringList::Iterator pathIt = m_imports.begin();
                                   pathIt != m_imports.end(); ++pathIt) {
        QString import = (*pathIt);
        QStringList split = QStringList::split( '.', import );
        split.pop_back(); // remove the * or the classname
        if ( import.endsWith( "*" ) || import.endsWith( className) ) {
            // check if the file we want is in this imported package
            // convert the org.test type package into a filename
            //
            QString aFile = sourceRoot + split.join("/") + "/" + className + ".java";
            if ( QFile::exists(aFile) ) {
                spawnImport( aFile );
                // we need to set the package for the class that will be resolved
                // start at the root package
                UMLPackage *parent = m_scope[0];
                UMLPackage *current;

                for (QStringList::Iterator it = split.begin(); it != split.end(); ++it) {
                    QString name = (*it);
                    UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Package,
                            name, parent);
                    current = static_cast<UMLPackage*>(ns);
                    parent = current;
                } // for
                // now that we have the right package, the class should be findable
                //
                return findObject ( className, current);
            } // if file exists
        } // if import matches
    } //foreach import
    return NULL; // no match
}


/// keep track of the current file being parsed and reset the list of imports
void JavaImport::parseFile(QString filename) {
    m_currentFileName= filename;
    m_imports.clear();
    // default visibility is Impl, unless we are an interface, then it is 
    // public for member vars and methods
    m_defaultCurrentAccess = Uml::Visibility::Implementation;
    m_currentAccess = m_defaultCurrentAccess;

    NativeImportBase::parseFile(filename);
}




bool JavaImport::parseStmt() {
    const uint srcLength = m_source.count();
    const QString& keyword = m_source[m_srcIndex];
    //kdDebug() << '"' << keyword << '"' << endl;
    if (keyword == "package") {
        m_currentPackage = advance();
        const QString& qualifiedName = m_currentPackage;
        QStringList names = QStringList::split(".", qualifiedName);
        for (QStringList::Iterator it = names.begin(); it != names.end(); ++it) {
            QString name = (*it);
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Package,
                            name, m_scope[m_scopeIndex], m_comment);
            m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
        }
        if (advance() != ";") {
            kdError() << "importJava: unexpected: " << m_source[m_srcIndex] << endl;
            skipStmt();
        }
        return true;
    }
    if (keyword == "class" || keyword == "interface") {
        const QString& name = advance();
        const Uml::Object_Type t = (keyword == "class" ? Uml::ot_Class : Uml::ot_Interface);
        UMLObject *ns = Import_Utils::createUMLObject(t, name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        m_klass->setAbstract(m_isAbstract);
        m_klass->setStatic(m_isStatic);
        m_klass->setVisibility(m_currentAccess);
        // The UMLObject found by createUMLObject might originally have been created as a
        // placeholder with a type of class but if is really an interface, then we need to
        // change it.
        m_klass->setInterface(keyword == "interface");
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
                kdError() << "importJava(" << name << "): template syntax error" << endl;
                return false;
            }
            while (1) {
                const QString arg = m_source[++start];
                if (! arg.contains( QRegExp("^[A-Za-z_]") )) {
                    kdDebug() << "importJava(" << name << "): cannot handle template syntax ("
                        << arg << ")" << endl;
                    break;
                }
                /* UMLTemplate *tmpl = */ m_klass->addTemplate(arg);
                const QString next = m_source[++start];
                if (next == ">")
                    break;
                if (next != ",") {
                    kdDebug() << "importJava(" << name << "): can't handle template syntax ("
                        << next << ")" << endl;
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
                kdDebug() << "importJava parentClass " << baseName 
                    << " is not resolveable. Creating placeholder" << endl;
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
                    kdDebug() << "importJava implementing interface "<< baseName 
                        <<" is not resolvable. Creating placeholder" <<endl;
                    Import_Utils::createGeneralization(m_klass, baseName); 
                }
                if (advance() != ",")
                    break;
            }
        }
        if (m_source[m_srcIndex] != "{") {
            kdError() << "importJava: ignoring excess chars at " << name
                << " (" << m_source[m_srcIndex] << ")" << endl;
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "enum") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Enum,
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
                    m_source[m_srcIndex] = "{";
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
        if ( import.endsWith(".") ) {
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
            kdError() << "importJava: too many }" << endl;
        return true;
    }
    // At this point, we expect `keyword' to be a type name
    // (of a member of class or interface, or return type
    // of an operation.) Up next is the name of the attribute
    // or operation.
    if (! keyword.contains( QRegExp("^\\w") )) {
        kdError() << "importJava: ignoring " << keyword << endl;
        return false;
    }
    QString typeName = joinTypename();
    // At this point we need a class.
    if (m_klass == NULL) {
        kdError() << "importJava: no class set for " << typeName << endl;
        return false;
    }
    QString name = advance();
    QString nextToken;
    if (typeName == m_klass->getName() && name == "(") {
        // Constructor.
        nextToken = name;
        name = typeName;
        typeName = QString::null;
    } else {
        nextToken = advance();
    }
    if (name.contains( QRegExp("\\W") )) {
        kdError() << "importJava: expecting name in " << name << endl;
        return false;
    }
    if (nextToken == "(") {
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        m_srcIndex++;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
            QString typeName = joinTypename();
            QString parName = advance();
            // the Class might not be resolved yet so resolve it if necessary
            UMLObject *obj = resolveClass(typeName);
            if (obj) {
                // by prepending the package, unwanted placeholder types will not get created
                typeName = obj->getFullyQualifiedName(".");
            }
            UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName);
            if (advance() != ",")
                break;
            m_srcIndex++;
        }
        // before adding the method, try resolving the return type
        UMLObject *obj = resolveClass(typeName);
        if (obj) {
            // using the fully qualified name means that a placeholder type will not be created.
            typeName = obj->getFullyQualifiedName(".");
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
        UMLAttribute *attr = static_cast<UMLAttribute*>(o);
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
    if (m_source[m_srcIndex] != ";") {
        kdError() << "importJava: ignoring trailing items at " << name << endl;
        skipStmt();
    }
    return true;
}


