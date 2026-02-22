/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "adaimport.h"

// app includes
#include "umlassociation.h"
#include "umlattribute.h"
#include "umlclassifier.h"
#define DBG_SRC QStringLiteral("AdaImport")
#include "debug_utils.h"
#include "umlenum.h"
#include "import_utils.h"
#include "umloperation.h"
#include "umlpackage.h"
#include "umlapp.h"
#include "umldoc.h"

// qt includes
#include <QRegularExpression>

DEBUG_REGISTER(AdaImport)

/**
 * Constructor.
 */
AdaImport::AdaImport(CodeImpThread* thread)
  : NativeImportBase(QStringLiteral("--"), thread)
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
 * Apply package renamings to the given name.
 * @return  expanded name
 */
QString AdaImport::expand(const QString& name)
{
    QRegularExpression pfxRegExp(QStringLiteral("^(\\w+)\\."));
    pfxRegExp.setPatternOptions(QRegularExpression::PatternOption::CaseInsensitiveOption);
    QRegularExpressionMatch match = pfxRegExp.match(name);
    if (!match.hasMatch()) {
        return name;
    }

    QString result = name;
    QString pfx = match.captured(1);;
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
        QString filename = base + QStringLiteral(".ads");
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
    if (keyword == QStringLiteral("with")) {
        if (m_inGenericFormalPart) {
            // mapping of generic formal subprograms or packages is not yet implemented
            return false;
        }
        while (++m_srcIndex < srcLength && m_source[m_srcIndex] != QStringLiteral(";")) {
            QStringList components = m_source[m_srcIndex].toLower().split(QLatin1Char('.'));
            const QString& prefix = components.first();
            if (prefix == QStringLiteral("system")
                || prefix == QStringLiteral("ada")
                || prefix == QStringLiteral("gnat")
                || prefix == QStringLiteral("interfaces")
                || prefix == QStringLiteral("text_io")
                || prefix == QStringLiteral("unchecked_conversion")
                || prefix == QStringLiteral("unchecked_deallocation")) {
                if (advance() != QStringLiteral(","))
                    break;
                continue;
            }
            parseStems(components);
            if (advance() != QStringLiteral(","))
                break;
        }
        return true;
    }
    if (keyword == QStringLiteral("generic")) {
        m_inGenericFormalPart = true;
        return true;
    }
    if (keyword == QStringLiteral("package")) {
        const QString& name = advance();
        QStringList parentPkgs = name.toLower().split(QLatin1Char('.'));
        parentPkgs.pop_back();  // exclude the current package
        parseStems(parentPkgs);
        UMLObject *ns = nullptr;
        if (advance() == QStringLiteral("is")) {
            ns = Import_Utils::createUMLObject(UMLObject::ot_Package, name,
                                               currentScope(), m_comment);
            if (m_source[m_srcIndex + 1] == QStringLiteral("new")) {
                m_srcIndex++;
                QString pkgName = advance();
                UMLObject *gp = Import_Utils::createUMLObject(UMLObject::ot_Package, pkgName,
                                                              currentScope());
                gp->setStereotype(QStringLiteral("generic"));
                // Add binding from instantiator to instantiatee
                UMLAssociation *assoc = new UMLAssociation(Uml::AssociationType::Dependency, ns, gp);
                assoc->setUMLPackage(umldoc->rootFolder(Uml::ModelType::Logical));
                assoc->setStereotype(QStringLiteral("bind"));
                // Work around missing display of stereotype in AssociationWidget:
                assoc->setName(assoc->stereotype(true));
                umldoc->addAssociation(assoc);
                skipStmt();
            } else {
                pushScope(ns->asUMLPackage());
            }
        } else if (m_source[m_srcIndex] == QStringLiteral("renames")) {
            m_renaming[name] = advance();
        } else {
            logError1("AdaImport::parseStmt unexpected: %1", m_source[m_srcIndex]);
            skipStmt(QStringLiteral("is"));
        }
        if (m_inGenericFormalPart) {
            if (ns)
                ns->setStereotype(QStringLiteral("generic"));
            m_inGenericFormalPart = false;
        }
        return true;
    }
    if (m_inGenericFormalPart)
        return false;  // skip generic formal parameter (not yet implemented)
    if (keyword == QStringLiteral("subtype")) {
        QString name = advance();
        advance();  // "is"
        QString base = expand(advance());
        base.remove(QStringLiteral("Standard."), Qt::CaseInsensitive);
        UMLObject *type = umldoc->findUMLObject(base, UMLObject::ot_UMLObject, currentScope());
        if (type == nullptr) {
            type = Import_Utils::createUMLObject(UMLObject::ot_Datatype, base, currentScope());
        }
        UMLObject *subtype = Import_Utils::createUMLObject(type->baseType(), name,
                                                           currentScope(), m_comment);
        UMLAssociation *assoc = new UMLAssociation(Uml::AssociationType::Dependency, subtype, type);
        assoc->setUMLPackage(umldoc->rootFolder(Uml::ModelType::Logical));
        assoc->setStereotype(QStringLiteral("subtype"));
        // Work around missing display of stereotype in AssociationWidget:
        assoc->setName(assoc->stereotype(true));
        umldoc->addAssociation(assoc);
        skipStmt();
        return true;
    }
    if (keyword == QStringLiteral("type")) {
        QString name = advance();
        QString next = advance();
        if (next == QStringLiteral("(")) {
            logDebug1("AdaImport::parseStmt %1: discriminant handling is not yet implemented", name);
            // @todo Find out how to map discriminated record to UML.
            //       For now, we just create a pro forma empty record.
            Import_Utils::createUMLObject(UMLObject::ot_Class, name, currentScope(),
                                          m_comment, QStringLiteral("record"));
            skipStmt(QStringLiteral("end"));
            if ((next = advance()) == QStringLiteral("case"))
                m_srcIndex += 2;  // skip "case" ";"
            skipStmt();
            return true;
        }
        if (next == QStringLiteral(";")) {
            // forward declaration
            Import_Utils::createUMLObject(UMLObject::ot_Class, name, currentScope(),
                                          m_comment);
            return true;
        }
        if (next != QStringLiteral("is")) {
            logError1("AdaImport::parseStmt: expecting \"is\" at %1", next);
            return false;
        }
        next = advance();
        if (next == QStringLiteral("(")) {
            // enum type
            UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                            name, currentScope(), m_comment);
            UMLEnum *enumType = ns->asUMLEnum();
            if (enumType == nullptr)
                enumType = Import_Utils::remapUMLEnum(ns, enumType);
            while ((next = advance()) != QStringLiteral(")")) {
                if (enumType != nullptr)
                    Import_Utils::addEnumLiteral(enumType, next, m_comment);
                m_comment.clear();
                if (advance() != QStringLiteral(","))
                    break;
            }
            skipStmt();
            return true;
        }
        bool isTaggedType = false;
        if (next == QStringLiteral("abstract")) {
            m_isAbstract = true;
            next = advance();
        }
        if (next == QStringLiteral("tagged")) {
            isTaggedType = true;
            next = advance();
        }
        if (next == QStringLiteral("limited") ||
            next == QStringLiteral("task") ||
            next == QStringLiteral("protected") ||
            next == QStringLiteral("synchronized")) {
            next = advance();  // we can't (yet?) represent that
        }
        if (next == QStringLiteral("private") ||
            next == QStringLiteral("interface") ||
            next == QStringLiteral("record") ||
            (next == QStringLiteral("null") &&
             m_source[m_srcIndex+1] == QStringLiteral("record"))) {
            UMLObject::ObjectType t = (next == QStringLiteral("interface") ? UMLObject::ot_Interface
                                                                          : UMLObject::ot_Class);
            UMLObject *ns = Import_Utils::createUMLObject(t, name, currentScope(), m_comment);
            if (t == UMLObject::ot_Interface) {
                while ((next = advance()) == QStringLiteral("and")) {
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
                ns->setStereotype(QStringLiteral("record"));
            }
            if (next == QStringLiteral("record"))
                m_klass = ns->asUMLClassifier();
            else
                skipStmt();
            return true;
        }
        if (next == QStringLiteral("new")) {
            QString base = expand(advance());
            QStringList baseInterfaces;
            while ((next = advance()) == QStringLiteral("and")) {
                baseInterfaces.append(expand(advance()));
            }
            const bool isExtension = (next == QStringLiteral("with"));
            UMLObject::ObjectType t;
            if (isExtension || m_isAbstract) {
                t = UMLObject::ot_Class;
            } else {
                base.remove(QStringLiteral("Standard."), Qt::CaseInsensitive);
                UMLObject *known = umldoc->findUMLObject(base, UMLObject::ot_UMLObject, currentScope());
                t = (known ? known->baseType() : UMLObject::ot_Datatype);
            }
            UMLObject *ns = Import_Utils::createUMLObject(t, base, nullptr);
            UMLClassifier *parent = ns->asUMLClassifier();
            ns = Import_Utils::createUMLObject(t, name, currentScope(), m_comment);
            if (isExtension) {
                next = advance();
                if (next == QStringLiteral("null") || next == QStringLiteral("record")) {
                    UMLClassifier *klass = ns->asUMLClassifier();
                    Import_Utils::createGeneralization(klass, parent);
                    if (next == QStringLiteral("record")) {
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
    if (keyword == QStringLiteral("private")) {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == QStringLiteral("end")) {
        if (m_klass) {
            if (advance() != QStringLiteral("record")) {
                logError1("AdaImport::parseStmt end: expecting \"record\" at %1",
                          m_source[m_srcIndex]);
            }
            m_klass = nullptr;
        } else if (scopeIndex()) {
            if (advance() != QStringLiteral(";")) {
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
    if (keyword == QStringLiteral("not"))
        keyword = advance();
    if (keyword == QStringLiteral("overriding"))
        keyword = advance();
    if (keyword == QStringLiteral("function") || keyword == QStringLiteral("procedure")) {
        const QString& name = advance();
        QString returnType;
        if (advance() != QStringLiteral("(")) {
            // Unlike an Ada package, a UML package does not support
            // subprograms.
            // In order to map those, we would need to create a UML
            // class with stereotype <<utility>> for the Ada package.
            logDebug2("AdaImport::parseStmt(%1): ignoring parameterless %2", keyword, name);
            skipStmt();
            return true;
        }
        UMLClassifier *klass = nullptr;
        UMLOperation *op = nullptr;
        const uint MAX_PARNAMES = 16;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != QStringLiteral(")")) {
            QString parName[MAX_PARNAMES];
            uint parNameCount = 0;
            do {
                if (parNameCount >= MAX_PARNAMES) {
                    logError1("AdaImport::parseStmt: MAX_PARNAMES is exceeded at %1", name);
                    break;
                }
                parName[parNameCount++] = advance();
            } while (advance() == QStringLiteral(","));
            if (m_source[m_srcIndex] != QStringLiteral(":")) {
                logError2("AdaImport::parseStmt: expecting ':' at %1 (index %2)",
                          m_source[m_srcIndex], m_srcIndex);
                skipStmt();
                break;
            }
            const QString &direction = advance();
            QString typeName;
            Uml::ParameterDirection::Enum dir = Uml::ParameterDirection::In;
            if (direction == QStringLiteral("access")) {
                // Oops, we have to improvise here because there
                // is no such thing as "access" in UML.
                // So we use the next best thing, "inout".
                // Better ideas, anyone?
                dir = Uml::ParameterDirection::InOut;
                typeName = advance();
            } else if (direction == QStringLiteral("in")) {
                if (m_source[m_srcIndex + 1] == QStringLiteral("out")) {
                    dir = Uml::ParameterDirection::InOut;
                    m_srcIndex++;
                }
                typeName = advance();
            } else if (direction == QStringLiteral("out")) {
                dir = Uml::ParameterDirection::Out;
                typeName = advance();
            } else {
                typeName = direction;  // In Ada, the default direction is "in"
            }
            typeName.remove(QStringLiteral("Standard."), Qt::CaseInsensitive);
            typeName = expand(typeName);
            if (op == nullptr) {
                // In Ada, the first parameter indicates the class.
                UMLObject *type = Import_Utils::createUMLObject(UMLObject::ot_Class, typeName, currentScope());
                UMLObject::ObjectType t = type->baseType();
                if ((t != UMLObject::ot_Interface &&
                     (t != UMLObject::ot_Class || type->stereotype() == QStringLiteral("record"))) ||
                    !m_classesDefinedInThisScope.contains(type)) {
                    // Not an instance bound method - we cannot represent it.
                    skipStmt(QStringLiteral(")"));
                    break;
                }
                klass = type->asUMLClassifier();
                op = Import_Utils::makeOperation(klass, name);
                // The controlling parameter is suppressed.
                if (parNameCount) {
                    for (uint i = 0; i < parNameCount - 1; ++i) { // stop at parNameCount-2
                        parName[i] = parName[i + 1];
                    }
                    parNameCount--; // reduce count after removing first element
                }
            }
            for (uint i = 0; i < parNameCount; ++i) {
                UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName[i]);
                att->setParmKind(dir);
            }
            if (advance() != QStringLiteral(";"))
                break;
        }
        if (keyword == QStringLiteral("function")) {
            if (advance() != QStringLiteral("return")) {
                if (klass)
                    logError1("AdaImport::parseStmt: expecting \"return\" at function %1", name);
                else
                    logError1("AdaImport::parseStmt: expecting \"return\" at %1", m_source[m_srcIndex]);
                return false;
            }
            returnType = expand(advance());
            returnType.remove(QStringLiteral("Standard."), Qt::CaseInsensitive);
        }
        bool isAbstract = false;
        if (advance() == QStringLiteral("is") && advance() == QStringLiteral("abstract"))
            isAbstract = true;
        if (klass != nullptr && op != nullptr)
            Import_Utils::insertMethod(klass, op, m_currentAccess, returnType,
                                       false, isAbstract, false, false, false, m_comment);
        skipStmt();
        return true;
    }
    if (keyword == QStringLiteral("task") || keyword == QStringLiteral("protected")) {
        // Can task and protected objects/types be mapped to UML?
        QString name = advance();
        if (name == QStringLiteral("type")) {
            name = advance();
        }
        QString next = advance();
        if (next == QStringLiteral("(")) {
            skipStmt(QStringLiteral(")"));  // skip discriminant
            next = advance();
        }
        if (next == QStringLiteral("is"))
            skipStmt(QStringLiteral("end"));
        skipStmt();
        return true;
    }
    if (keyword == QStringLiteral("for")) {    // rep spec
        QString typeName = advance();
        QString next = advance();
        if (next == QStringLiteral("'")) {
            advance();  // skip qualifier
            next = advance();
        }
        if (next == QStringLiteral("use")) {
            if (advance() == QStringLiteral("record"))
                skipStmt(QStringLiteral("end"));
        } else {
            logError1("AdaImport::parseStmt: expecting \"use\" at rep spec of %1", typeName);
        }
        skipStmt();
        return true;
    }
    // At this point we're only interested in attribute declarations.
    if (m_klass == nullptr || keyword == QStringLiteral("null")) {
        skipStmt();
        return true;
    }
    const QString& name = keyword;
    if (advance() != QStringLiteral(":")) {
        logError2("AdaImport::parseStmt:: expecting \":\" at %1 %2", name, m_source[m_srcIndex]);
        skipStmt();
        return true;
    }
    QString nextToken = advance();
    if (nextToken == QStringLiteral("aliased"))
        nextToken = advance();
    QString typeName = expand(nextToken);
    QString initialValue;
    if (advance() == QStringLiteral(":=")) {
        initialValue = advance();
        QString token;
        while ((token = advance()) != QStringLiteral(";")) {
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


