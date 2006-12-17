/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2006                                                     *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

// own header
#include "pythonimport.h"

// qt/kde includes
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

PythonImport::PythonImport() : NativeImportBase("#") {
    setMultiLineComment("\"\"\"", "\"\"\"");
    initVars();
}

PythonImport::~PythonImport() {
}

void PythonImport::initVars() {
    m_srcIndentIndex = 0;
    m_srcIndent[m_srcIndentIndex] = 0;
    m_braceWasOpened = false;
}

bool PythonImport::preprocess(QString& line) {
    if (NativeImportBase::preprocess(line))
        return true;
    // Handle single line comment
    int pos = line.find(m_singleLineCommentIntro);
    if (pos != -1) {
        QString cmnt = line.mid(pos);
        m_source.append(cmnt);
        m_srcIndex++;
        if (pos == 0)
            return true;
        line = line.left(pos);
        line.remove( QRegExp("\\s+$") );
    }
    // Transform changes in indentation into braces a la C++/Java/Perl/...
    pos = line.find( QRegExp("\\S") );
    if (pos == -1)
        return true;
    bool isContinuation = false;
    int leadingWhite = line.left(pos).contains( QRegExp("\\s") );
    if (leadingWhite > m_srcIndent[m_srcIndentIndex]) {
        if (m_srcIndex == 0) {
            kError() << "PythonImport::preprocess(): internal error 1" << endl;
            return true;
        }
        if (m_braceWasOpened) {
            m_srcIndent[++m_srcIndentIndex] = leadingWhite;
            m_braceWasOpened = false;
        } else {
            isContinuation = true;
        }
    } else {
        while (m_srcIndentIndex > 0 && leadingWhite < m_srcIndent[m_srcIndentIndex]) {
            m_srcIndentIndex--;
            m_source.append("}");
            m_srcIndex++;
        }
    }
    if (line.endsWith(":")) {
        line.replace( QRegExp(":$"), "{" );
        m_braceWasOpened = true;
    } else {
        m_braceWasOpened = false;
    }
    if (!isContinuation && !m_braceWasOpened)
        line += ';';
    return false;  // The input was not completely consumed by preprocessing.
}

void PythonImport::fillSource(const QString& word) {
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; i++) {
        const QChar& c = word[i];
        if (c.isLetterOrNumber() || c == '_' || c == '.') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                m_srcIndex++;
                lexeme = QString();
            }
            m_source.append(QString(c));
            m_srcIndex++;
        }
    }
    if (!lexeme.isEmpty()) {
        m_source.append(lexeme);
        m_srcIndex++;
    }
}

void PythonImport::skipBody() {
    if (m_source[m_srcIndex] != "{")
        skipStmt("{");
    int braceNesting = 0;
    QString token;
    while (!(token = advance()).isNull()) {
        if (token == "}") {
            if (braceNesting <= 0)
                break;
            braceNesting--;
        } else if (token == "{") {
            braceNesting++;
        }
    }
}

bool PythonImport::parseStmt() {
    const uint srcLength = m_source.count();
    const QString& keyword = m_source[m_srcIndex];
    if (keyword == "class") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                        name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        m_comment = QString();
        if (advance() == "(") {
            while (m_srcIndex < srcLength - 1 && advance() != ")") {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != ",")
                    break;
            }
        }
        if (m_source[m_srcIndex] != "{") {
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "def") {
        if (m_klass == NULL) {
            // skip functions outside of a class
            skipBody();
            return true;
        }
        const QString& name = advance();
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        if (advance() != "(") {
            kError() << "importPython def " << name << ": expecting \"(\"" << endl;
            skipBody();
            return true;
        }
        while (m_srcIndex < srcLength && advance() != ")") {
            const QString& parName = m_source[m_srcIndex];
            UMLAttribute *att = Import_Utils::addMethodParameter(op, "string", parName);
            if (advance() != ",")
                break;
        }
        Import_Utils::insertMethod(m_klass, op, Uml::Visibility::Public, "string",
                                   false /*isStatic*/, false /*isAbstract*/, false /*isFriend*/,
                                   false /*isConstructor*/, m_comment);
        skipBody();
        return true;
    }
    if (keyword == "}") {
        if (m_scopeIndex)
            m_klass = dynamic_cast<UMLClassifier*>(m_scope[--m_scopeIndex]);
        else
            kError() << "importPython: too many }" << endl;
        return true;
    }
    return false;  // @todo parsing of attributes
}


