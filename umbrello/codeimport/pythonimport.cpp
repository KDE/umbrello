/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "pythonimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "codeimpthread.h"
#define DBG_SRC QLatin1String("PythonImport")
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "operation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QRegExp>

DEBUG_REGISTER(PythonImport)

/**
 * Constructor.
 */
PythonImport::PythonImport(CodeImpThread* thread)
  : NativeImportBase(QLatin1String("#"), thread)
{
    setMultiLineComment(QLatin1String("\"\"\""), QLatin1String("\"\"\""));
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
    m_isStatic = false;
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
        line.remove(QRegExp(QLatin1String("\\s+$")));
    }
    // Transform changes in indentation into braces a la C++/Java/Perl/...
    pos = line.indexOf(QRegExp(QLatin1String("\\S")));
    if (pos == -1)
        return true;
    bool isContinuation = false;
    int leadingWhite = line.left(pos).count(QRegExp(QLatin1String("\\s")));
    if (leadingWhite > m_srcIndent[m_srcIndentIndex]) {
        if (m_srcIndex == 0) {
            logError0("PythonImport::preprocess internal error");
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
            m_source.append(QLatin1String("}"));
            m_srcIndex++;
        }
    }

    if (m_braceWasOpened && m_srcIndentIndex == 0) {
        m_source.append(QLatin1String("}"));
        m_srcIndex++;
    }

    if (line.endsWith(QLatin1Char(':'))) {
        line.replace(QRegExp(QLatin1String(":$")), QLatin1String("{"));
        m_braceWasOpened = true;
    } else {
        m_braceWasOpened = false;
    }
    if (!isContinuation && !m_braceWasOpened)
        line += QLatin1Char(';');
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
        if (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.')) {
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
        spaces += QLatin1String("  ");
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
    QString body;
    if (m_source[m_srcIndex] != QLatin1String("{"))
        skipStmt(QLatin1String("{"));
    bool firstTokenAfterNewline = true;
    int braceNesting = 0;
    QString token;
    while (!(token = advance()).isNull()) {
        if (token == QLatin1String("}")) {
            if (braceNesting <= 0)
                break;
            braceNesting--;
            body += QLatin1Char('\n');
            firstTokenAfterNewline = true;
        } else if (token == QLatin1String("{")) {
            braceNesting++;
            body += QLatin1String(":\n");
            firstTokenAfterNewline = true;
        } else if (token == QLatin1String(";")) {
            body += QLatin1Char('\n');
            firstTokenAfterNewline = true;
        } else {
            if (firstTokenAfterNewline) {
                body += indentation(braceNesting);
                firstTokenAfterNewline = false;
            } else if (body.contains(QRegExp(QLatin1String("\\w$"))) &&
                       token.contains(QRegExp(QLatin1String("^\\w")))) {
                body += QLatin1Char(' ');
            }
            body += token;
        }
    }
    return body;
}

/**
 * Parses a python initializer
 * @param _keyword current string from parser
 * @param type returns type of assignment
 * @param value returns assignment value
 * @return success status of parsing
 */
bool PythonImport::parseInitializer(const QString &_keyword, QString &type, QString &value)
{
    QString keyword = _keyword;
    if (_keyword == QLatin1String("-"))
        keyword.append(advance());

    if (keyword == QLatin1String("[")) {
        type = QLatin1String("list");
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('['));
        for (int i = index; i <= m_srcIndex; i++)
            value += m_source[i];
    } else if (keyword == QLatin1String("{")) {
        type = QLatin1String("dict");
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('{'));
        for (int i = index; i <= m_srcIndex; i++)
            value += m_source[i];
    } else if (keyword == QLatin1String("(")) {
        type = QLatin1String("tuple");
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('('));
        for (int i = index; i <= m_srcIndex; i++)
            value += m_source[i];
    } else if (keyword.startsWith(QLatin1String("\""))) {
        type = QLatin1String("string");
        value = keyword;
    } else if (keyword == QLatin1String("True") || keyword == QLatin1String("False")) {
        type = QLatin1String("bool");
        value = keyword;
    } else if (keyword.contains(QRegExp(QLatin1String("-?\\d+\\.\\d*")))) {
        type = QLatin1String("float");
        value = keyword;
    } else if (keyword.contains(QRegExp(QLatin1String("-?\\d+")))) {
        type = QLatin1String("int");
        value = keyword;
    } else if (keyword.toLower() == QLatin1String("none")) {
        type = QLatin1String("object");
        value = keyword;
    } else if (!keyword.isEmpty()) {
        if (lookAhead() == QLatin1String("(")) {
            advance();
            type = keyword;
            int index = m_srcIndex;
            skipToClosing(QLatin1Char('('));
            for (int i = index; i <= m_srcIndex; i++)
                value += m_source[i];
        } else
            type = QLatin1String("object");
    } else
        type = QLatin1String("object");
    return true;
}

/**
 * Parse assignments in the form \<identifier\> '=' \<value\>
 * Instance variables are identified by a prefixed 'self.'.
 * @param keyword current string from parser
 * @return success status of parsing
 */
bool PythonImport::parseAssignmentStmt(const QString &keyword)
{
    QString variableName = keyword;

    bool isStatic = true;
    if (variableName.startsWith(QLatin1String("self."))) {
        variableName.remove(0,5);
        isStatic = false;
    }
    Uml::Visibility::Enum visibility = Uml::Visibility::Public;
    if (variableName.startsWith(QLatin1String("__"))) {
        visibility = Uml::Visibility::Private;
        variableName.remove(0, 2);
    } else if (variableName.startsWith(QLatin1String("_"))) {
        visibility = Uml::Visibility::Protected;
        variableName.remove(0, 1);
    }

    QString type;
    QString initialValue;
    if (advance() == QLatin1String("=")) {

        if (!parseInitializer(advance(), type, initialValue))
            return false;
    }

    UMLObject* o = Import_Utils::insertAttribute(m_klass, visibility, variableName,
                                                 type, m_comment, false);
    UMLAttribute* a = o->asUMLAttribute();
    a->setInitialValue(initialValue);
    a->setStatic(isStatic);
    return true;
}

/**
 * Parses method parameter list
 * @param op UMLOperation instance to add parameter
 * @return success status of parsing
 */
bool PythonImport::parseMethodParameters(UMLOperation *op)
{
    bool firstParam = true;
    UMLAttribute *attr = nullptr;
    while (m_srcIndex < m_source.count() && advance() != QLatin1String(")")) {
        const QString& parName = m_source[m_srcIndex];
        if (attr && parName == QLatin1String("=")) {
            QString type, value;
            parseInitializer(advance(), type, value);
            attr->setInitialValue(value);
            attr->setTypeName(type);
        } else {
            if (firstParam) {
                if (parName.compare(QLatin1String("self"), Qt::CaseInsensitive) != 0) {
                    m_isStatic = true;
                    attr = Import_Utils::addMethodParameter(op, QLatin1String("string"), parName);
                }
                firstParam = false;
            } else {
                attr = Import_Utils::addMethodParameter(op, QLatin1String("string"), parName);
            }
        }
        if (lookAhead() == QLatin1String(","))
            advance();
    }
    return true;
}

/**
 * Implement abstract operation from NativeImportBase.
 * @return success status of operation
 */
bool PythonImport::parseStmt()
{
    const int srcLength = m_source.count();
    QString keyword = m_source[m_srcIndex];
    if (keyword == QLatin1String("class")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, name,
                                                      currentScope(), m_comment);
        pushScope(m_klass = ns->asUMLClassifier());
        m_comment.clear();
        if (advance() == QLatin1String("(")) {
            while (m_srcIndex < srcLength - 1 && advance() != QLatin1String(")")) {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != QLatin1String(","))
                    break;
            }
        }
        if (m_source[m_srcIndex] != QLatin1String("{")) {
            skipStmt(QLatin1String("{"));
        }
        log(QLatin1String("class ") + name);
        return true;
    }
    if (keyword == QLatin1String("@")) {
        const QString& annotation = m_source[++m_srcIndex];
        logDebug1("PythonImport::parseStmt annotation: %1", annotation);
        if (annotation == QLatin1String("staticmethod"))
            m_isStatic = true;
        return true;
    }
    if (keyword == QLatin1String("def")) {
        if (m_klass == 0) {
            // skip functions outside of a class
            skipBody();
            return true;
        }

        if (!m_klass->hasDoc() && !m_comment.isEmpty()) {
            m_klass->setDoc(m_comment);
            m_comment = QString();
        }

        QString name = advance();
        bool isConstructor = name == QLatin1String("__init__");
        Uml::Visibility::Enum visibility = Uml::Visibility::Public;
        if (!isConstructor) {
            if (name.startsWith(QLatin1String("__"))) {
                name = name.mid(2);
                visibility = Uml::Visibility::Private;
            } else if (name.startsWith(QLatin1String("_"))) {
                name = name.mid(1);
                visibility = Uml::Visibility::Protected;
            }
        }
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        if (advance() != QLatin1String("(")) {
            logError1("PythonImport::parseStmt def %1: expecting \" (\"",  name);
            skipBody();
            return true;
        }
        if (!parseMethodParameters(op)) {
            logError1("PythonImport::parseStmt error on parsing method parameter for method %1", name);
            skipBody();
            return true;
        }

        Import_Utils::insertMethod(m_klass, op, visibility, QLatin1String("string"),
                                   m_isStatic, false /*isAbstract*/, false /*isFriend*/,
                                   isConstructor, false, m_comment);
        m_isStatic = false;
        int srcIndex = m_srcIndex;
        op->setSourceCode(skipBody());

        if (!op->hasDoc() && !m_comment.isEmpty()) {
            op->setDoc(m_comment);
            m_comment = QString();
        }

        // parse instance variables from __init__ method
        if (isConstructor) {
            int indexSave = m_srcIndex;
            m_srcIndex = srcIndex;
            advance();
            keyword = advance();
            while (m_srcIndex < indexSave) {
                if (lookAhead() == QLatin1String("=")) {
                    parseAssignmentStmt(keyword);
                    // skip ; inserted by lexer
                    if (lookAhead() == QLatin1String(";")) {
                        advance();
                        keyword = advance();
                    }
                } else {
                    skipStmt(QLatin1String(";"));
                    keyword = advance();
                }
            }
            m_srcIndex = indexSave;
        }
        log(QLatin1String("def ") + name);

        return true;
    }

    // parse class variables
    if (m_klass && lookAhead() == QLatin1String("=")) {
        bool result = parseAssignmentStmt(keyword);
        log(QLatin1String("class attribute ") + keyword);
        return result;
    }

    if (keyword == QLatin1String("}")) {
        if (scopeIndex()) {
            m_klass = popScope()->asUMLClassifier();
        }
        else
            logError2("PythonImport::parseStmt: too many } at index %1 of %2",
                      m_srcIndex, m_source.count());
        return true;
    }
    return false;  // @todo parsing of attributes
}
