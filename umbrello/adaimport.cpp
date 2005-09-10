/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "adaimport.h"

#include <stdio.h>
// qt/kde includes
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "import_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "package.h"
#include "classifier.h"
#include "enum.h"
#include "operation.h"
#include "attribute.h"

AdaImport::AdaImport() : NativeImportBase("--") {
}

AdaImport::~AdaImport() {
}

bool AdaImport::preprocess(QString&) {
    return false;
}

void AdaImport::fillSource(QString word) {
    QString lexeme;
    const uint len = word.length();
    bool inString = false;
    for (uint i = 0; i < len; i++) {
        QChar c = word[i];
        if (c == '"') {
            lexeme += c;
            if (inString) {
                m_source.append(lexeme);
                lexeme = QString::null;
            }
            inString = !inString;
        } else if (inString ||
                   c.isLetterOrNumber() || c == '_' || c == '.' || c == '#') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme = QString::null;
            }
            if (c == ':' && word[i + 1] == '=') {
                m_source.append(":=");
                i++;
            } else {
                m_source.append(c);
            }
        }
    }
    if (!lexeme.isEmpty())
        m_source.append(lexeme);
}

void AdaImport::parseFile(QString filename) {
    if (filename.contains('/')) {
        QString path = filename;
        path.remove( QRegExp("/[^/]+$") );
        kdDebug() << "IDLImport::parseFile: adding path " << path << endl;
        Import_Utils::addIncludePath(path);
    }
    if (! QFile::exists(filename)) {
        if (filename.startsWith("/")) {
            kdError() << "AdaImport::parseFile(" << filename
                      << "): cannot find file" << endl;
            return;
        }
        bool found = false;
        QStringList includePaths = Import_Utils::includePathList();
        for (QStringList::Iterator pathIt = includePaths.begin();
                                   pathIt != includePaths.end(); ++pathIt) {
            QString path = (*pathIt);
            if (! path.endsWith("/")) {
                path.append("/");
            }
            if (QFile::exists(path + filename)) {
                filename.prepend(path);
                found = true;
                break;
            }
        }
        if (! found) {
            kdError() << "AdaImport::parseFile(" << filename
                      << "): cannot find file" << endl;
            return;
        }
    }
    QFile file(filename);
    if (! file.open(IO_ReadOnly)) {
        kdError() << "AdaImport::parseFile(" << filename
                  << "): cannot open file" << endl;
        return;
    }
    // Scan the input file into the QStringList m_source.
    m_srcIndex = 0;
    QTextStream stream(&file);
    while (! stream.atEnd()) {
        QString line = stream.readLine();
        scan(line);
    }
    file.close();
    // Parse the QStringList m_source.
    m_klass = NULL;
    m_currentAccess = Uml::Public;
    m_isAbstract = false;
    const uint srcLength = m_source.count();
    bool inGenericFormalPart = false;
    for (m_srcIndex = 0; m_srcIndex < srcLength; m_srcIndex++) {
        const QString& keyword = m_source[m_srcIndex];
        kdDebug() << '"' << keyword << '"' << endl;
        if (keyword.startsWith("--")) {
            m_comment += keyword.mid(2) + '\n';
            continue;
        }
        if (keyword == "with") {
            if (inGenericFormalPart) {
                // mapping of generic formal subprograms or packages is not yet implemented
                skipStmt();
                continue;
            }
            while (++m_srcIndex < srcLength && m_source[m_srcIndex] != ";") {
                QStringList components = QStringList::split(".", m_source[m_srcIndex].lower());
                const QString& prefix = components.first();
                if (prefix == "system" || prefix == "ada" || prefix == "gnat" ||
                    prefix == "interfaces" || prefix == "text_io" ||
                    prefix == "unchecked_conversion" ||
                    prefix == "unchecked_deallocation") {
                    if (advance() != ",")
                        break;
                    continue;
                }
                QString base = prefix;
                uint i = 0;
                while (1) {
                    if (! m_parsedFiles.contains(base)) {
                        m_parsedFiles.append(base);
                        QString filename = base + ".ads";
                        // Save current m_source and m_srcIndex.
                        QStringList source(m_source);
                        uint srcIndex = m_srcIndex;
                        m_source.clear();
                        parseFile(filename);
                        // Restore m_source and m_srcIndex.
                        m_source = source;
                        m_srcIndex = srcIndex;
                        // Also reset m_currentAccess.
                        // CHECK: need to reset more stuff?
                        m_currentAccess = Uml::Public;
                    }
                    if (++i >= components.count())
                        break;
                    base += '-' + components[i];
                }
                if (advance() != ",")
                    break;
            }
            continue;
        }
        if (keyword == "generic") {
            inGenericFormalPart = true;
            continue;
        }
        if (keyword == "package") {
            const QString& name = advance();
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Package,
                            name, m_scope[m_scopeIndex], m_comment);
            if (advance() == "is") {
                if (m_source[m_srcIndex + 1] == "new") {
                    // generic package instantiation: TBD
                    skipStmt();
                } else {
                    m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
                }
            } else if (m_source[m_srcIndex] != "renames") {
                kdError() << "AdaImport::parseFile: unexpected: " << m_source[m_srcIndex] << endl;
                skipStmt("is");
            }
            if (inGenericFormalPart) {
                // handling of generic formal parameters: TBD
                inGenericFormalPart = false;
            }
            m_comment = QString::null;
            continue;
        }
        if (keyword == "type") {
            const QString& name = advance();
            if (advance() == "(") {
                kdDebug() << "AdaImport::parseFile(" << name << "): "
                    << "discriminant handling is not yet implemented" << endl;
                skipStmt(")");
            }
            if (m_source[m_srcIndex] == ";") {
                // forward declaration
                // To Be Done
                continue;
            }
            if (m_source[m_srcIndex] != "is") {
                kdError() << "AdaImport::parseFile: expecting \"is\"" << endl;
                skipStmt();
                continue;
            }
            if (advance() == "(") {
                // enum type
                UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Enum,
                                name, m_scope[m_scopeIndex], m_comment);
                UMLEnum *enumType = static_cast<UMLEnum*>(ns);
                while (++m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
                    Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
                    if (advance() != ",")
                        break;
                }
                skipStmt();
                m_comment = QString::null;
                continue;
            }
            bool isTaggedType = false;
            if (m_source[m_srcIndex] == "abstract") {
                m_isAbstract = true;
                m_srcIndex++;
            }
            if (m_source[m_srcIndex] == "tagged") {
                UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                                name, m_scope[m_scopeIndex], m_comment);
                ns->setAbstract(m_isAbstract);
                m_isAbstract = false;
                m_comment = QString::null;
                m_srcIndex++;
                isTaggedType = true;
            }
            if (m_source[m_srcIndex] == "limited") {
                m_srcIndex++;  // we can't (yet?) represent that
            }
            if (m_source[m_srcIndex] == "private" ||
                (m_source[m_srcIndex] == "null" &&
                 m_source[m_srcIndex+1] == "record")) {
                skipStmt();
                continue;
            }
            if (m_source[m_srcIndex] == "record") {
                // If it's a tagged record then the class was already created
                // above (see processing for "tagged".) Doesn't matter;
                // in that case Import_Utils::createUMLObject() just returns
                // the existing class instead of creating a new one.
                UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                                name, m_scope[m_scopeIndex], m_comment);
                if (! isTaggedType)
                    ns->setStereotype("record");
                m_klass = static_cast<UMLClassifier*>(ns);
                m_comment = QString::null;
                continue;
            }
            if (m_source[m_srcIndex] == "new") {
                QString base = advance();
                UMLClassifier *parent = NULL;
                if (advance() == "with") {
                    UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                                    base, NULL);
                    parent = static_cast<UMLClassifier*>(ns);
                    ns = Import_Utils::createUMLObject(Uml::ot_Class, name,
                                           m_scope[m_scopeIndex], m_comment);
                    m_comment = QString::null;
                    QString nextLexeme = advance();
                    if (nextLexeme == "null" || nextLexeme == "record") {
                        UMLClassifier *klass = static_cast<UMLClassifier*>(ns);
                        Import_Utils::createGeneralization(klass, parent);
                        if (nextLexeme == "record") {
                            // Set the m_klass for attributes.
                            m_klass = klass;
                            continue;
                        }
                    }
                }
            }
            // Datatypes: TO BE DONE
            skipStmt();
            m_comment = QString::null;
            continue;
        }
        if (keyword == "private") {
            m_currentAccess = Uml::Private;
            continue;
        }
        if (keyword == "end") {
            if (m_klass) {
                if (advance() != "record") {
                    kdError() << "end: expecting \"record\" at "
                              << m_source[m_srcIndex] << endl;
                }
                m_klass = NULL;
            } else if (m_scopeIndex) {
                if (advance() != ";") {
                    const QString& scopeName = m_scope[m_scopeIndex]->getName();
                    if (scopeName != m_source[m_srcIndex])
                        kdError() << "end: expecting " << scopeName << ", found "
                                  << m_source[m_srcIndex] << endl;
                }
                m_scopeIndex--;
                m_currentAccess = Uml::Public;   // @todo make a stack for this
            } else {
                kdError() << "importAda: too many \"end\"" << endl;
            }
            skipStmt();
            continue;
        }
        if (keyword == "function" || keyword == "procedure") {
            const QString& name = advance();
            QString returnType;
            if (advance() != "(") {
                // Unlike an Ada package, a UML package does not support
                // subprograms.
                // In order to map those, we would need to create a UML
                // class with stereotype <<utility>> for the Ada package.
                kdDebug() << "ignoring parameterless " << keyword << " " << name << endl;
                skipStmt();
                continue;
            }
            UMLClassifier *klass = NULL;
            UMLOperation *op = NULL;
            const uint MAX_PARNAMES = 16;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
                QString parName[MAX_PARNAMES];
                uint parNameCount = 0;
                do {
                    if (parNameCount >= MAX_PARNAMES) {
                        kdError() << "MAX_PARNAMES is exceeded at " << name << endl;
                        break;
                    }
                    parName[parNameCount++] = advance();
                } while (advance() == ",");
                if (m_source[m_srcIndex] != ":") {
                    kdError() << "importAda: expecting ':'" << endl;
                    skipStmt();
                    break;
                }
                const QString &direction = advance();
                QString typeName;
                Uml::Parameter_Direction dir = Uml::pd_In;
                if (direction == "access") {
                    // Oops, we have to improvise here because there
                    // is no such thing as "access" in UML.
                    // So we use the next best thing, "inout".
                    // Better ideas, anyone?
                    dir = Uml::pd_InOut;
                    typeName = advance();
                } else if (direction == "in") {
                    if (m_source[m_srcIndex + 1] == "out") {
                        dir = Uml::pd_InOut;
                        m_srcIndex++;
                    }
                    typeName = advance();
                } else if (direction == "out") {
                    dir = Uml::pd_Out;
                    typeName = advance();
                } else {
                    typeName = direction;  // In Ada, the default direction is "in"
                }
                if (op == NULL) {
                    // In Ada, the first parameter indicates the class.
                    UMLDoc *umldoc = UMLApp::app()->getDocument();
                    UMLObject *type = umldoc->findUMLObject(typeName, Uml::ot_Class, m_scope[m_scopeIndex]);
                    if (type == NULL) {
                        kdError() << "importAda: cannot find UML object for " << typeName << endl;
                        skipStmt();
                        break;
                        /*** better:
                        if (advance() == ";") {
                            m_srcIndex++;
                            continue;
                        } else {
                            break;
                        }
                         ****/
                    }
                    Uml::Object_Type t = type->getBaseType();
                    if (t != Uml::ot_Interface &&
                        (t != Uml::ot_Class || type->getStereotype(false) == "record")) {
                        // Not an instance bound method - we cannot represent it.
                        skipStmt(")");
                        break;
                    }
                    klass = static_cast<UMLClassifier*>(type);
                    op = Import_Utils::makeOperation(klass, name);
                    // The controlling parameter is suppressed.
                    parNameCount--;
                    if (parNameCount) {
                        for (uint i = 0; i < parNameCount; i++)
                            parName[i] = parName[i + 1];
                    }
                }
                for (uint i = 0; i < parNameCount; i++) {
                    UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName[i]);
                    att->setParmKind(dir);
                }
                if (advance() != ";")
                    break;
            }
            if (keyword == "function") {
                if (advance() != "return") {
                    if (klass)
                        kdError() << "importAda: expecting \"return\" at function "
                            << name << endl;
                    skipStmt();
                    continue;
                }
                returnType = advance();
            }
            bool isAbstract = false;
            if (advance() == "is" && advance() == "abstract")
                isAbstract = true;
            if (klass != NULL && op != NULL)
                Import_Utils::insertMethod(klass, op, m_currentAccess, returnType,
                                           false, isAbstract, false, false, m_comment);
            m_comment = QString::null;
            skipStmt();
            continue;
        }
        if (keyword == "subtype") {    // FIXMEnow: potentially important but not yet implemented
            m_comment = QString::null;
            skipStmt();
            continue;
        }
        if (keyword == "task" || keyword == "protected") {
            // Can task and protected objects/types be mapped to UML?
            bool isType = false;
            QString name = advance();
            if (name == "type") {
                isType = true;
                name = advance();
            }
            QString next = advance();
            if (next == "(") {
                skipStmt(")");  // skip discriminant
                next = advance();
            }
            if (next == "is")
                skipStmt("end");
            skipStmt();
            m_comment = QString::null;
            continue;
        }
        if (keyword == "for") {    // rep spec (yuck)
            QString typeName = advance();
            QString next = advance();
            if (next == "'") {
                advance();  // skip qualifier
                next = advance();
            }
            if (next == "use") {
                if (advance() == "record")
                    skipStmt("end");
            } else {
                kdError() << "importAda: expecting \"use\" at rep spec of "
                          << typeName << endl;
            }
            m_comment = QString::null;
            skipStmt();
            continue;
        }
        // At this point we're only interested in attribute declarations.
        if (m_klass == NULL || keyword == "null") {
            skipStmt();
            m_comment = QString::null;
            continue;
        }
        const QString& name = keyword;
        if (advance() != ":") {
            kdError() << "adaImport: expecting \":\" at " << name << " "
                      << m_source[m_srcIndex] << endl;
            skipStmt();
            continue;
        }
        QString typeName = advance();
        QString initialValue;
        if (advance() == ":=") {
            QString expr = advance();
            QString token;
            while ((token = advance()) != ";") {
                expr.append(" " + token);
            }
        }
        UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                     typeName, m_comment);
        UMLAttribute *attr = static_cast<UMLAttribute*>(o);
        attr->setInitialValue(initialValue);
        m_comment = QString::null;
        skipStmt();
    }
}


