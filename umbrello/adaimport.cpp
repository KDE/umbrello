/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
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
#include "umlpackagelist.h"
#include "package.h"
#include "classifier.h"
#include "enum.h"
#include "operation.h"
#include "attribute.h"

AdaImport::AdaImport() : NativeImportBase("--") {
}

AdaImport::~AdaImport() {
}

void AdaImport::fillSource(QString lexeme) {
    QString word;
    const uint len = lexeme.length();
    for (uint i = 0; i < len; i++) {
        QChar c = lexeme[i];
        if (c.isLetterOrNumber() || c == '_' || c == '.' || c == '#') {
            word += c;
        } else {
            if (!word.isEmpty()) {
                m_source.append(word);
                word = QString::null;
            }
            m_source.append(c);
        }
    }
    if (!word.isEmpty())
        m_source.append(word);
}

void AdaImport::parseFile(QString filename) {
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
    for (m_srcIndex = 0; m_srcIndex < srcLength; m_srcIndex++) {
        const QString& keyword = m_source[m_srcIndex];
        kdDebug() << '"' << keyword << '"' << endl;
        if (keyword.startsWith("--")) {
            m_comment = keyword.mid(2);
            continue;
        }
        if (keyword == "with") {
            while (++m_srcIndex < srcLength && m_source[m_srcIndex] != ";") {
                QString filename = m_source[m_srcIndex].lower();
                QStringList components = QStringList::split(".", filename);
                const QString& prefix = components.first();
                if (prefix == "system" || prefix == "ada" || prefix == "gnat" ||
                    prefix == "text_io" ||
                    prefix == "unchecked_conversion" ||
                    prefix == "unchecked_deallocation") {
                    if (advance() != ",")
                        break;
                    continue;
                }
                filename.replace(".", "-");
                filename.append(".ads");
                parseFile(filename);
                if (advance() != ",")
                    break;
            }
            continue;
        }
        if (keyword == "package") {
            const QString& name = advance();
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Package,
                            name, m_scope[m_scopeIndex], m_comment);
            m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
            if (advance() != "is") {
                kdError() << "AdaImport::parseFile: unexpected: " << m_source[m_srcIndex] << endl;
                skipStmt("is");
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
            if (m_source[m_srcIndex] == "abstract") {
                m_isAbstract = true;
                m_srcIndex++;
            }
            if (m_source[m_srcIndex] == "tagged") {
                UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                                name, m_scope[m_scopeIndex], m_comment);
                m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
                m_klass->setAbstract(m_isAbstract);
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
                UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                                name, m_scope[m_scopeIndex], m_comment);
                m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
                if (keyword == "struct")
                    m_klass->setStereotype("CORBAStruct");
                else
                    m_klass->setStereotype("CORBAException");
                if (advance() != "{") {
                    kdError() << "importIDL: expecting '{' at " << name << endl;
                    skipStmt("{");
                }
                m_comment = QString::null;
                continue;
            }
            if (m_source[m_srcIndex] == "new") {
                QString ancestor = advance();
                // Handle ancestor: To Be Done
            }
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                                     name, m_scope[m_scopeIndex], m_comment);
            m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
            // TO BE DONE
            skipStmt();
            continue;
        }
        if (keyword == "private") {
            m_currentAccess = Uml::Private;
            continue;
        }
        if (keyword == "end") {
            if (m_scopeIndex) {
                if (advance() != ";" && m_source[m_srcIndex] != "record") {
                    const QString& scopeName = m_scope[m_scopeIndex]->getName();
                    if (scopeName != m_source[m_srcIndex])
                        kdError() << "end: expecting " << scopeName << ", found "
                                  << m_source[m_srcIndex] << endl;
                }
                m_klass = dynamic_cast<UMLClassifier*>(m_scope[--m_scopeIndex]);
            } else {
                kdError() << "importAda: too many \"end\"" << endl;
            }
            skipStmt();
            continue;
        }
        if (keyword == "function" || keyword == "procedure") {
            QString name = advance();
            QString returnType;
            if (m_klass == NULL) {
                // Unlike an Ada package, a UML package does not support
                // subprograms.
                // In order to map those, we would need to create a UML
                // class with stereotype <<utility>> for the Ada package.
                kdDebug() << "importAda: no class set for " << name << endl;
                skipStmt();
                continue;
            }
            if (advance() != "(") {
                kdDebug() << "ignoring parameterless " << keyword << " " << name << endl;
                skipStmt();
                continue;
            }
            UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
            m_srcIndex++;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
                const QString &parName = advance();
                if (advance() != ":") {
                    kdError() << "importAda: expecting ':'" << endl;
                    skipStmt();
                    continue;
                }
                const QString &direction = advance();
                QString typeName;
                Uml::Parameter_Direction dir = Uml::pd_In;
                if (direction == "in") {
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
                UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName);
                att->setParmKind(dir);
                if (advance() != ";")
                    break;
                m_srcIndex++;
            }
            if (keyword == "function") {
                if (advance() != "return") {
                    kdError() << "importAda: expecting \"return\" at function " << name << endl;
                    skipStmt();
                    continue;
                }
                returnType = advance();
            }
            Import_Utils::insertMethod(m_klass, op, Uml::Public, returnType,
                                       false, false, false, false, m_comment);
            m_comment = QString::null;
            continue;
        }
        // Still lots To Be Done.......
        skipStmt();
        /* At this point we know it's some kind of attribute declaration.
        UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name,
                                                     typeName, m_comment);
        UMLAttribute *attr = static_cast<UMLAttribute*>(o);
        if (m_source[m_srcIndex] != ";") {
            kdError() << "importAda: ignoring trailing items at " << name << endl;
            skipStmt();
        }
        m_comment = QString::null;
         */
    }
}


