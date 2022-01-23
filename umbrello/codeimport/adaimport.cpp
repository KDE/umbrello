/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "adaimport.h"

// app includes
#include "association.h"
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "enum.h"
#include "folder.h"
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
AdaImport::AdaImport(CodeImpThread* thread)
  : NativeImportBase(QLatin1String("--"), thread)
{
    initVars();
}

/**
 * Destructor.
 */
AdaImport::~AdaImport()
{
}

/**
 * Reimplement operation from NativeImportBase.
 */
void AdaImport::initVars()
{
    m_inGenericFormalPart = false;
    m_classesDefinedInThisScope.clear();
    m_renaming.clear();
}

/**
 * Split the line so that a string is returned as a single element of the list.
 * When not in a string then split at white space.
 * Reimplementation of method from NativeImportBase is required because of
 * Ada's tic which is liable to be confused with the beginning of a character
 * constant.
 */
QStringList AdaImport::split(const QString& lin)
{
    QStringList list;
    QString listElement;
    bool inString = false;
    bool seenSpace = false;
    QString line = lin.trimmed();
    uint len = line.length();
    for (uint i = 0; i < len; ++i) {
        const QChar& c = line[i];
        if (inString) {
            listElement += c;
            if (c == QLatin1Char('"')) {
                if (i < len - 1 && line[i + 1] == QLatin1Char('"')) {
                    i++;     // escaped quotation mark
                    continue;
                }
                list.append(listElement);
                listElement.clear();
                inString = false;
            }
        } else if (c == QLatin1Char('"')) {
            inString = true;
            if (!listElement.isEmpty())
                list.append(listElement);
            listElement = QString(c);
            seenSpace = false;
        } else if (c == QLatin1Char('\'')) {
            if (i < len - 2 && line[i + 2] == QLatin1Char('\'')) {
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

/**
 * Implement abstract operation from NativeImportBase.
 */
void AdaImport::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        QChar c = word[i];
        if (c.isLetterOrNumber() || c == QLatin1Char('_')
                                 || c == QLatin1Char('.') || c == QLatin1Char('#')) {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme.clear();
            }
            if (c == QLatin1Char(':') && i < len - 1 && word[i + 1] == QLatin1Char('=')) {
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
 * Apply package renamings to the given name.
 * @return  expanded name
 */
QString AdaImport::expand(const QString& name)
{
    QRegExp pfxRegExp(QLatin1String("^(\\w+)\\."));
    pfxRegExp.setCaseSensitivity(Qt::CaseInsensitive);
    int pos = pfxRegExp.indexIn(name);
    if (pos == -1)
        return name;
    QString result = name;
    QString pfx = pfxRegExp.cap(1);
    if (m_renaming.contains(pfx)) {
        result.remove(pfxRegExp);
        result.prepend(m_renaming[pfx] + QLatin1Char('.'));
    }
    return result;
}

/**
 * Parse all files that can be formed by concatenation of the given stems.
 */
void AdaImport::parseStems(const QStringList& stems)
{
    if (stems.isEmpty())
        return;
    QString base = stems.first();
    int i = 0;
    while (1) {
        QString filename = base + QLatin1String(".ads");
        if (! m_parsedFiles.contains(filename)) {
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
        if (++i >= stems.count())
            break;
        base += QLatin1Char('-') + stems[i];
    }
}

/**
 * Implement abstract operation from NativeImportBase.
 */
bool AdaImport::parseStmt()
{
    const int srcLength = m_source.count();
    QString keyword = m_source[m_srcIndex];
    UMLDoc *umldoc = UMLApp::app()->document();
    //uDebug() << '"' << keyword << '"';
    if (keyword == QLatin1String("with")) {
        if (m_inGenericFormalPart) {
            // mapping of generic formal subprograms or packages is not yet implemented
            return false;
        }
        while (++m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String(";")) {
            QStringList components = m_source[m_srcIndex].toLower().split(QLatin1Char('.'));
            const QString& prefix = components.first();
            if (prefix == QLatin1String("system")
                || prefix == QLatin1String("ada")
                || prefix == QLatin1String("gnat")
                || prefix == QLatin1String("interfaces")
                || prefix == QLatin1String("text_io")
                || prefix == QLatin1String("unchecked_conversion")
                || prefix == QLatin1String("unchecked_deallocation")) {
                if (advance() != QLatin1String(","))
                    break;
                continue;
            }
            parseStems(components);
            if (advance() != QLatin1String(","))
                break;
        }
        return true;
    }
    if (keyword == QLatin1String("generic")) {
        m_inGenericFormalPart = true;
        return true;
    }
    if (keyword == QLatin1String("package")) {
        const QString& name = advance();
        QStringList parentPkgs = name.toLower().split(QLatin1Char('.'));
        parentPkgs.pop_back();  // exclude the current package
        parseStems(parentPkgs);
        UMLObject *ns = 0;
        if (advance() == QLatin1String("is")) {
            ns = Import_Utils::createUMLObject(UMLObject::ot_Package, name,
                                               currentScope(), m_comment);
            if (m_source[m_srcIndex + 1] == QLatin1String("new")) {
                m_srcIndex++;
                QString pkgName = advance();
                UMLObject *gp = Import_Utils::createUMLObject(UMLObject::ot_Package, pkgName,
                                                              currentScope());
                gp->setStereotype(QLatin1String("generic"));
                // Add binding from instantiator to instantiatee
                UMLAssociation *assoc = new UMLAssociation(Uml::AssociationType::Dependency, ns, gp);
                assoc->setUMLPackage(umldoc->rootFolder(Uml::ModelType::Logical));
                assoc->setStereotype(QLatin1String("bind"));
                // Work around missing display of stereotype in AssociationWidget:
                assoc->setName(assoc->stereotype(true));
                umldoc->addAssociation(assoc);
                skipStmt();
            } else {
                pushScope(ns->asUMLPackage());
            }
        } else if (m_source[m_srcIndex] == QLatin1String("renames")) {
            m_renaming[name] = advance();
        } else {
            logError1("AdaImport::parseStmt unexpected: %1", m_source[m_srcIndex]);
            skipStmt(QLatin1String("is"));
        }
        if (m_inGenericFormalPart) {
            if (ns)
                ns->setStereotype(QLatin1String("generic"));
            m_inGenericFormalPart = false;
        }
        return true;
    }
    if (m_inGenericFormalPart)
        return false;  // skip generic formal parameter (not yet implemented)
    if (keyword == QLatin1String("subtype")) {
        QString name = advance();
        advance();  // "is"
        QString base = expand(advance());
        base.remove(QLatin1String("Standard."), Qt::CaseInsensitive);
        UMLObject *type = umldoc->findUMLObject(base, UMLObject::ot_UMLObject, currentScope());
        if (type == 0) {
            type = Import_Utils::createUMLObject(UMLObject::ot_Datatype, base, currentScope());
        }
        UMLObject *subtype = Import_Utils::createUMLObject(type->baseType(), name,
                                                           currentScope(), m_comment);
        UMLAssociation *assoc = new UMLAssociation(Uml::AssociationType::Dependency, subtype, type);
        assoc->setUMLPackage(umldoc->rootFolder(Uml::ModelType::Logical));
        assoc->setStereotype(QLatin1String("subtype"));
        // Work around missing display of stereotype in AssociationWidget:
        assoc->setName(assoc->stereotype(true));
        umldoc->addAssociation(assoc);
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("type")) {
        QString name = advance();
        QString next = advance();
        if (next == QLatin1String("(")) {
            uDebug() << name << ": discriminant handling is not yet implemented";
            // @todo Find out how to map discriminated record to UML.
            //       For now, we just create a pro forma empty record.
            Import_Utils::createUMLObject(UMLObject::ot_Class, name, currentScope(),
                                          m_comment, QLatin1String("record"));
            skipStmt(QLatin1String("end"));
            if ((next = advance()) == QLatin1String("case"))
                m_srcIndex += 2;  // skip "case" ";"
            skipStmt();
            return true;
        }
        if (next == QLatin1String(";")) {
            // forward declaration
            Import_Utils::createUMLObject(UMLObject::ot_Class, name, currentScope(),
                                          m_comment);
            return true;
        }
        if (next != QLatin1String("is")) {
            logError1("AdaImport::parseStmt: expecting \"is\" at %1", next);
            return false;
        }
        next = advance();
        if (next == QLatin1String("(")) {
            // enum type
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                            name, currentScope(), m_comment);
            UMLEnum *enumType = ns->asUMLEnum();
            if (enumType == 0)
                enumType = Import_Utils::remapUMLEnum(ns, enumType);
            while ((next = advance()) != QLatin1String(")")) {
                if (enumType != 0)
                    Import_Utils::addEnumLiteral(enumType, next, m_comment);
                m_comment.clear();
                if (advance() != QLatin1String(","))
                    break;
            }
            skipStmt();
            return true;
        }
        bool isTaggedType = false;
        if (next == QLatin1String("abstract")) {
            m_isAbstract = true;
            next = advance();
        }
        if (next == QLatin1String("tagged")) {
            isTaggedType = true;
            next = advance();
        }
        if (next == QLatin1String("limited") ||
            next == QLatin1String("task") ||
            next == QLatin1String("protected") ||
            next == QLatin1String("synchronized")) {
            next = advance();  // we can't (yet?) represent that
        }
        if (next == QLatin1String("private") ||
            next == QLatin1String("interface") ||
            next == QLatin1String("record") ||
            (next == QLatin1String("null") &&
             m_source[m_srcIndex+1] == QLatin1String("record"))) {
            UMLObject::ObjectType t = (next == QLatin1String("interface") ? UMLObject::ot_Interface
                                                                          : UMLObject::ot_Class);
            UMLObject *ns = Import_Utils::createUMLObject(t, name, currentScope(), m_comment);
            if (t == UMLObject::ot_Interface) {
                while ((next = advance()) == QLatin1String("and")) {
                    UMLClassifier *klass = ns->asUMLClassifier();
                    QString base = expand(advance());
                    UMLObject *p = Import_Utils::createUMLObject(UMLObject::ot_Interface, base, currentScope());
                    UMLClassifier *parent = p->asUMLClassifier();
                    Import_Utils::createGeneralization(klass, parent);
                }
            } else {
                ns->setAbstract(m_isAbstract);
            }
            m_isAbstract = false;
            if (isTaggedType) {
                if (! m_classesDefinedInThisScope.contains(ns))
                    m_classesDefinedInThisScope.append(ns);
            } else {
                ns->setStereotype(QLatin1String("record"));
            }
            if (next == QLatin1String("record"))
                m_klass = ns->asUMLClassifier();
            else
                skipStmt();
            return true;
        }
        if (next == QLatin1String("new")) {
            QString base = expand(advance());
            QStringList baseInterfaces;
            while ((next = advance()) == QLatin1String("and")) {
                baseInterfaces.append(expand(advance()));
            }
            const bool isExtension = (next == QLatin1String("with"));
            UMLObject::ObjectType t;
            if (isExtension || m_isAbstract) {
                t = UMLObject::ot_Class;
            } else {
                base.remove(QLatin1String("Standard."), Qt::CaseInsensitive);
                UMLObject *known = umldoc->findUMLObject(base, UMLObject::ot_UMLObject, currentScope());
                t = (known ? known->baseType() : UMLObject::ot_Datatype);
            }
            UMLObject *ns = Import_Utils::createUMLObject(t, base, 0);
            UMLClassifier *parent = ns->asUMLClassifier();
            ns = Import_Utils::createUMLObject(t, name, currentScope(), m_comment);
            if (isExtension) {
                next = advance();
                if (next == QLatin1String("null") || next == QLatin1String("record")) {
                    UMLClassifier *klass = ns->asUMLClassifier();
                    Import_Utils::createGeneralization(klass, parent);
                    if (next == QLatin1String("record")) {
                        // Set the m_klass for attributes.
                        m_klass = klass;
                    }
                    if (baseInterfaces.count()) {
                        t = UMLObject::ot_Interface;
                        QStringList::Iterator end(baseInterfaces.end());
                        for (QStringList::Iterator bi(baseInterfaces.begin()); bi != end; ++bi) {
                             ns = Import_Utils::createUMLObject(t, *bi, currentScope());
                             parent = ns->asUMLClassifier();
                             Import_Utils::createGeneralization(klass, parent);
                        }
                    }
                }
            }
            skipStmt();
            return true;
        }
        // Datatypes: TO BE DONE
        return false;
    }
    if (keyword == QLatin1String("private")) {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == QLatin1String("end")) {
        if (m_klass) {
            if (advance() != QLatin1String("record")) {
                logError1("AdaImport::parseStmt end: expecting \"record\" at %1",
                          m_source[m_srcIndex]);
            }
            m_klass = 0;
        } else if (scopeIndex()) {
            if (advance() != QLatin1String(";")) {
                QString scopeName = currentScope()->fullyQualifiedName();
                if (scopeName.toLower() != m_source[m_srcIndex].toLower())
                    logError2("AdaImport::parseStmt end: expecting %1, found %2",
                              scopeName, m_source[m_srcIndex]);
            }
            popScope();
            m_currentAccess = Uml::Visibility::Public;   // @todo make a stack for this
        } else {
            logError1("AdaImport::parseStmt: too many \"end\" at index %1", m_srcIndex);
        }
        skipStmt();
        return true;
    }
    // subprogram
    if (keyword == QLatin1String("not"))
        keyword = advance();
    if (keyword == QLatin1String("overriding"))
        keyword = advance();
    if (keyword == QLatin1String("function") || keyword == QLatin1String("procedure")) {
        const QString& name = advance();
        QString returnType;
        if (advance() != QLatin1String("(")) {
            // Unlike an Ada package, a UML package does not support
            // subprograms.
            // In order to map those, we would need to create a UML
            // class with stereotype <<utility>> for the Ada package.
            uDebug() << "ignoring parameterless " << keyword << " " << name;
            skipStmt();
            return true;
        }
        UMLClassifier *klass = 0;
        UMLOperation *op = 0;
        const uint MAX_PARNAMES = 16;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String(")")) {
            QString parName[MAX_PARNAMES];
            uint parNameCount = 0;
            do {
                if (parNameCount >= MAX_PARNAMES) {
                    logError1("AdaImport::parseStmt: MAX_PARNAMES is exceeded at %1", name);
                    break;
                }
                parName[parNameCount++] = advance();
            } while (advance() == QLatin1String(","));
            if (m_source[m_srcIndex] != QLatin1String(":")) {
                logError2("AdaImport::parseStmt: expecting ':' at %1 (index %2)",
                          m_source[m_srcIndex], m_srcIndex);
                skipStmt();
                break;
            }
            const QString &direction = advance();
            QString typeName;
            Uml::ParameterDirection::Enum dir = Uml::ParameterDirection::In;
            if (direction == QLatin1String("access")) {
                // Oops, we have to improvise here because there
                // is no such thing as "access" in UML.
                // So we use the next best thing, "inout".
                // Better ideas, anyone?
                dir = Uml::ParameterDirection::InOut;
                typeName = advance();
            } else if (direction == QLatin1String("in")) {
                if (m_source[m_srcIndex + 1] == QLatin1String("out")) {
                    dir = Uml::ParameterDirection::InOut;
                    m_srcIndex++;
                }
                typeName = advance();
            } else if (direction == QLatin1String("out")) {
                dir = Uml::ParameterDirection::Out;
                typeName = advance();
            } else {
                typeName = direction;  // In Ada, the default direction is "in"
            }
            typeName.remove(QLatin1String("Standard."), Qt::CaseInsensitive);
            typeName = expand(typeName);
            if (op == 0) {
                // In Ada, the first parameter indicates the class.
                UMLObject *type = Import_Utils::createUMLObject(UMLObject::ot_Class, typeName, currentScope());
                UMLObject::ObjectType t = type->baseType();
                if ((t != UMLObject::ot_Interface &&
                     (t != UMLObject::ot_Class || type->stereotype() == QLatin1String("record"))) ||
                    !m_classesDefinedInThisScope.contains(type)) {
                    // Not an instance bound method - we cannot represent it.
                    skipStmt(QLatin1String(")"));
                    break;
                }
                klass = type->asUMLClassifier();
                op = Import_Utils::makeOperation(klass, name);
                // The controlling parameter is suppressed.
                parNameCount--;
                if (parNameCount) {
                    for (uint i = 0; i < parNameCount; ++i) {
                        parName[i] = parName[i + 1];
                    }
                }
            }
            for (uint i = 0; i < parNameCount; ++i) {
                UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName[i]);
                att->setParmKind(dir);
            }
            if (advance() != QLatin1String(";"))
                break;
        }
        if (keyword == QLatin1String("function")) {
            if (advance() != QLatin1String("return")) {
                if (klass)
                    logError1("AdaImport::parseStmt: expecting \"return\" at function %1", name);
                else
                    logError1("AdaImport::parseStmt: expecting \"return\" at %1", m_source[m_srcIndex]);
                return false;
            }
            returnType = expand(advance());
            returnType.remove(QLatin1String("Standard."), Qt::CaseInsensitive);
        }
        bool isAbstract = false;
        if (advance() == QLatin1String("is") && advance() == QLatin1String("abstract"))
            isAbstract = true;
        if (klass != 0 && op != 0)
            Import_Utils::insertMethod(klass, op, m_currentAccess, returnType,
                                       false, isAbstract, false, false, false, m_comment);
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("task") || keyword == QLatin1String("protected")) {
        // Can task and protected objects/types be mapped to UML?
        QString name = advance();
        if (name == QLatin1String("type")) {
            name = advance();
        }
        QString next = advance();
        if (next == QLatin1String("(")) {
            skipStmt(QLatin1String(")"));  // skip discriminant
            next = advance();
        }
        if (next == QLatin1String("is"))
            skipStmt(QLatin1String("end"));
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("for")) {    // rep spec
        QString typeName = advance();
        QString next = advance();
        if (next == QLatin1String("'")) {
            advance();  // skip qualifier
            next = advance();
        }
        if (next == QLatin1String("use")) {
            if (advance() == QLatin1String("record"))
                skipStmt(QLatin1String("end"));
        } else {
            logError1("AdaImport::parseStmt: expecting \"use\" at rep spec of %1", typeName);
        }
        skipStmt();
        return true;
    }
    // At this point we're only interested in attribute declarations.
    if (m_klass == 0 || keyword == QLatin1String("null")) {
        skipStmt();
        return true;
    }
    const QString& name = keyword;
    if (advance() != QLatin1String(":")) {
        logError2("AdaImport::parseStmt:: expecting \":\" at %1 %2", name, m_source[m_srcIndex]);
        skipStmt();
        return true;
    }
    QString nextToken = advance();
    if (nextToken == QLatin1String("aliased"))
        nextToken = advance();
    QString typeName = expand(nextToken);
    QString initialValue;
    if (advance() == QLatin1String(":=")) {
        initialValue = advance();
        QString token;
        while ((token = advance()) != QLatin1String(";")) {
            initialValue.append(QLatin1Char(' ') + token);
        }
    }
    UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                 typeName, m_comment);
    if (o) {
        UMLAttribute *attr = o->asUMLAttribute();
        attr->setInitialValue(initialValue);
    }
    skipStmt();
    return true;
}


