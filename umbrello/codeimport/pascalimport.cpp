/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "pascalimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "datatype.h"
#define DBG_SRC QStringLiteral("PascalImport")
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

DEBUG_REGISTER(PascalImport)

/**
 * Constructor.
 */
PascalImport::PascalImport(CodeImpThread* thread) : NativeImportBase(QStringLiteral("//"), thread)
{
    setMultiLineComment(QStringLiteral("(*"), QStringLiteral("*)"));
    setMultiLineAltComment(QStringLiteral("{"), QStringLiteral("}"));
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
 * We need to reimplement the split operation from NativeImportBase
 * because Delphi appears to support UUIDs with syntax such as
 *    ['{23170F69-40C1-278A-0000-000500100000}']
 * which the NativeImportBase implementation does not split properly.
 */
QStringList PascalImport::split(const QString& line)
{
    // @todo implement this properly
    return NativeImportBase::split(line);
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
                m_source.append(QStringLiteral(":="));
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
        if (lookAhead != QStringLiteral("virtual") && lookAhead != QStringLiteral("abstract") &&
            lookAhead != QStringLiteral("override") &&
            lookAhead != QStringLiteral("register") && lookAhead != QStringLiteral("cdecl") &&
            lookAhead != QStringLiteral("pascal") && lookAhead != QStringLiteral("stdcall") &&
            lookAhead != QStringLiteral("safecall") && lookAhead != QStringLiteral("saveregisters") &&
            lookAhead != QStringLiteral("popstack"))
            break;
        if (lookAhead == QStringLiteral("abstract"))
            isAbstract = true;
        else if (lookAhead == QStringLiteral("virtual"))
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
    QString token   = m_source[m_srcIndex];
    QString nextTok = (m_srcIndex < srcLength-1 ? m_source[m_srcIndex + 1] : QStringLiteral("!END!"));
    QString keyword = token.toLower();
    logDebug3("PascalImport::parseStmt : %1 (%2) index %3", token, nextTok, m_srcIndex);
    if (keyword == QStringLiteral("uses")) {
        while (m_srcIndex < srcLength - 1) {
            QString unit = advance();
            const QString& prefix = unit.toLower();
            if (prefix == QStringLiteral("sysutils") || prefix == QStringLiteral("types") || prefix == QStringLiteral("classes") ||
                prefix == QStringLiteral("graphics") || prefix == QStringLiteral("controls") || prefix == QStringLiteral("strings") ||
                prefix == QStringLiteral("forms") || prefix == QStringLiteral("windows") || prefix == QStringLiteral("messages") ||
                prefix == QStringLiteral("variants") || prefix == QStringLiteral("stdctrls") || prefix == QStringLiteral("extctrls") ||
                prefix == QStringLiteral("activex") || prefix == QStringLiteral("comobj") || prefix == QStringLiteral("registry") ||
                prefix == QStringLiteral("classes") || prefix == QStringLiteral("dialogs")) {
                if (advance() != QStringLiteral(","))
                    break;
                continue;
            }
            QString filename = unit + QStringLiteral(".pas");
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
            if (advance() != QStringLiteral(","))
                break;
        }
        skipStmt();
        return true;
    }
    if (keyword == QStringLiteral("unit")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package, name,
                                                      currentScope(), m_comment);
        pushScope(ns->asUMLPackage());
        skipStmt();
        return true;
    }
    if (keyword == QStringLiteral("interface")) {
        m_inInterface = true;
        return true;
    }
    if (keyword == QStringLiteral("initialization") || keyword == QStringLiteral("implementation")) {
        m_inInterface = false;
        return true;
    }
    if (! m_inInterface) {
        // @todo parseStmt() should support a notion for "quit parsing, close file immediately"
        return false;
    }
    if (keyword == QStringLiteral("label")) {
        m_section = sect_LABEL;
        return true;
    }
    if (keyword == QStringLiteral("const")) {
        m_section = sect_CONST;
        return true;
    }
    if (keyword == QStringLiteral("resourcestring")) {
        m_section = sect_RESOURCESTRING;
        return true;
    }
    if (keyword == QStringLiteral("type")) {
        m_section = sect_TYPE;
        return true;
    }
    if (keyword == QStringLiteral("var")) {
        m_section = sect_VAR;
        return true;
    }
    if (keyword == QStringLiteral("threadvar")) {
        m_section = sect_THREADVAR;
        return true;
    }
    if (keyword == QStringLiteral("automated") || keyword == QStringLiteral("published")  // no classifier in UML
     || keyword == QStringLiteral("public")) {
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
    if (keyword == QStringLiteral("packed")) {
        return true;  // TBC: perhaps this could be stored in a TaggedValue
    }
    if (keyword == QStringLiteral("[")) {
        //skipStmt(QStringLiteral("]"));
        //  Not using skipStmt here because the closing bracket may be glued on to
        //  some other character(s), e.g. ']  (quote)
        // This is an imperfection in the token splitter.
        // @todo flesh out function split() which should be reimplemented from NativeImportBase
        while (m_srcIndex < m_source.count()) {
            if (advance().endsWith("]"))
                break;
        }
        return true;
    }
    if (keyword == QStringLiteral("end")) {
        if (m_klass) {
            m_klass = nullptr;
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
    if (keyword == QStringLiteral("function") || keyword == QStringLiteral("procedure") ||
        keyword == QStringLiteral("constructor") || keyword == QStringLiteral("destructor")) {
        if (m_klass == nullptr) {
            // Unlike a Pascal unit, a UML package does not support subprograms.
            // In order to map those, we would need to create a UML class with
            // stereotype <<utility>> for the unit, https://bugs.kde.org/89167
            bool dummyVirtual = false;
            bool dummyAbstract = false;
            checkModifiers(dummyVirtual, dummyAbstract);
            skipStmt();
            return true;
        }
        const QString& name = advance();
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        if (m_source[m_srcIndex + 1] == QStringLiteral("(")) {
            advance();
            const uint MAX_PARNAMES = 16;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != QStringLiteral(")")) {
                QString nextToken = m_source[m_srcIndex + 1].toLower();
                Uml::ParameterDirection::Enum dir = Uml::ParameterDirection::In;
                if (nextToken == QStringLiteral("var")) {
                    dir = Uml::ParameterDirection::InOut;
                    advance();
                } else if (nextToken == QStringLiteral("const")) {
                    advance();
                } else if (nextToken == QStringLiteral("out")) {
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
                } while (advance() == QStringLiteral(","));
                if (m_source[m_srcIndex] != QStringLiteral(":")) {
                    logError1("PascalImport::parseStmt: expecting ':' at %1", m_source[m_srcIndex]);
                    skipStmt();
                    break;
                }
                nextToken = advance();
                if (nextToken.toLower() == QStringLiteral("array")) {
                    nextToken = advance().toLower();
                    if (nextToken != QStringLiteral("of")) {
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
                if (advance() != QStringLiteral(";"))
                    break;
            }
        }
        bool isConstructor = false;
        bool isDestructor = false;
        QString returnType;
        if (keyword == QStringLiteral("function")) {
            if (advance() != QStringLiteral(":")) {
                logError1("PascalImport::parseStmt: expecting \":\" at function %1", name);
                return false;
            }
            returnType = advance();
        } else if (keyword == QStringLiteral("constructor")) {
            isConstructor = true;
        } else if (keyword == QStringLiteral("destructor")) {
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
    if (m_klass == nullptr) {
        const QString& name = m_source[m_srcIndex];
        QString nextToken = advance();
        if (nextToken != QStringLiteral("=")) {
            logDebug2("PascalImport::parseStmt %1: expecting '=' at %2", name, nextToken);
            return false;
        }
        QString rhsName = advance();
        keyword = rhsName.toLower();
        if (keyword == QStringLiteral("(")) {
            // enum type
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                            name, currentScope(), m_comment);
            UMLEnum *enumType = ns->asUMLEnum();
            if (enumType == nullptr)
                enumType = Import_Utils::remapUMLEnum(ns, currentScope());
            while (++m_srcIndex < srcLength && m_source[m_srcIndex] != QStringLiteral(")")) {
                if (enumType != nullptr)
                    Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
                if (advance() != QStringLiteral(","))
                    break;
            }
            skipStmt();
            return true;
        }
        if (keyword == QStringLiteral("set")) {  // @todo implement Pascal set types
            skipStmt();
            return true;
        }
        if (keyword == QStringLiteral("array")) {  // @todo implement Pascal array types
            skipStmt();
            return true;
        }
        if (keyword == QStringLiteral("file")) {  // @todo implement Pascal file types
            skipStmt();
            return true;
        }
        if (keyword == QStringLiteral("^")) {  // @todo implement Pascal pointer types
            skipStmt();
            return true;
        }
        if (keyword == QStringLiteral("class") || keyword == QStringLiteral("interface")) {
            UMLObject::ObjectType t = (keyword == QStringLiteral("class") ? UMLObject::ot_Class
                                                                         : UMLObject::ot_Interface);
            UMLObject *ns = Import_Utils::createUMLObject(t, name,
                                                          currentScope(), m_comment);
            UMLClassifier *klass = ns->asUMLClassifier();
            m_comment.clear();
            QString lookAhead = m_source[m_srcIndex + 1];
            if (lookAhead == QStringLiteral("(")) {
                advance();
                do {
                    QString base = advance();
                    UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, base, nullptr);
                    UMLClassifier *parent = ns->asUMLClassifier();
                    m_comment.clear();
                    Import_Utils::createGeneralization(klass, parent);
                } while (advance() == QStringLiteral(","));
                if (m_source[m_srcIndex] != QStringLiteral(")")) {
                    logError1("PascalImport::parseStmt: expecting \")\" at %1", m_source[m_srcIndex]);
                    return false;
                }
                lookAhead = m_source[m_srcIndex + 1];
            }
            if (lookAhead == QStringLiteral(";")) {
                skipStmt();
                return true;
            }
            if (lookAhead == QStringLiteral("of")) {
                // @todo implement class-reference type
                return false;
            }
            m_klass = klass;
            m_currentAccess = Uml::Visibility::Public;
            return true;
        }
        if (keyword == QStringLiteral("record")) {
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, name,
                                                          currentScope(), m_comment);
            ns->setStereotype(QStringLiteral("record"));
            m_klass = ns->asUMLClassifier();
            return true;
        }
        if (keyword == QStringLiteral("function") || keyword == QStringLiteral("procedure")) {
            /*UMLObject *ns =*/ Import_Utils::createUMLObject(UMLObject::ot_Datatype, name,
                                                          currentScope(), m_comment);
            if (m_source[m_srcIndex + 1] == QStringLiteral("("))
                skipToClosing(QLatin1Char('('));
            skipStmt();
            return true;
        }
        // Datatypes
        UMLObject *o = nullptr;
        UMLDoc *umldoc = UMLApp::app()->document();
        UMLDatatype *newType = umldoc->findDatatype(name);
        if (!newType) {
            o = Import_Utils::createUMLObject(UMLObject::ot_Datatype, name,
                                              currentScope(), m_comment);
            newType = dynamic_cast<UMLDatatype*>(o);
        }
        if (!newType) {
            logError1("PascalImport::parseStmt: Finding/creating datatype %1 failed", name);
            skipStmt();
            return false;
        }
        /* TODO: Create <<typedef>> from originType to newType
        UMLDatatype *originType = umldoc->findDatatype(rhsName);
        if (originType) {
           ....
        } */
        o = newType;
        skipStmt();
        return true;
    }
    // At this point we need a class because we're expecting its member attributes.
    if (m_klass == nullptr) {
        logDebug1("PascalImport::parseStmt: skipping %1", m_source[m_srcIndex]);
        skipStmt();
        return true;
    }
    QString name, stereotype;
    if (keyword == QStringLiteral("property")) {
        stereotype = keyword;
        name = advance();
    } else {
        name = m_source[m_srcIndex];
    }
    if (advance() != QStringLiteral(":")) {
        logError2("PascalImport::parseStmt: expecting ':' at %1 %2", name, m_source[m_srcIndex]);
        skipStmt();
        return true;
    }
    QString typeName = advance();
    QString initialValue;
    if (advance() == QStringLiteral("=")) {
        initialValue = advance();
        QString token;
        while ((token = advance()) != QStringLiteral(";")) {
            initialValue.append(QLatin1Char(' ') + token);
        }
    }
    UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                 typeName, m_comment);
    if (!o) {
        logDebug2("PascalImport::parseStmt: Could not insert attribute %1 in class %2",
                  name, m_klass->name());
        return false;
    }
    UMLAttribute *attr = o->asUMLAttribute();
    if (!attr) {
        logDebug2("PascalImport::parseStmt: insertAttribute returned different object named %1 in class %2",
                  name, m_klass->name());
        return false;
    }
    attr->setStereotype(stereotype);
    attr->setInitialValue(initialValue);
    skipStmt();
    return true;
}


