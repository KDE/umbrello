/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "pythonimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "codeimpthread.h"
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "operation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QtCore/QRegExp>

/**
 * Constructor.
 */
PythonImport::PythonImport(CodeImpThread* thread)
  : NativeImportBase("#", thread)
{
    setMultiLineComment("\"\"\"", "\"\"\"");
    initVars();
}

/**
 * Destructor.
 */
PythonImport::~PythonImport()
{
}

/**
 * Reimplement operation from NativeImportBase.
 */
void PythonImport::initVars()
{
    m_srcIndentIndex = 0;
    m_srcIndent[m_srcIndentIndex] = 0;
    m_braceWasOpened = false;
}

/**
 * Reimplement operation from NativeImportBase.
 * In addition to handling multiline comments, this method transforms
 * changes in leading indentation into braces (opening brace for increase
 * in indentation, closing brace for decrease in indentation) in m_source.
 * Removal of Python's indentation sensitivity simplifies subsequent
 * processing using Umbrello's native import framework.
 * @param line   the line to preprocess
 * @return success status of operation
 */
bool PythonImport::preprocess(QString& line)
{
    if (NativeImportBase::preprocess(line))
        return true;
    // Handle single line comment
    int pos = line.indexOf(m_singleLineCommentIntro);
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
    pos = line.indexOf( QRegExp("\\S") );
    if (pos == -1)
        return true;
    bool isContinuation = false;
    int leadingWhite = line.left(pos).count( QRegExp("\\s") );
    if (leadingWhite > m_srcIndent[m_srcIndentIndex]) {
        if (m_srcIndex == 0) {
            uError() << "internal error";
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
    if (line.endsWith(':')) {
        line.replace( QRegExp(":$"), "{" );
        m_braceWasOpened = true;
    } else {
        m_braceWasOpened = false;
    }
    if (!isContinuation && !m_braceWasOpened)
        line += ';';
    return false;  // The input was not completely consumed by preprocessing.
}

/**
 * Implement abstract operation from NativeImportBase.
 * @param word   whitespace delimited item
 */
void PythonImport::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        const QChar& c = word[i];
        if (c.isLetterOrNumber() || c == '_' || c == '.') {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                m_srcIndex++;
                lexeme.clear();
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

/**
 * Return an amount of spaces that corresponds to @param level
 * @return spaces of indentation
 */
QString PythonImport::indentation(int level)
{
    QString spaces;
    for (int i = 0; i < level; ++i) {
        spaces += "  ";
    }
    return spaces;
}

/**
 * Skip ahead to outermost closing brace.
 * @return  body contents skipped
 */
QString PythonImport::skipBody()
{
    /* During input preprocessing, changes in indentation were replaced by
       braces, and a semicolon was appended to each line ending.
       In order to return the body, we try to reconstruct the original Python
       syntax by reverting those changes.
     */
    QString body = ":\n";
    if (m_source[m_srcIndex] != "{")
        skipStmt("{");
    bool firstTokenAfterNewline = true;
    int braceNesting = 0;
    QString token;
    while (!(token = advance()).isNull()) {
        if (token == "}") {
            if (braceNesting <= 0)
                break;
            braceNesting--;
            body += '\n';
            firstTokenAfterNewline = true;
        } else if (token == "{") {
            braceNesting++;
            body += ":\n";
            firstTokenAfterNewline = true;
        } else if (token == ";") {
            body += '\n';
            firstTokenAfterNewline = true;
        } else {
            if (firstTokenAfterNewline) {
                body += indentation(braceNesting);
                firstTokenAfterNewline = false;
            } else if (body.contains(QRegExp("\\w$")) && token.contains(QRegExp("^\\w"))) {
                body += ' ';
            }
            body += token;
        }
    }
    return body;
}

/**
 * Implement abstract operation from NativeImportBase.
 * @return success status of operation
 */
bool PythonImport::parseStmt()
{
    const int srcLength = m_source.count();
    const QString& keyword = m_source[m_srcIndex];
    if (keyword == "class") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, name,
                                                      m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        m_comment.clear();
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
        log("class " + name);
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
            uError() << "importPython def " << name << ": expecting \"(\"";
            skipBody();
            return true;
        }
        while (m_srcIndex < srcLength && advance() != ")") {
            const QString& parName = m_source[m_srcIndex];
            /*UMLAttribute *att =*/ Import_Utils::addMethodParameter(op, "string", parName);
            if (advance() != ",")
                break;
        }
        Import_Utils::insertMethod(m_klass, op, Uml::Visibility::Public, "string",
                                   false /*isStatic*/, false /*isAbstract*/, false /*isFriend*/,
                                   false /*isConstructor*/, m_comment);
        op->setSourceCode(skipBody());
        log("def " + name);

        return true;
    }
    if (keyword == "}") {
        if (m_scopeIndex)
            m_klass = dynamic_cast<UMLClassifier*>(m_scope[--m_scopeIndex]);
        else
            uError() << "parsing: too many }";
        return true;
    }
    return false;  // @todo parsing of attributes
}
