/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
#include "../folder.h"
#include "../classifier.h"
#include "../enum.h"
#include "../operation.h"
#include "../attribute.h"
#include "../association.h"

AdaImport::AdaImport() : NativeImportBase("--") {
   initVars();
}

AdaImport::~AdaImport() {
}

void AdaImport::initVars() {
    m_inGenericFormalPart = false;
    m_classesDefinedInThisScope.clear();
    m_renaming.clear();
}

/// Split the line so that a string is returned as a single element of the list,
/// when not in a string then split at white space.
QStringList AdaImport::split(const QString& lin) {
    QStringList list;
    QString listElement;
    bool inString = false;
    bool seenSpace = false;
    QString line = lin.stripWhiteSpace();
    uint len = line.length();
    for (uint i = 0; i < len; i++) {
        const QChar& c = line[i];
        if (inString) {
            listElement += c;
            if (c == '"') {
                if (i < len - 1 && line[i + 1] == '"') {
                    i++;     // escaped quotation mark
                    continue;
                }
                list.append(listElement);
                listElement = QString();
                inString = false;
            }
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
                listElement = QString();
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
                listElement = QString();
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

void AdaImport::fillSource(const QString& word) {
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; i++) {
        QChar c = word[i];
        if (c.isLetterOrNumber() || c == '_' || c == '.' || c == '#') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme = QString();
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

QString AdaImport::expand(const QString& name) {
    QRegExp pfxRegExp("^(\\w+)\\.");
    pfxRegExp.setCaseSensitive(false);
    int pos = pfxRegExp.search(name);
    if (pos == -1)
        return name;
    QString result = name;
    QString pfx = pfxRegExp.cap(1);
    if (m_renaming.contains(pfx)) {
        result.remove(pfxRegExp);
        result.prepend(m_renaming[pfx] + '.');
    }
    return result;
}

void AdaImport::parseStems(const QStringList& stems) {
    if (stems.isEmpty())
        return;
    QString base = stems.first();
    uint i = 0;
    while (1) {
        QString filename = base + ".ads";
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
        base += '-' + stems[i];
    }
}

bool AdaImport::parseStmt() {
    const uint srcLength = m_source.count();
    QString keyword = m_source[m_srcIndex];
    UMLDoc *umldoc = UMLApp::app()->getDocument();
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
            parseStems(components);
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
        QStringList parentPkgs = QStringList::split(".", name.lower());
        parentPkgs.pop_back();  // exclude the current package
        parseStems(parentPkgs);
        UMLObject *ns = NULL;
        if (advance() == "is") {
            ns = Import_Utils::createUMLObject(Uml::ot_Package, name,
                                               m_scope[m_scopeIndex], m_comment);
            if (m_source[m_srcIndex + 1] == "new") {
                m_srcIndex++;
                QString pkgName = advance();
                UMLObject *gp = Import_Utils::createUMLObject(Uml::ot_Package, pkgName,
                                                              m_scope[m_scopeIndex]);
                gp->setStereotype("generic");
                // Add binding from instantiator to instantiatee
                UMLAssociation *assoc = new UMLAssociation(Uml::at_Dependency, ns, gp);
                assoc->setUMLPackage(umldoc->getRootFolder(Uml::mt_Logical));
                assoc->setStereotype("bind");
                // Work around missing display of stereotype in AssociationWidget:
                assoc->setName(assoc->getStereotype(true));
                umldoc->addAssociation(assoc);
                skipStmt();
            } else {
                m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
            }
        } else if (m_source[m_srcIndex] == "renames") {
            m_renaming[name] = advance();
        } else {
            kError() << "AdaImport::parseStmt: unexpected: " << m_source[m_srcIndex] << endl;
            skipStmt("is");
        }
        if (m_inGenericFormalPart) {
            ns->setStereotype("generic");
            m_inGenericFormalPart = false;
        }
        return true;
    }
    if (m_inGenericFormalPart)
        return false;  // skip generic formal parameter (not yet implemented)
    if (keyword == "subtype") {
        QString name = advance();
        advance();  // "is"
        QString base = expand(advance());
        base.remove("Standard.", false);
        UMLObject *type = umldoc->findUMLObject(base, Uml::ot_UMLObject, m_scope[m_scopeIndex]);
        if (type == NULL) {
            type = Import_Utils::createUMLObject(Uml::ot_Datatype, base, m_scope[m_scopeIndex]);
        }
        UMLObject *subtype = Import_Utils::createUMLObject(type->getBaseType(), name,
                                                           m_scope[m_scopeIndex], m_comment);
        UMLAssociation *assoc = new UMLAssociation(Uml::at_Dependency, subtype, type);
        assoc->setUMLPackage(umldoc->getRootFolder(Uml::mt_Logical));
        assoc->setStereotype("subtype");
        // Work around missing display of stereotype in AssociationWidget:
        assoc->setName(assoc->getStereotype(true));
        umldoc->addAssociation(assoc);
        skipStmt();
        return true;
    }
    if (keyword == "type") {
        QString name = advance();
        QString next = advance();
        if (next == "(") {
            kDebug() << "AdaImport::parseStmt(" << name << "): "
                << "discriminant handling is not yet implemented" << endl;
            // @todo Find out how to map discriminated record to UML.
            //       For now, we just create a pro forma empty record.
            Import_Utils::createUMLObject(Uml::ot_Class, name, m_scope[m_scopeIndex],
                                          m_comment, "record");
            skipStmt("end");
            if ((next = advance()) == "case")
                m_srcIndex += 2;  // skip "case" ";"
            skipStmt();
            return true;
        }
        if (next == ";") {
            // forward declaration
            Import_Utils::createUMLObject(Uml::ot_Class, name, m_scope[m_scopeIndex],
                                          m_comment);
            return true;
        }
        if (next != "is") {
            kError() << "AdaImport::parseStmt: expecting \"is\"" << endl;
            return false;
        }
        next = advance();
        if (next == "(") {
            // enum type
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Enum,
                            name, m_scope[m_scopeIndex], m_comment);
            UMLEnum *enumType = static_cast<UMLEnum*>(ns);
            while ((next = advance()) != ")") {
                Import_Utils::addEnumLiteral(enumType, next, m_comment);
                m_comment = QString();
                if (advance() != ",")
                    break;
            }
            skipStmt();
            return true;
        }
        bool isTaggedType = false;
        if (next == "abstract") {
            m_isAbstract = true;
            next = advance();
        }
        if (next == "tagged") {
            isTaggedType = true;
            next = advance();
        }
        if (next == "limited" ||
            next == "task" ||
            next == "protected" ||
            next == "synchronized") {
            next = advance();  // we can't (yet?) represent that
        }
        if (next == "private" ||
            next == "interface" ||
            next == "record" ||
            (next == "null" &&
             m_source[m_srcIndex+1] == "record")) {
            Uml::Object_Type t = (next == "interface" ? Uml::ot_Interface : Uml::ot_Class);
            UMLObject *ns = Import_Utils::createUMLObject(t, name, m_scope[m_scopeIndex], m_comment);
            if (t == Uml::ot_Interface) {
                while ((next = advance()) == "and") {
                    UMLClassifier *klass = static_cast<UMLClassifier*>(ns);
                    QString base = expand(advance());
                    UMLObject *p = Import_Utils::createUMLObject(Uml::ot_Interface, base, m_scope[m_scopeIndex]);
                    UMLClassifier *parent = static_cast<UMLClassifier*>(p);
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
                ns->setStereotype("record");
            }
            if (next == "record")
                m_klass = static_cast<UMLClassifier*>(ns);
            else
                skipStmt();
            return true;
        }
        if (next == "new") {
            QString base = expand(advance());
            QStringList baseInterfaces;
            while ((next = advance()) == "and") {
                baseInterfaces.append(expand(advance()));
            }
            const bool isExtension = (next == "with");
            Uml::Object_Type t;
            if (isExtension || m_isAbstract) {
                t = Uml::ot_Class;
            } else {
                base.remove("Standard.", false);
                UMLObject *known = umldoc->findUMLObject(base, Uml::ot_UMLObject, m_scope[m_scopeIndex]);
                t = (known ? known->getBaseType() : Uml::ot_Datatype);
            }
            UMLObject *ns = Import_Utils::createUMLObject(t, base, NULL);
            UMLClassifier *parent = static_cast<UMLClassifier*>(ns);
            ns = Import_Utils::createUMLObject(t, name, m_scope[m_scopeIndex], m_comment);
            if (isExtension) {
                next = advance();
                if (next == "null" || next == "record") {
                    UMLClassifier *klass = static_cast<UMLClassifier*>(ns);
                    Import_Utils::createGeneralization(klass, parent);
                    if (next == "record") {
                        // Set the m_klass for attributes.
                        m_klass = klass;
                    }
                    if (baseInterfaces.count()) {
                        t = Uml::ot_Interface;
                        QStringList::Iterator end(baseInterfaces.end());
                        for (QStringList::Iterator bi(baseInterfaces.begin()); bi != end; ++bi) {
                             ns = Import_Utils::createUMLObject(t, *bi, m_scope[m_scopeIndex]);
                             parent = static_cast<UMLClassifier*>(ns);
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
                QString scopeName = m_scope[m_scopeIndex]->getFullyQualifiedName();
                if (scopeName.lower() != m_source[m_srcIndex].lower())
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
    // subprogram
    if (keyword == "not")
        keyword = advance();
    if (keyword == "overriding")
        keyword = advance();
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
            typeName.remove("Standard.", false);
            typeName = expand(typeName);
            if (op == NULL) {
                // In Ada, the first parameter indicates the class.
                UMLObject *type = Import_Utils::createUMLObject(Uml::ot_Class, typeName, m_scope[m_scopeIndex]);
                Uml::Object_Type t = type->getBaseType();
                if ((t != Uml::ot_Interface &&
                     (t != Uml::ot_Class || type->getStereotype() == "record")) ||
                    !m_classesDefinedInThisScope.contains(type)) {
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
            returnType = expand(advance());
            returnType.remove("Standard.", false);
        }
        bool isAbstract = false;
        if (advance() == "is" && advance() == "abstract")
            isAbstract = true;
        if (klass != NULL && op != NULL)
            Import_Utils::insertMethod(klass, op, m_currentAccess, returnType,
                                       false, isAbstract, false, false, m_comment);
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
    if (keyword == "for") {    // rep spec
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
    QString nextToken = advance();
    if (nextToken == "aliased")
        nextToken = advance();
    QString typeName = expand(nextToken);
    QString initialValue;
    if (advance() == ":=") {
        initialValue = advance();
        QString token;
        while ((token = advance()) != ";") {
            initialValue.append(' ' + token);
        }
    }
    UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                 typeName, m_comment);
    UMLAttribute *attr = static_cast<UMLAttribute*>(o);
    attr->setInitialValue(initialValue);
    skipStmt();
    return true;
}


