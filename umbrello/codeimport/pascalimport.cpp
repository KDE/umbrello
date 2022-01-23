/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
#include <QRegExp>

#include <stdio.h>

/**
 * Constructor.
 */
PascalImport::PascalImport(CodeImpThread* thread) : NativeImportBase(QLatin1String("//"), thread)
{
    setMultiLineComment(QLatin1String("(*"), QLatin1String("*)"));
    setMultiLineAltComment(QLatin1String("{"), QLatin1String("}"));
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
        if (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('#')) {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme.clear();
            }
            if (i+1 < len && c == QLatin1Char(':') && word[i + 1] == QLatin1Char('=')) {
                m_source.append(QLatin1String(":="));
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
        if (lookAhead != QLatin1String("virtual") && lookAhead != QLatin1String("abstract") &&
            lookAhead != QLatin1String("override") &&
            lookAhead != QLatin1String("register") && lookAhead != QLatin1String("cdecl") &&
            lookAhead != QLatin1String("pascal") && lookAhead != QLatin1String("stdcall") &&
            lookAhead != QLatin1String("safecall") && lookAhead != QLatin1String("saveregisters") &&
            lookAhead != QLatin1String("popstack"))
            break;
        if (lookAhead == QLatin1String("abstract"))
            isAbstract = true;
        else if (lookAhead == QLatin1String("virtual"))
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
    if (keyword == QLatin1String("uses")) {
        while (m_srcIndex < srcLength - 1) {
            QString unit = advance();
            const QString& prefix = unit.toLower();
            if (prefix == QLatin1String("sysutils") || prefix == QLatin1String("types") || prefix == QLatin1String("classes") ||
                prefix == QLatin1String("graphics") || prefix == QLatin1String("controls") || prefix == QLatin1String("strings") ||
                prefix == QLatin1String("forms") || prefix == QLatin1String("windows") || prefix == QLatin1String("messages") ||
                prefix == QLatin1String("variants") || prefix == QLatin1String("stdctrls") || prefix == QLatin1String("extctrls") ||
                prefix == QLatin1String("activex") || prefix == QLatin1String("comobj") || prefix == QLatin1String("registry") ||
                prefix == QLatin1String("classes") || prefix == QLatin1String("dialogs")) {
                if (advance() != QLatin1String(","))
                    break;
                continue;
            }
            QString filename = unit + QLatin1String(".pas");
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
            if (advance() != QLatin1String(","))
                break;
        }
        return true;
    }
    if (keyword == QLatin1String("unit")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package, name,
                                                      currentScope(), m_comment);
        pushScope(ns->asUMLPackage());
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("interface")) {
        m_inInterface = true;
        return true;
    }
    if (keyword == QLatin1String("initialization") || keyword == QLatin1String("implementation")) {
        m_inInterface = false;
        return true;
    }
    if (! m_inInterface) {
        // @todo parseStmt() should support a notion for "quit parsing, close file immediately"
        return false;
    }
    if (keyword == QLatin1String("label")) {
        m_section = sect_LABEL;
        return true;
    }
    if (keyword == QLatin1String("const")) {
        m_section = sect_CONST;
        return true;
    }
    if (keyword == QLatin1String("resourcestring")) {
        m_section = sect_RESOURCESTRING;
        return true;
    }
    if (keyword == QLatin1String("type")) {
        m_section = sect_TYPE;
        return true;
    }
    if (keyword == QLatin1String("var")) {
        m_section = sect_VAR;
        return true;
    }
    if (keyword == QLatin1String("threadvar")) {
        m_section = sect_THREADVAR;
        return true;
    }
    if (keyword == QLatin1String("automated") || keyword == QLatin1String("published")  // no concept in UML
     || keyword == QLatin1String("public")) {
        m_currentAccess = Uml::Visibility::Public;
        return true;
    }
    if (keyword == QLatin1String("protected")) {
        m_currentAccess = Uml::Visibility::Protected;
        return true;
    }
    if (keyword == QLatin1String("private")) {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == QLatin1String("packed")) {
        return true;  // TBC: perhaps this could be stored in a TaggedValue
    }
    if (keyword == QLatin1String("[")) {
        skipStmt(QLatin1String("]"));
        return true;
    }
    if (keyword == QLatin1String("end")) {
        if (m_klass) {
            m_klass = 0;
        } else if (scopeIndex()) {
            popScope();
            m_currentAccess = Uml::Visibility::Public;
        } else {
            logError2("PascalImport::parseStmt: too many \"end\" at index %1 of %2",
                      m_srcIndex, m_source.count());
        }
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("function") || keyword == QLatin1String("procedure") ||
        keyword == QLatin1String("constructor") || keyword == QLatin1String("destructor")) {
        if (m_klass == 0) {
            // Unlike a Pascal unit, a UML package does not support subprograms.
            // In order to map those, we would need to create a UML class with
            // stereotype <<utility>> for the unit, https://bugs.kde.org/89167
            bool dummyVirtual = false;
            bool dummyAbstract = false;
            checkModifiers(dummyVirtual, dummyAbstract);
            return true;
        }
        const QString& name = advance();
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        if (m_source[m_srcIndex + 1] == QLatin1String("(")) {
            advance();
            const uint MAX_PARNAMES = 16;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String(")")) {
                QString nextToken = m_source[m_srcIndex + 1].toLower();
                Uml::ParameterDirection::Enum dir = Uml::ParameterDirection::In;
                if (nextToken == QLatin1String("var")) {
                    dir = Uml::ParameterDirection::InOut;
                    advance();
                } else if (nextToken == QLatin1String("const")) {
                    advance();
                } else if (nextToken == QLatin1String("out")) {
                    dir = Uml::ParameterDirection::Out;
                    advance();
                }
                QString parName[MAX_PARNAMES];
                uint parNameCount = 0;
                do {
                    if (parNameCount >= MAX_PARNAMES) {
                        logError1("PascalImport::parseStmt: MAX_PARNAMES is exceeded at %1", name);
                        break;
                    }
                    parName[parNameCount++] = advance();
                } while (advance() == QLatin1String(","));
                if (m_source[m_srcIndex] != QLatin1String(":")) {
                    logError1("PascalImport::parseStmt: expecting ':' at %1", m_source[m_srcIndex]);
                    skipStmt();
                    break;
                }
                nextToken = advance();
                if (nextToken.toLower() == QLatin1String("array")) {
                    nextToken = advance().toLower();
                    if (nextToken != QLatin1String("of")) {
                        logError2("PascalImport::parseStmt(%1) : expecting 'array OF' at %2", name, nextToken);
                        skipStmt();
                        return false;
                    }
                    nextToken = advance();
                }
                for (uint i = 0; i < parNameCount; ++i) {
                    UMLAttribute *att = Import_Utils::addMethodParameter(op, nextToken, parName[i]);
                    att->setParmKind(dir);
                }
                if (advance() != QLatin1String(";"))
                    break;
            }
        }
        bool isConstructor = false;
        bool isDestructor = false;
        QString returnType;
        if (keyword == QLatin1String("function")) {
            if (advance() != QLatin1String(":")) {
                logError1("PascalImport::parseStmt: expecting \":\" at function %1", name);
                return false;
            }
            returnType = advance();
        } else if (keyword == QLatin1String("constructor")) {
            isConstructor = true;
        } else if (keyword == QLatin1String("destructor")) {
            isDestructor = true;
        }
        skipStmt();
        bool isVirtual = false;
        bool isAbstract = false;
        checkModifiers(isVirtual, isAbstract);
        Import_Utils::insertMethod(m_klass, op, m_currentAccess, returnType,
                                   !isVirtual, isAbstract, false, isConstructor,
                                   isDestructor, m_comment);
        return true;
    }
    if (m_section != sect_TYPE) {
        skipStmt();
        return true;
    }
    if (m_klass == 0) {
        const QString& name = m_source[m_srcIndex];
        QString nextToken = advance();
        if (nextToken != QLatin1String("=")) {
            uDebug() << name << ": expecting '=' at " << nextToken;
            return false;
        }
        keyword = advance().toLower();
        if (keyword == QLatin1String("(")) {
            // enum type
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                            name, currentScope(), m_comment);
            UMLEnum *enumType = ns->asUMLEnum();
            if (enumType == 0)
                enumType = Import_Utils::remapUMLEnum(ns, currentScope());
            while (++m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String(")")) {
                if (enumType != 0)
                    Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
                if (advance() != QLatin1String(","))
                    break;
            }
            skipStmt();
            return true;
        }
        if (keyword == QLatin1String("set")) {  // @todo implement Pascal set types
            skipStmt();
            return true;
        }
        if (keyword == QLatin1String("array")) {  // @todo implement Pascal array types
            skipStmt();
            return true;
        }
        if (keyword == QLatin1String("file")) {  // @todo implement Pascal file types
            skipStmt();
            return true;
        }
        if (keyword == QLatin1String("^")) {  // @todo implement Pascal pointer types
            skipStmt();
            return true;
        }
        if (keyword == QLatin1String("class") || keyword == QLatin1String("interface")) {
            UMLObject::ObjectType t = (keyword == QLatin1String("class") ? UMLObject::ot_Class
                                                                         : UMLObject::ot_Interface);
            UMLObject *ns = Import_Utils::createUMLObject(t, name,
                                                          currentScope(), m_comment);
            UMLClassifier *klass = ns->asUMLClassifier();
            m_comment.clear();
            QString lookAhead = m_source[m_srcIndex + 1];
            if (lookAhead == QLatin1String("(")) {
                advance();
                do {
                    QString base = advance();
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, base, 0);
                    UMLClassifier *parent = ns->asUMLClassifier();
                    m_comment.clear();
                    Import_Utils::createGeneralization(klass, parent);
                } while (advance() == QLatin1String(","));
                if (m_source[m_srcIndex] != QLatin1String(")")) {
                    logError1("PascalImport::parseStmt: expecting \")\" at %1", m_source[m_srcIndex]);
                    return false;
                }
                lookAhead = m_source[m_srcIndex + 1];
            }
            if (lookAhead == QLatin1String(";")) {
                skipStmt();
                return true;
            }
            if (lookAhead == QLatin1String("of")) {
                // @todo implement class-reference type
                return false;
            }
            m_klass = klass;
            m_currentAccess = Uml::Visibility::Public;
            return true;
        }
        if (keyword == QLatin1String("record")) {
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, name,
                                                          currentScope(), m_comment);
            ns->setStereotype(QLatin1String("record"));
            m_klass = ns->asUMLClassifier();
            return true;
        }
        if (keyword == QLatin1String("function") || keyword == QLatin1String("procedure")) {
            /*UMLObject *ns =*/ Import_Utils::createUMLObject(UMLObject::ot_Datatype, name,
                                                          currentScope(), m_comment);
            if (m_source[m_srcIndex + 1] == QLatin1String("("))
                skipToClosing(QLatin1Char('('));
            skipStmt();
            return true;
        }
        // Datatypes: TO BE DONE
        return false;
    }
    // At this point we need a class because we're expecting its member attributes.
    if (m_klass == 0) {
        uDebug() << "importPascal: skipping " << m_source[m_srcIndex];
        skipStmt();
        return true;
    }
    QString name, stereotype;
    if (keyword == QLatin1String("property")) {
        stereotype = keyword;
        name = advance();
    } else {
        name = m_source[m_srcIndex];
    }
    if (advance() != QLatin1String(":")) {
        logError2("PascalImport::parseStmt: expecting ':' at %1 %2", name, m_source[m_srcIndex]);
        skipStmt();
        return true;
    }
    QString typeName = advance();
    QString initialValue;
    if (advance() == QLatin1String("=")) {
        initialValue = advance();
        QString token;
        while ((token = advance()) != QLatin1String(";")) {
            initialValue.append(QLatin1Char(' ') + token);
        }
    }
    UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                 typeName, m_comment);
    UMLAttribute *attr = o->asUMLAttribute();
    attr->setStereotype(stereotype);
    attr->setInitialValue(initialValue);
    skipStmt();
    return true;
}


