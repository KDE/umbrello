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
#include "pascalimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "operation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"

// qt includes
#include <QtCore/QRegExp>

#include <stdio.h>

/**
 * Constructor.
 */
PascalImport::PascalImport(CodeImpThread* thread) : NativeImportBase("//", thread)
{
    setMultiLineComment("(*", "*)");
    setMultiLineAltComment("{", "}");
    initVars();
}

/**
 * Destructor.
 */
PascalImport::~PascalImport()
{
}

/**
 * Reimplement operation from NativeImportBase.
 */
void PascalImport::initVars()
{
    m_inInterface = false;
    m_section = sect_NONE;
    NativeImportBase::m_currentAccess = Uml::Visibility::Public;
}

/**
 * Implement abstract operation from NativeImportBase.
 */
void PascalImport::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        QChar c = word[i];
        if (c.isLetterOrNumber() || c == '_' || c == '.' || c == '#') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme.clear();
            }
            if (i+1 < len && c == ':' && word[i + 1] == '=') {
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

/**
 * Check for, and skip over, all modifiers following a method.
 * Set the output arguments on encountering abstract and/or virtual.
 * @param isVirtual   return value, set to true when "virtual" seen
 * @param isAbstract  return value, set to true when "abstract" seen
 */
void PascalImport::checkModifiers(bool& isVirtual, bool& isAbstract)
{
    const int srcLength = m_source.count();
    while (m_srcIndex < srcLength - 1) {
        QString lookAhead = m_source[m_srcIndex + 1].toLower();
        if (lookAhead != "virtual" && lookAhead != "abstract" &&
            lookAhead != "override" &&
            lookAhead != "register" && lookAhead != "cdecl" &&
            lookAhead != "pascal" && lookAhead != "stdcall" &&
            lookAhead != "safecall" && lookAhead != "saveregisters" &&
            lookAhead != "popstack")
            break;
        if (lookAhead == "abstract")
            isAbstract = true;
        else if (lookAhead == "virtual")
            isVirtual = true;
        advance();
        skipStmt();
    }
}

/**
 * Implement abstract operation from NativeImportBase.
 * @return success status of operation
 */
bool PascalImport::parseStmt()
{
    const int srcLength = m_source.count();
    QString keyword = m_source[m_srcIndex].toLower();
    //uDebug() << '"' << keyword << '"';
    if (keyword == "uses") {
        while (m_srcIndex < srcLength - 1) {
            QString unit = advance();
            const QString& prefix = unit.toLower();
            if (prefix == "sysutils" || prefix == "types" || prefix == "classes" ||
                prefix == "graphics" || prefix == "controls" || prefix == "strings" ||
                prefix == "forms" || prefix == "windows" || prefix == "messages" ||
                prefix == "variants" || prefix == "stdctrls" || prefix == "extctrls" ||
                prefix == "activex" || prefix == "comobj" || prefix == "registry" ||
                prefix == "classes" || prefix == "dialogs") {
                if (advance() != ",")
                    break;
                continue;
            }
            QString filename = unit + ".pas";
            if (! m_parsedFiles.contains(unit)) {
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
            if (advance() != ",")
                break;
        }
        return true;
    }
    if (keyword == "unit") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package, name,
                                                      m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
        skipStmt();
        return true;
    }
    if (keyword == "interface") {
        m_inInterface = true;
        return true;
    }
    if (keyword == "initialization" || keyword == "implementation") {
        m_inInterface = false;
        return true;
    }
    if (! m_inInterface) {
        // @todo parseStmt() should support a notion for "quit parsing, close file immediately"
        return false;
    }
    if (keyword == "label") {
        m_section = sect_LABEL;
        return true;
    }
    if (keyword == "const") {
        m_section = sect_CONST;
        return true;
    }
    if (keyword == "resourcestring") {
        m_section = sect_RESOURCESTRING;
        return true;
    }
    if (keyword == "type") {
        m_section = sect_TYPE;
        return true;
    }
    if (keyword == "var") {
        m_section = sect_VAR;
        return true;
    }
    if (keyword == "threadvar") {
        m_section = sect_THREADVAR;
        return true;
    }
    if (keyword == "automated" || keyword == "published"  // no concept in UML
     || keyword == "public") {
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
    if (keyword == "packed") {
        return true;  // TBC: perhaps this could be stored in a TaggedValue
    }
    if (keyword == "[") {
        skipStmt("]");
        return true;
    }
    if (keyword == "end") {
        if (m_klass) {
            m_klass = NULL;
        } else if (m_scopeIndex) {
            m_scopeIndex--;
            m_currentAccess = Uml::Visibility::Public;
        } else {
            uError() << "importPascal: too many \"end\"";
        }
        skipStmt();
        return true;
    }
    if (keyword == "function" || keyword == "procedure" ||
        keyword == "constructor" || keyword == "destructor") {
        if (m_klass == NULL) {
            // Unlike a Pascal unit, a UML package does not support subprograms.
            // In order to map those, we would need to create a UML class with
            // stereotype <<utility>> for the unit, http://bugs.kde.org/89167
            bool dummyVirtual = false;
            bool dummyAbstract = false;
            checkModifiers(dummyVirtual, dummyAbstract);
            return true;
        }
        const QString& name = advance();
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        if (m_source[m_srcIndex + 1] == "(") {
            advance();
            const uint MAX_PARNAMES = 16;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
                QString nextToken = m_source[m_srcIndex + 1].toLower();
                Uml::Parameter_Direction dir = Uml::pd_In;
                if (nextToken == "var") {
                    dir = Uml::pd_InOut;
                    advance();
                } else if (nextToken == "const") {
                    advance();
                } else if (nextToken == "out") {
                    dir = Uml::pd_Out;
                    advance();
                }
                QString parName[MAX_PARNAMES];
                uint parNameCount = 0;
                do {
                    if (parNameCount >= MAX_PARNAMES) {
                        uError() << "MAX_PARNAMES is exceeded at " << name;
                        break;
                    }
                    parName[parNameCount++] = advance();
                } while (advance() == ",");
                if (m_source[m_srcIndex] != ":") {
                    uError() << "importPascal: expecting ':' at " << m_source[m_srcIndex];
                    skipStmt();
                    break;
                }
                nextToken = advance();
                if (nextToken.toLower() == "array") {
                    nextToken = advance().toLower();
                    if (nextToken != "of") {
                        uError() << "importPascal(" << name << "): expecting 'array OF' at "
                                  << nextToken;
                        skipStmt();
                        return false;
                    }
                    nextToken = advance();
                }
                for (uint i = 0; i < parNameCount; ++i) {
                    UMLAttribute *att = Import_Utils::addMethodParameter(op, nextToken, parName[i]);
                    att->setParmKind(dir);
                }
                if (advance() != ";")
                    break;
            }
        }
        QString returnType;
        if (keyword == "function") {
            if (advance() != ":") {
                uError() << "importPascal: expecting \":\" at function "
                        << name;
                return false;
            }
            returnType = advance();
        } else if (keyword == "constructor" || keyword == "destructor") {
            op->setStereotype(keyword);
        }
        skipStmt();
        bool isVirtual = false;
        bool isAbstract = false;
        checkModifiers(isVirtual, isAbstract);
        Import_Utils::insertMethod(m_klass, op, m_currentAccess, returnType,
                                   !isVirtual, isAbstract, false, false, m_comment);
        return true;
    }
    if (m_section != sect_TYPE) {
        skipStmt();
        return true;
    }
    if (m_klass == NULL) {
        const QString& name = m_source[m_srcIndex];
        QString nextToken = advance();
        if (nextToken != "=") {
            uDebug() << name << ": expecting '=' at " << nextToken;
            return false;
        }
        keyword = advance().toLower();
        if (keyword == "(") {
            // enum type
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
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
        if (keyword == "set") {  // @todo implement Pascal set types
            skipStmt();
            return true;
        }
        if (keyword == "array") {  // @todo implement Pascal array types
            skipStmt();
            return true;
        }
        if (keyword == "file") {  // @todo implement Pascal file types
            skipStmt();
            return true;
        }
        if (keyword == "^") {  // @todo implement Pascal pointer types
            skipStmt();
            return true;
        }
        if (keyword == "class" || keyword == "interface") {
            UMLObject::ObjectType t = (keyword == "class" ? UMLObject::ot_Class : UMLObject::ot_Interface);
            UMLObject *ns = Import_Utils::createUMLObject(t, name,
                                                          m_scope[m_scopeIndex], m_comment);
            UMLClassifier *klass = static_cast<UMLClassifier*>(ns);
            m_comment.clear();
            QString lookAhead = m_source[m_srcIndex + 1];
            if (lookAhead == "(") {
                advance();
                do {
                    QString base = advance();
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, base, NULL);
                    UMLClassifier *parent = static_cast<UMLClassifier*>(ns);
                    m_comment.clear();
                    Import_Utils::createGeneralization(klass, parent);
                } while (advance() == ",");
                if (m_source[m_srcIndex] != ")") {
                    uError() << "PascalImport: expecting \")\" at "
                        << m_source[m_srcIndex];
                    return false;
                }
                lookAhead = m_source[m_srcIndex + 1];
            }
            if (lookAhead == ";") {
                skipStmt();
                return true;
            }
            if (lookAhead == "of") {
                // @todo implement class-reference type
                return false;
            }
            m_klass = klass;
            m_currentAccess = Uml::Visibility::Public;
            return true;
        }
        if (keyword == "record") {
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, name,
                                                          m_scope[m_scopeIndex], m_comment);
            ns->setStereotype("record");
            m_klass = static_cast<UMLClassifier*>(ns);
            return true;
        }
        if (keyword == "function" || keyword == "procedure") {
            /*UMLObject *ns =*/ Import_Utils::createUMLObject(UMLObject::ot_Datatype, name,
                                                          m_scope[m_scopeIndex], m_comment);
            if (m_source[m_srcIndex + 1] == "(")
                skipToClosing('(');
            skipStmt();
            return true;
        }
        // Datatypes: TO BE DONE
        return false;
    }
    // At this point we need a class because we're expecting its member attributes.
    if (m_klass == NULL) {
        uDebug() << "importPascal: skipping " << m_source[m_srcIndex];
        skipStmt();
        return true;
    }
    QString name, stereotype;
    if (keyword == "property") {
        stereotype = keyword;
        name = advance();
    } else {
        name = m_source[m_srcIndex];
    }
    if (advance() != ":") {
        uError() << "PascalImport: expecting \":\" at " << name << " "
                 << m_source[m_srcIndex];
        skipStmt();
        return true;
    }
    QString typeName = advance();
    QString initialValue;
    if (advance() == "=") {
        initialValue = advance();
        QString token;
        while ((token = advance()) != ";") {
            initialValue.append(' ' + token);
        }
    }
    UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                 typeName, m_comment);
    UMLAttribute *attr = static_cast<UMLAttribute*>(o);
    attr->setStereotype(stereotype);
    attr->setInitialValue(initialValue);
    skipStmt();
    return true;
}


