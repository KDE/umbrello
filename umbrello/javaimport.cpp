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
#include "umlpackagelist.h"
#include "package.h"
#include "classifier.h"
#include "enum.h"
#include "operation.h"
#include "attribute.h"

JavaImport::JavaImport() : NativeImportBase("//") {
    m_isStatic = false;
}

JavaImport::~JavaImport() {
}

/// Catenate possible template arguments/array dimensions to the end of the type name.
/// Currently this function does not advance() to the next token. (TBC should it?)
QString JavaImport::joinTypename() {
    QString typeName = m_source[m_srcIndex];
    const uint srcLength = m_source.count();
    QString next = m_source[m_srcIndex + 1];
    if (next == "<") {
        m_srcIndex++;
        typeName += '<';
        while (m_srcIndex < srcLength) {
            next = advance();
            typeName += next;
            if (next == ">")
                break;
        }
        next = m_source[m_srcIndex + 1];
    }
    if (next == "[") {
        m_srcIndex++;
        typeName += '[';
        while (m_srcIndex < srcLength) {
            next = advance();
            typeName += next;
            if (next == "]")
                break;
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

void JavaImport::parseFile(QString filename) {
    if (filename.contains('/')) {
        QString path = filename;
        path.remove( QRegExp("/[^/]+$") );
        kdDebug() << "JavaImport::parseFile: adding path " << path << endl;
        Import_Utils::addIncludePath(path);
    }
    if (! QFile::exists(filename)) {
        if (filename.startsWith("/")) {
            kdError() << "JavaImport::parseFile(" << filename << "): cannot find file" << endl;
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
            kdError() << "JavaImport::parseFile(" << filename << "): cannot find file" << endl;
            return;
        }
    }
    QFile file(filename);
    if (! file.open(IO_ReadOnly)) {
        kdError() << "JavaImport::parseFile(" << filename << "): cannot open file" << endl;
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
    m_currentAccess = Uml::Visibility::Public;
    m_isStatic = m_isAbstract = false;
    m_scopeIndex = 0;
    m_scope[0] = NULL;
    const uint srcLength = m_source.count();
    for (m_srcIndex = 0; m_srcIndex < srcLength; m_srcIndex++) {
        const QString& keyword = m_source[m_srcIndex];
        //kdDebug() << '"' << keyword << '"' << endl;
        if (keyword.startsWith(m_singleLineCommentIntro)) {
            m_comment = keyword.mid(2);
            continue;
        }
        if (keyword == "package") {
            const QString& qualifiedName = advance();
            QStringList names = QStringList::split(".", qualifiedName);
            for (QStringList::Iterator nameIt = names.begin(); nameIt != names.end(); ++nameIt) {
                QString name = (*nameIt);
                UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Package,
                                name, m_scope[m_scopeIndex], m_comment);
                m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
            }
            if (advance() != ";") {
                kdError() << "importJava: unexpected: " << m_source[m_srcIndex] << endl;
                skipStmt();
            }
            m_comment = QString::null;
            continue;
        }
        if (keyword == "class" || keyword == "interface") {
            const QString& name = advance();
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                            name, m_scope[m_scopeIndex], m_comment);
            m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
            m_klass->setAbstract(m_isAbstract);
            m_klass->setStatic(m_isStatic);
            m_isAbstract = m_isStatic = false;
            m_comment = QString::null;
            if (advance() == ";")   // forward declaration
                continue;
            if (m_source[m_srcIndex] == "extends") {
                const QString& baseName = advance();
                Import_Utils::createGeneralization(m_klass, baseName);
                advance();
            }
            if (m_source[m_srcIndex] == "implements") {
                while (m_srcIndex < srcLength - 1 && advance() != "{") {
                    const QString& baseName = m_source[m_srcIndex];
                    Import_Utils::createGeneralization(m_klass, baseName);
                    if (advance() != ",")
                        break;
                }
            }
            if (m_source[m_srcIndex] != "{") {
                kdError() << "importJava: ignoring excess chars at " << name << endl;
                skipStmt("{");
            }
            continue;
        }
        if (keyword == "enum") {
            const QString& name = advance();
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Enum,
                            name, m_scope[m_scopeIndex], m_comment);
            UMLEnum *enumType = static_cast<UMLEnum*>(ns);
            advance();  // skip name
            while (m_srcIndex < srcLength - 1 && advance() != "}") {
                Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
                if (advance() != ",")
                    break;
            }
            m_comment = QString::null;
            continue;
        }
        if (keyword == "static") {
            m_isStatic = true;
            continue;
        }
        if (keyword == "abstract") {
            m_isAbstract = true;
            continue;
        }
        if (keyword == "public") {
            m_currentAccess = Uml::Visibility::Public;
            continue;
        }
        if (keyword == "protected") {
            m_currentAccess = Uml::Visibility::Protected;
            continue;
        }
        if (keyword == "private") {
            m_currentAccess = Uml::Visibility::Private;
            continue;
        }
        if (keyword == "final" ||
            keyword == "native" ||
            keyword == "synchronized" ||
            keyword == "transient" ||
            keyword == "volatile") {
            //@todo anything to do here?
            continue;
        }
        if (keyword == "import") {
            skipStmt();
            continue;
        }
        if (keyword == "}") {
            if (m_scopeIndex)
                m_klass = dynamic_cast<UMLClassifier*>(m_scope[--m_scopeIndex]);
            else
                kdError() << "importJava: too many }" << endl;
            continue;
        }
        // At this point, we expect `keyword' to be a type name
        // (of a member of class or interface, or return type
        // of an operation.) Up next is the name of the attribute
        // or operation.
        if (! keyword.contains( QRegExp("^\\w") )) {
            kdError() << "importJava: ignoring " << keyword << endl;
            skipStmt();
            continue;
        }
        QString typeName = joinTypename();
        QString name;
        if (typeName == m_klass->getName()) {
            // Constructor.
            name = typeName;
            typeName = QString::null;
        } else {
            name = advance();
        }
        if (name.contains( QRegExp("\\W") )) {
            kdError() << "importJava: expecting name in " << name << endl;
            skipStmt();
            continue;
        }
        // At this point we need a class.
        if (m_klass == NULL) {
            kdError() << "importJava: no class set for " << name << endl;
            continue;
        }
        QString nextToken = advance();
        if (nextToken == "(") {
            // operation
            UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
            m_srcIndex++;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
                QString typeName = joinTypename();
                QString parName = advance();
                UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName);
                if (advance() != ",")
                    break;
                m_srcIndex++;
            }
            Import_Utils::insertMethod(m_klass, op, Uml::Visibility::Public, typeName,
                                       m_isStatic, m_isAbstract, false /*isFriend*/,
                                       false /*isConstructor*/, m_comment);
            m_isAbstract = m_isStatic = false;
            m_comment = QString::null;
            skipStmt("{");
            int braceNesting = 0;
            while (m_srcIndex < srcLength) {
                nextToken = advance();
                if (nextToken == "}") {
                    if (braceNesting <= 0)
                        break;
                    braceNesting--;
                } else if (nextToken == "{") {
                    braceNesting++;
                }
            }
            m_comment = QString::null;
            continue;
        }
        // At this point we know it's some kind of attribute declaration.
        while (1) {
            while (nextToken != "," && nextToken != ";") {
                name += nextToken;  // add possible array dimensions to `name'
                nextToken = advance();
            }
            UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name, typeName, m_comment);
            UMLAttribute *attr = static_cast<UMLAttribute*>(o);
            if (nextToken != ",")
                break;
            name = advance();
            nextToken = advance();
        }
        m_currentAccess = Uml::Visibility::Public;
        if (m_source[m_srcIndex] != ";") {
            kdError() << "importJava: ignoring trailing items at " << name << endl;
            skipStmt();
        }
        m_comment = QString::null;
    }
}


