/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "adaimport.h"

#include <stdio.h>
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "import_utils.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../package.h"
#include "../classifier.h"
#include "../enum.h"
#include "../operation.h"
#include "../attribute.h"

AdaImport::AdaImport() : NativeImportBase("--") {
   initVars();
}

AdaImport::~AdaImport() {
}

void AdaImport::initVars() {
    m_inGenericFormalPart = false;
}

/// Split the line so that a string is returned as a single element of the list,
/// when not in a string then split at white space.
QStringList AdaImport::split(QString line) {
    QStringList list;
    QString listElement;
    bool inString = false;
    bool seenSpace = false;
    line = line.stripWhiteSpace();
    uint len = line.length();
    for (uint i = 0; i < len; i++) {
        const QChar& c = line[i];
        if (inString) {
            listElement += c;
            if (i > 0 && line[i - 1] == '"')
                continue;   // escaped quotation mark
            list.append(listElement);
            listElement.clear();
            inString = false;
        } else if (c == '"') {
            inString = true;
            if (!listElement.isEmpty())
                list.append(listElement);
            listElement = QString(c);
            seenSpace = false;
        } else if (c == '\'') {
            if (i < len - 2 && line[i + 2] == '\'') {
                // character constant
                if (!listElement.isEmpty())
                    list.append(listElement);
                listElement = line.mid(i, 3);
                i += 2;
                list.append(listElement);
                listElement.clear();
                continue;
            }
            listElement += c;
            seenSpace = false;
        } else if (c.isSpace()) {
            if (seenSpace)
                continue;
            seenSpace = true;
            if (!listElement.isEmpty()) {
                list.append(listElement);
                listElement.clear();
            }
        } else {
            listElement += c;
            seenSpace = false;
        }
    }
    if (!listElement.isEmpty())
        list.append(listElement);
    return list;
}

void AdaImport::fillSource(QString word) {
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; i++) {
        QChar c = word[i];
        if (c.isLetterOrNumber() || c == '_' || c == '.' || c == '#') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme.clear();
            }
            if (c == ':' && word[i + 1] == '=') {
                m_source.append(":=");
                i++;
            } else {
                m_source.append(QString(c));
            }
        }
    }
    if (!lexeme.isEmpty())
        m_source.append(lexeme);
}

bool AdaImport::parseStmt() {
    const uint srcLength = m_source.count();
    const QString& keyword = m_source[m_srcIndex];
    //kDebug() << '"' << keyword << '"' << endl;
    if (keyword == "with") {
        if (m_inGenericFormalPart) {
            // mapping of generic formal subprograms or packages is not yet implemented
            return false;
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
                    m_currentAccess = Uml::Visibility::Public;
                }
                if (++i >= components.count())
                    break;
                base += '-' + components[i];
            }
            if (advance() != ",")
                break;
        }
        return true;
    }
    if (keyword == "generic") {
        m_inGenericFormalPart = true;
        return true;
    }
    if (keyword == "package") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Package, name,
                                                      m_scope[m_scopeIndex], m_comment);
        if (advance() == "is") {
            if (m_source[m_srcIndex + 1] == "new") {
                // generic package instantiation: TBD
                skipStmt();
            } else {
                m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
            }
        } else if (m_source[m_srcIndex] != "renames") {
            kError() << "AdaImport::parseFile: unexpected: " << m_source[m_srcIndex] << endl;
            skipStmt("is");
        }
        if (m_inGenericFormalPart) {
            // handling of generic formal parameters: TBD
            m_inGenericFormalPart = false;
        }
        return true;
    }
    if (keyword == "type") {
        const QString& name = advance();
        if (advance() == "(") {
            kDebug() << "AdaImport::parseFile(" << name << "): "
                << "discriminant handling is not yet implemented" << endl;
            // @todo Find out how to map discriminated record to UML.
            //       For now, we just create a pro forma empty record.
            Import_Utils::createUMLObject(Uml::ot_Class, name, m_scope[m_scopeIndex],
                                          m_comment, "record");
            skipStmt("end");
            if (m_source[++m_srcIndex] == "case")
                m_srcIndex += 2;  // skip "case" ";"
            // we are now positioned on "end"
            m_srcIndex += 2;      // skip "end" "record"
            // we are now positioned on ";"
            return true;
        }
        if (m_source[m_srcIndex] == ";") {
            // forward declaration
            // To Be Done
            return true;
        }
        if (m_source[m_srcIndex] != "is") {
            kError() << "AdaImport::parseFile: expecting \"is\"" << endl;
            return false;
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
            return true;
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
            return true;
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
            return true;
        }
        if (m_source[m_srcIndex] == "new") {
            QString base = advance();
            const bool isExtension = (advance() == "with");
            Uml::Object_Type t = (isExtension || m_isAbstract ? Uml::ot_Class
                                                              : Uml::ot_Datatype);
            UMLObject *ns = Import_Utils::createUMLObject(t, base, NULL);
            UMLClassifier *parent = static_cast<UMLClassifier*>(ns);
            ns = Import_Utils::createUMLObject(Uml::ot_Class, name,
                                               m_scope[m_scopeIndex], m_comment);
            if (isExtension) {
                QString nextLexeme = advance();
                if (nextLexeme == "null" || nextLexeme == "record") {
                    UMLClassifier *klass = static_cast<UMLClassifier*>(ns);
                    Import_Utils::createGeneralization(klass, parent);
                    if (nextLexeme == "record") {
                        // Set the m_klass for attributes.
                        m_klass = klass;
                    }
                }
            }
            skipStmt();
            return true;
        }
        // Datatypes: TO BE DONE
        return false;
    }
    if (keyword == "private") {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == "end") {
        if (m_klass) {
            if (advance() != "record") {
                kError() << "end: expecting \"record\" at "
                          << m_source[m_srcIndex] << endl;
            }
            m_klass = NULL;
        } else if (m_scopeIndex) {
            if (advance() != ";") {
                const QString& scopeName = m_scope[m_scopeIndex]->getName();
                if (scopeName != m_source[m_srcIndex])
                    kError() << "end: expecting " << scopeName << ", found "
                              << m_source[m_srcIndex] << endl;
            }
            m_scopeIndex--;
            m_currentAccess = Uml::Visibility::Public;   // @todo make a stack for this
        } else {
            kError() << "importAda: too many \"end\"" << endl;
        }
        skipStmt();
        return true;
    }
    if (keyword == "function" || keyword == "procedure") {
        const QString& name = advance();
        QString returnType;
        if (advance() != "(") {
            // Unlike an Ada package, a UML package does not support
            // subprograms.
            // In order to map those, we would need to create a UML
            // class with stereotype <<utility>> for the Ada package.
            kDebug() << "ignoring parameterless " << keyword << " " << name << endl;
            skipStmt();
            return true;
        }
        UMLClassifier *klass = NULL;
        UMLOperation *op = NULL;
        const uint MAX_PARNAMES = 16;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
            QString parName[MAX_PARNAMES];
            uint parNameCount = 0;
            do {
                if (parNameCount >= MAX_PARNAMES) {
                    kError() << "MAX_PARNAMES is exceeded at " << name << endl;
                    break;
                }
                parName[parNameCount++] = advance();
            } while (advance() == ",");
            if (m_source[m_srcIndex] != ":") {
                kError() << "importAda: expecting ':'" << endl;
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
                    kError() << "importAda: cannot find UML object for " << typeName << endl;
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
                    (t != Uml::ot_Class || type->getStereotype() == "record")) {
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
                    kError() << "importAda: expecting \"return\" at function "
                        << name << endl;
                return false;
            }
            returnType = advance();
        }
        bool isAbstract = false;
        if (advance() == "is" && advance() == "abstract")
            isAbstract = true;
        if (klass != NULL && op != NULL)
            Import_Utils::insertMethod(klass, op, m_currentAccess, returnType,
                                       false, isAbstract, false, false, m_comment);
        return true;
    }
    if (keyword == "subtype") {    // FIXMEnow: potentially important but not yet implemented
        skipStmt();
        return true;
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
        return true;
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
            kError() << "importAda: expecting \"use\" at rep spec of "
                      << typeName << endl;
        }
        skipStmt();
        return true;
    }
    // At this point we're only interested in attribute declarations.
    if (m_klass == NULL || keyword == "null") {
        skipStmt();
        return true;
    }
    const QString& name = keyword;
    if (advance() != ":") {
        kError() << "adaImport: expecting \":\" at " << name << " "
                  << m_source[m_srcIndex] << endl;
        skipStmt();
        return true;
    }
    QString typeName = advance();
    QString initialValue;
    if (advance() == ":=") {
        initialValue = advance();
        QString token;
        while ((token = advance()) != ";") {
            initialValue.append(" " + token);
        }
    }
    UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                 typeName, m_comment);
    UMLAttribute *attr = static_cast<UMLAttribute*>(o);
    attr->setInitialValue(initialValue);
    skipStmt();
    return true;
}


