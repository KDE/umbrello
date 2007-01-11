/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2005                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "idlimport.h"

#include <stdio.h>
// qt/kde includes
// #include <qprocess.h>  //should use this instead of popen()
#include <qstringlist.h>
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "import_utils.h"
#include "../uml.h"
#include "../umldoc.h"
#include "../umlpackagelist.h"
#include "../package.h"
#include "../classifier.h"
#include "../enum.h"
#include "../operation.h"
#include "../attribute.h"

IDLImport::IDLImport() : NativeImportBase("//") {
    m_isOneway = m_isReadonly = m_isAttribute = false;
    setMultiLineComment("/*", "*/");
}

IDLImport::~IDLImport() {
}

/// Check for split type names (e.g. unsigned long long)
QString IDLImport::joinTypename() {
    QString typeName = m_source[m_srcIndex];
    if (m_source[m_srcIndex] == "unsigned")
        typeName += ' ' + advance();
    if (m_source[m_srcIndex] == "long" &&
            (m_source[m_srcIndex + 1] == "long" || m_source[m_srcIndex + 1] == "double"))
        typeName += ' ' + advance();
    return typeName;
}

bool IDLImport::preprocess(QString& line) {
    // Ignore C preprocessor generated lines.
    if (line.startsWith("#"))
        return true;  // done
    return NativeImportBase::preprocess(line);
}

void IDLImport::fillSource(const QString& word) {
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; i++) {
        QChar c = word[i];
        if (c.isLetterOrNumber() || c == '_') {
            lexeme += c;
        } else if (c == ':' && word[i + 1] == ':') {
            // compress scoped name into lexeme
            lexeme += "::";
            i++;
        } else if (c == '<') {
            // compress sequence or bounded string into lexeme
            do {
                lexeme += word[i];
            } while (word[i] != '>' && ++i < len);
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme = QString();
            }
            m_source.append(QString(c));
        }
    }
    if (!lexeme.isEmpty())
        m_source.append(lexeme);
}

void IDLImport::parseFile(const QString& filename) {
    if (filename.contains('/')) {
        QString path = filename;
        path.remove( QRegExp("/[^/]+$") );
        kDebug() << "IDLImport::parseFile: adding path " << path << endl;
        Import_Utils::addIncludePath(path);
    }
    QStringList includePaths = Import_Utils::includePathList();
    //QProcess command("cpp", UMLAp::app());
    QString command("cpp -C");   // -C means "preserve comments"
    for (QStringList::Iterator pathIt = includePaths.begin();
            pathIt != includePaths.end(); ++pathIt) {
        QString path = (*pathIt);
        //command.addArgument(" -I" + path);
        command += " -I" + path;
    }
    command += ' ' + filename;
    kDebug() << "importIDL: " << command << endl;
    FILE *fp = popen(command.ascii(), "r");
    if (fp == NULL) {
        kError() << "IDLImport::parseFile: cannot popen(" << command << ")" << endl;
        return;
    }
    // Scan the input file into the QStringList m_source.
    m_source.clear();
    char buf[256];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        int len = strlen(buf);
        if (buf[len - 1] == '\n')
            buf[--len] = '\0';
        NativeImportBase::scan( QString(buf) );
    }
    // Parse the QStringList m_source.
    m_scopeIndex = 0;
    m_scope[0] = NULL;
    const uint srcLength = m_source.count();
    for (m_srcIndex = 0; m_srcIndex < srcLength; m_srcIndex++) {
        const QString& keyword = m_source[m_srcIndex];
        //kDebug() << '"' << keyword << '"' << endl;
        if (keyword.startsWith(m_singleLineCommentIntro)) {
            m_comment = keyword.mid(m_singleLineCommentIntro.length());
            continue;
        }
        if (! parseStmt())
            skipStmt();
        m_currentAccess = Uml::Visibility::Public;
        m_comment = QString();
    }
    pclose(fp);
}

bool IDLImport::parseStmt() {
    const QString& keyword = m_source[m_srcIndex];
    const uint srcLength = m_source.count();
    if (keyword == "module") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Package,
                        name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
        m_scope[m_scopeIndex]->setStereotype("CORBAModule");
        if (advance() != "{") {
            kError() << "importIDL: unexpected: " << m_source[m_srcIndex] << endl;
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "interface") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                        name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        m_klass->setStereotype("CORBAInterface");
        m_klass->setAbstract(m_isAbstract);
        m_isAbstract = false;
        m_comment = QString();
        if (advance() == ";")   // forward declaration
            return true;
        if (m_source[m_srcIndex] == ":") {
            while (++m_srcIndex < srcLength && m_source[m_srcIndex] != "{") {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != ",")
                    break;
            }
        }
        if (m_source[m_srcIndex] != "{") {
            kError() << "importIDL: ignoring excess chars at "
            << name << endl;
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "struct" || keyword == "exception") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                        name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        if (keyword == "struct")
            m_klass->setStereotype("CORBAStruct");
        else
            m_klass->setStereotype("CORBAException");
        if (advance() != "{") {
            kError() << "importIDL: expecting '{' at " << name << endl;
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "union") {
        // TBD. <gulp>
        skipStmt("}");
        m_srcIndex++;  // advance to ';'
        return true;
    }
    if (keyword == "enum") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Enum,
                        name, m_scope[m_scopeIndex], m_comment);
        UMLEnum *enumType = static_cast<UMLEnum*>(ns);
        m_srcIndex++;  // skip name
        while (++m_srcIndex < srcLength && m_source[m_srcIndex] != "}") {
            Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
            if (advance() != ",")
                break;
        }
        skipStmt();
        return true;
    }
    if (keyword == "typedef") {
        const QString& existingType = advance();
        const QString& newType = advance();
        Import_Utils::createUMLObject(Uml::ot_Class, newType, m_scope[m_scopeIndex],
                                     m_comment, "CORBATypedef" /* stereotype */);
        // @todo How do we convey the existingType ?
        skipStmt();
        return true;
    }
    if (keyword == "const") {
        skipStmt();
        return true;
    }
    if (keyword == "custom") {
        return true;
    }
    if (keyword == "abstract") {
        m_isAbstract = true;
        return true;
    }
    if (keyword == "valuetype") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                        name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        m_klass->setAbstract(m_isAbstract);
        m_isAbstract = false;
        if (advance() == ";")   // forward declaration
            return true;
        if (m_source[m_srcIndex] == ":") {
            if (advance() == "truncatable")
                m_srcIndex++;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != "{") {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != ",")
                    break;
                m_srcIndex++;
            }
        }
        if (m_source[m_srcIndex] != "{") {
            kError() << "importIDL: ignoring excess chars at "
            << name << endl;
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "public") {
        return true;
    }
    if (keyword == "private") {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == "readonly") {
        m_isReadonly = true;
        return true;
    }
    if (keyword == "attribute") {
        m_isAttribute = true;
        return true;
    }
    if (keyword == "oneway") {
        m_isOneway = true;
        return true;
    }
    if (keyword == "}") {
        if (m_scopeIndex)
            m_klass = dynamic_cast<UMLClassifier*>(m_scope[--m_scopeIndex]);
        else
            kError() << "importIDL: too many }" << endl;
        m_srcIndex++;  // skip ';'
        return true;
    }
    if (keyword == ";")
        return true;
    // At this point, we expect `keyword' to be a type name
    // (of a member of struct or valuetype, or return type
    // of an operation.) Up next is the name of the attribute
    // or operation.
    if (! keyword.contains( QRegExp("^\\w") )) {
        kError() << "importIDL: ignoring " << keyword << endl;
        return false;
    }
    QString typeName = joinTypename();
    QString name = advance();
    if (name.contains( QRegExp("\\W") )) {
        kError() << "importIDL: expecting name in " << name << endl;
        return false;
    }
    // At this point we most definitely need a class.
    if (m_klass == NULL) {
        kError() << "importIDL: no class set for " << name << endl;
        return false;
    }
    QString nextToken = advance();
    if (nextToken == "(") {
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        m_srcIndex++;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != ")") {
            const QString &direction = m_source[m_srcIndex++];
            QString typeName = joinTypename();
            const QString &parName = advance();
            UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName);
            Uml::Parameter_Direction dir;
            if (Model_Utils::stringToDirection(direction, dir))
                att->setParmKind(dir);
            else
                kError() << "importIDL: expecting parameter direction at "
                << direction << endl;
            if (advance() != ",")
                break;
            m_srcIndex++;
        }
        Import_Utils::insertMethod(m_klass, op, Uml::Visibility::Public, typeName,
                                  false, false, false, false, m_comment);
        if (m_isOneway) {
            op->setStereotype("oneway");
            m_isOneway = false;
        }
        skipStmt();  // skip possible "raises" clause
        return true;
    }
    // At this point we know it's some kind of attribute declaration.
    while (1) {
        while (nextToken != "," && nextToken != ";") {
            name += nextToken;  // add possible array dimensions to `name'
            nextToken = advance();
        }
        UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name, typeName, m_comment);
        UMLAttribute *attr = static_cast<UMLAttribute*>(o);
        if (m_isReadonly) {
            attr->setStereotype("readonly");
            m_isReadonly = false;
        }
        if (nextToken != ",")
            break;
        name = advance();
        nextToken = advance();
    }
    return true;
}

