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
#include "pythonimport.h"

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

PythonImport::PythonImport() : NativeImportBase("#") {
    m_srcIndentIndex = 0;
    m_srcIndent[m_srcIndentIndex] = 0;
    m_braceWasOpened = false;
}

PythonImport::~PythonImport() {
}

bool PythonImport::preprocess(QString& line) {
    // Check for end of multi line comment.
    if (m_inComment) {
        int pos = line.find("\"\"\"");
        if (pos == -1) {
            m_comment += line + "\n";
            return true;  // done
        }
        if (pos > 0) {
            QString text = line.mid(0, pos - 1);
            m_comment += text.stripWhiteSpace();
        }
        m_source.append(m_singleLineCommentIntro + m_comment);  // denotes comments in `m_source'
        m_comment = "";
        m_inComment = false;
        pos += 3;  // pos now points behind the closed comment
        if (pos == (int)line.length())
            return true;  // done
        line = line.mid(pos);
    }
    // If we get here then m_inComment is false.
    // Check for start of multi line comment.
    int pos = line.find("\"\"\"");
    if (pos != -1) {
        int endpos = line.find("\"\"\"", pos + 3);
        if (endpos == -1) {
            m_inComment = true;
            if (pos + 3 < (int)line.length()) {
                QString cmnt = line.mid(pos + 3);
                m_comment += cmnt.stripWhiteSpace() + "\n";
            }
            if (pos == 0)
                return true;  // done
            line = line.left(pos);
        } else {   // It's a multiline comment on a single line.
            if (endpos > pos + 3)  {
                QString cmnt = line.mid(pos + 3, endpos - pos - 3);
                cmnt = cmnt.stripWhiteSpace();
                if (!cmnt.isEmpty())
                    m_source.append(m_singleLineCommentIntro + cmnt);
            }
            endpos += 3;  // endpos now points behind the closed comment
            QString pre;
            if (pos > 0)
                pre = line.left(pos);
            QString post;
            if (endpos < (int)line.length())
                post = line.mid(endpos);
            line = pre + post;
        }
        if (line.contains( QRegExp("^\\s*$") ))
            return true;
    }
    // Handle single line comment
    pos = line.find(m_singleLineCommentIntro);
    if (pos != -1) {
        QString cmnt = line.mid(pos);
        m_source.append(cmnt);
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
            kdError() << "PythonImport::preprocess(): internal error 1" << endl;
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

void PythonImport::fillSource(QString word) {
    QString lexeme;
    const uint len = word.length();
    QChar stringIntro = 0;  // buffers the string introducer character
    for (uint i = 0; i < len; i++) {
        const QChar& c = word[i];
        if (stringIntro) {        // we are in a string
            lexeme += c;
            if (c == stringIntro) {
                if (word[i - 1] != '\\') {
                    m_source.append(lexeme);
                    m_srcIndex++;
                    lexeme = QString::null;
                }
                stringIntro = 0;  // we are no longer in a string
            }
        } else if (c == '"' || c == '\'') {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                m_srcIndex++;
            }
            lexeme = stringIntro = c;
        } else if (c.isLetterOrNumber() || c == '_' || c == '.') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                m_srcIndex++;
                lexeme = QString::null;
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
    while ((token = advance()) != QString::null) {
        if (token == "}") {
            if (braceNesting <= 0)
                break;
            braceNesting--;
        } else if (token == "{") {
            braceNesting++;
        }
    }
}

void PythonImport::parseFile(QString filename) {
    if (filename.contains('/')) {
        QString path = filename;
        path.remove( QRegExp("/[^/]+$") );
        kdDebug() << "PythonImport::parseFile: adding path " << path << endl;
        Import_Utils::addIncludePath(path);
    }
    if (! QFile::exists(filename)) {
        if (filename.startsWith("/")) {
            kdError() << "PythonImport::parseFile(" << filename << "): cannot find file" << endl;
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
            kdError() << "PythonImport::parseFile(" << filename << "): cannot find file" << endl;
            return;
        }
    }
    QFile file(filename);
    if (! file.open(IO_ReadOnly)) {
        kdError() << "PythonImport::parseFile(" << filename << "): cannot open file" << endl;
        return;
    }
    // Scan the input file into the QStringList m_source.
    m_srcIndex = m_srcIndentIndex = 0;
    m_braceWasOpened = false;
    QTextStream stream(&file);
    while (! stream.atEnd()) {
        QString line = stream.readLine();
        scan(line);
    }
    file.close();
    // Parse the QStringList m_source.
    m_klass = NULL;
    m_currentAccess = Uml::Visibility::Public;
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
        if (keyword == "class") {
            const QString& name = advance();
            UMLObject *ns = Import_Utils::createUMLObject(Uml::ot_Class,
                            name, m_scope[m_scopeIndex], m_comment);
            m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
            m_comment = QString::null;
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
            continue;
        }
        if (keyword == "def") {
            const QString& name = advance();
            // operation
            UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
            if (advance() != "(") {
                kdError() << "importPython def " << name << ": expecting \"(\"" << endl;
                skipBody();
                continue;
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
            m_comment = QString::null;
            skipBody();
            continue;
        }
        if (keyword == "}") {
            if (m_scopeIndex)
                m_klass = dynamic_cast<UMLClassifier*>(m_scope[--m_scopeIndex]);
            else
                kdError() << "importPython: too many }" << endl;
            continue;
        }
        skipStmt(); continue;
        // At this point, we expect `keyword' to be a type name
        // (of a member of class or interface, or return type
        // of an operation.) Up next is the name of the attribute
        // or operation.
        if (! keyword.contains( QRegExp("^\\w") )) {
            kdError() << "importPython: ignoring " << keyword << endl;
            skipStmt();
            continue;
        }
        QString typeName; // = joinTypename();
        QString name;
        if (typeName == m_klass->getName()) {
            // Constructor.
            name = typeName;
            typeName = QString::null;
        } else {
            name = advance();
        }
        if (name.contains( QRegExp("\\W") )) {
            kdError() << "importPython: expecting name in " << name << endl;
            skipStmt();
            continue;
        }
        // At this point we need a class.
        if (m_klass == NULL) {
            kdError() << "importPython: no class set for " << name << endl;
            continue;
        }
        QString nextToken = advance();
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
            kdError() << "importPython: ignoring trailing items at " << name << endl;
            skipStmt();
        }
        m_comment = QString::null;
    }
}


