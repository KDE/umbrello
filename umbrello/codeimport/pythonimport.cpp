/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2006-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "pythonimport.h"

// app includes
#include "umlattribute.h"
#include "umlclassifier.h"
#include "codeimpthread.h"
#define DBG_SRC QStringLiteral("PythonImport")
#include "debug_utils.h"
#include "umlenum.h"
#include "import_utils.h"
#include "umloperation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QRegularExpression>

DEBUG_REGISTER(PythonImport)

/**
 * Constructor.
 */
PythonImport::PythonImport(CodeImpThread* thread)
  : NativeImportBase(QStringLiteral("#"), thread)
{
    setMultiLineComment(QStringLiteral("\"\"\""), QStringLiteral("\"\"\""));
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
        line.remove(QRegularExpression(QStringLiteral("\\s+$")));
    }
    // Transform changes in indentation into braces a la C++/Java/Perl/...
    pos = line.indexOf(QRegularExpression(QStringLiteral("\\S")));
    if (pos == -1)
        return true;
    bool isContinuation = false;
    int leadingWhite = line.left(pos).count(QRegularExpression(QStringLiteral("\\s")));
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
            m_source.append(QStringLiteral("}"));
            m_srcIndex++;
        }
    }

    if (m_braceWasOpened && m_srcIndentIndex == 0) {
        m_source.append(QStringLiteral("}"));
        m_srcIndex++;
    }

    if (line.endsWith(QLatin1Char(':'))) {
        line.replace(QRegularExpression(QStringLiteral(":$")), QStringLiteral("{"));
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
    uint i = 0;
    if (word[0] == QLatin1Char('-') && len > 1) {
        const QChar& c1 = word[1];
        if (c1 == QLatin1Char('>')) {
            m_source.append(QStringLiteral("->"));
            i = 2;
        } else if (c1.isDigit()) {
            lexeme.append(word[0]).append(word[1]);
            i = 2;
        }
    }
    for (; i < len; ++i) {
        const QChar& c = word[i];
        if (c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.')) {
            lexeme += c;
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                m_srcIndex++;
                lexeme.clear();
            }
            QString tok(c);
            m_source.append(tok);
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
        spaces += QStringLiteral("  ");
    }
    return spaces;
}

/**
 * Skip ahead to outermost closing brace.
 * @param foundReturn  Optional pointer to Uml::PrimitiveTypes::Enum.
 *                     If given then the variable pointed to will be set if
 *                     a 'return' statement is encountered while skipping:
 *                     - If after 'return' there is a value True or False then
 *                       *foundReturn is set to Boolean;
 *                     - elsif after 'return' there is a number without decimal
 *                       point then *foundReturn is set to Integer;
 *                     - elsif after 'return' there is a number with decimal
 *                       point then *foundReturn is set to Real;
 *                     - else *foundReturn is set to String.
 *                     If no 'return' statement was encountered then
 *                     *foundReturn is set to Reserved.
 * @return  body contents skipped
 */
QString PythonImport::skipBody(Uml::PrimitiveTypes::Enum *foundReturn)
{
    /* During input preprocessing, changes in indentation were replaced by
       braces, and a semicolon was appended to each line ending.
       In order to return the body, we try to reconstruct the original Python
       syntax by reverting those changes.
     */
    QString body;
    if (foundReturn != nullptr)
        *foundReturn = Uml::PrimitiveTypes::Reserved;
    if (m_source[m_srcIndex] != QStringLiteral("{"))
        skipStmt(QStringLiteral("{"));
    bool firstTokenAfterNewline = true;
    bool dictInitializer = false;
    int braceNesting = 0;
    QString token;
    while (!(token = advance()).isNull()) {
        if (token == QStringLiteral("}")) {
            if (dictInitializer) {
                body += QLatin1Char('}');
                dictInitializer = false;
            } else {
                if (braceNesting <= 0)
                    break;
                braceNesting--;
            }
            body += QLatin1Char('\n');
            firstTokenAfterNewline = true;
        } else if (token == QStringLiteral("{")) {
            if (m_source[m_srcIndex - 1] == QStringLiteral("=")) {
                body += QLatin1Char('{');
                dictInitializer = true;
            } else {
                body += QLatin1Char(':');
                braceNesting++;
            }
            body += QLatin1Char('\n');
            firstTokenAfterNewline = true;
        } else if (token == QStringLiteral(";")) {
            body += QLatin1Char('\n');
            firstTokenAfterNewline = true;
        } else {
            if (firstTokenAfterNewline) {
                body += indentation(braceNesting);
                firstTokenAfterNewline = false;
                if (foundReturn != nullptr && token == QStringLiteral("return") &&
                        (*foundReturn == Uml::PrimitiveTypes::Reserved ||
                         *foundReturn == Uml::PrimitiveTypes::String)) {
                    QString next = lookAhead();
                    if (next == QStringLiteral("False") || next == QStringLiteral("True")) {
                        *foundReturn = Uml::PrimitiveTypes::Boolean;
                    } else if (next.contains(QRegularExpression(QStringLiteral("^-?\\d+$")))) {
                        *foundReturn = Uml::PrimitiveTypes::Integer;
                    } else if (next.contains(QRegularExpression(QStringLiteral("^-?\\d+\\.")))) {
                        *foundReturn = Uml::PrimitiveTypes::Real;
                    } else if (next != QStringLiteral("None")) {
                        *foundReturn = Uml::PrimitiveTypes::String;
                    }
                }
            } else if (body.contains(QRegularExpression(QStringLiteral("\\w$"))) &&
                       token.contains(QRegularExpression(QStringLiteral("^\\w")))) {
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
    if (_keyword == QStringLiteral("-"))
        keyword.append(advance());

    if (keyword == QStringLiteral("[")) {
        type = QStringLiteral("list");
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('['));
        for (int i = index; i <= m_srcIndex; i++)
            value += m_source[i];
    } else if (keyword == QStringLiteral("{")) {
        type = QStringLiteral("dict");
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('{'));
        for (int i = index; i <= m_srcIndex; i++)
            value += m_source[i];
    } else if (keyword == QStringLiteral("(")) {
        type = QStringLiteral("tuple");
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('('));
        for (int i = index; i <= m_srcIndex; i++)
            value += m_source[i];
    } else if (keyword.startsWith(QStringLiteral("\""))) {
        type = QStringLiteral("str");
        value = keyword;
    } else if (keyword == QStringLiteral("True") || keyword == QStringLiteral("False")) {
        type = QStringLiteral("bool");
        value = keyword;
    } else if (keyword.contains(QRegularExpression(QStringLiteral("-?\\d+\\.\\d*")))) {
        type = QStringLiteral("float");
        value = keyword;
    } else if (keyword.contains(QRegularExpression(QStringLiteral("-?\\d+")))) {
        type = QStringLiteral("int");
        value = keyword;
    } else if (keyword.toLower() == QStringLiteral("none")) {
        type = QStringLiteral("object");
        value = keyword;
    } else if (!keyword.isEmpty()) {
        if (lookAhead() == QStringLiteral("(")) {
            advance();
            type = keyword;
            int index = m_srcIndex;
            skipToClosing(QLatin1Char('('));
            for (int i = index; i <= m_srcIndex; i++)
                value += m_source[i];
        } else {
            type = QStringLiteral("object");
        }
    } else {
        type = QStringLiteral("object");
    }
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
    if (variableName.startsWith(QStringLiteral("self."))) {
        variableName.remove(0,5);
        isStatic = false;
    }
    Uml::Visibility::Enum visibility = Uml::Visibility::Public;
    if (variableName.startsWith(QStringLiteral("__"))) {
        visibility = Uml::Visibility::Private;
        variableName.remove(0, 2);
    } else if (variableName.startsWith(QStringLiteral("_"))) {
        visibility = Uml::Visibility::Protected;
        variableName.remove(0, 1);
    }

    QString type;
    QString initialValue;
    if (advance() == QStringLiteral("=")) {

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
    while (m_srcIndex < m_source.count() && advance() != QStringLiteral(")")) {
        const QString& parName = m_source[m_srcIndex];
        if (firstParam) {
            firstParam = false;
            if (parName.compare(QStringLiteral("self"), Qt::CaseInsensitive) == 0) {
                if (lookAhead() == QStringLiteral(","))
                    advance();
                continue;
            }
            m_isStatic = true;
        }
        QString type, value;
        QString next = lookAhead();
        if (next == QStringLiteral(":")) {
            advance();
            type = advance();
            if (lookAhead() == QStringLiteral("[")) {
                 int index = ++m_srcIndex;
                 skipToClosing(QLatin1Char('['));
                 for (int i = index; i <= m_srcIndex; i++) {
                     type += m_source[i];
                 }
             }
             next = lookAhead();
        }
        if (next == QStringLiteral("=")) {
            advance();
            QString iniType;
            parseInitializer(advance(), iniType, value);
            if (type.isEmpty())
                type = iniType;
        }
        UMLAttribute *attr = Import_Utils::addMethodParameter(op, type, parName);
        if (!value.isEmpty())
            attr->setInitialValue(value);
        if (lookAhead() == QStringLiteral(","))
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
    if (keyword == QStringLiteral("class")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class, name,
                                                      currentScope(), m_comment);
        pushScope(m_klass = ns->asUMLClassifier());
        m_comment.clear();
        if (advance() == QStringLiteral("(")) {
            while (m_srcIndex < srcLength - 1 && advance() != QStringLiteral(")")) {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != QStringLiteral(","))
                    break;
            }
        }
        if (m_source[m_srcIndex] != QStringLiteral("{")) {
            skipStmt(QStringLiteral("{"));
        }
        log(QStringLiteral("class ") + name);
        return true;
    }
    if (keyword == QStringLiteral("@")) {
        const QString& annotation = m_source[++m_srcIndex];
        logDebug1("PythonImport::parseStmt annotation: %1", annotation);
        if (annotation == QStringLiteral("staticmethod"))
            m_isStatic = true;
        return true;
    }
    if (keyword == QStringLiteral("def")) {
        if (m_klass == nullptr) {
            // skip functions outside of a class
            skipBody();
            return true;
        }

        if (!m_klass->hasDoc() && !m_comment.isEmpty()) {
            m_klass->setDoc(m_comment);
            m_comment = QString();
        }

        QString name = advance();
        bool isConstructor = name == QStringLiteral("__init__");
        Uml::Visibility::Enum visibility = Uml::Visibility::Public;
        if (!isConstructor) {
            if (name.startsWith(QStringLiteral("__"))) {
                name = name.mid(2);
                visibility = Uml::Visibility::Private;
            } else if (name.startsWith(QStringLiteral("_"))) {
                name = name.mid(1);
                visibility = Uml::Visibility::Protected;
            }
        }
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        if (advance() != QStringLiteral("(")) {
            logError1("PythonImport::parseStmt def %1: expecting \" (\"",  name);
            skipBody();
            return true;
        }
        if (!parseMethodParameters(op)) {
            logError1("PythonImport::parseStmt error on parsing method parameter for method %1", name);
            skipBody();
            return true;
        }
        // m_srcIndex is now at ")"
        int srcIndex = ++m_srcIndex;
        QString returnTypeName;
        if (current() == QStringLiteral("->")) {  // type hint
            returnTypeName = advance();
            if (returnTypeName == QStringLiteral("None"))
                returnTypeName.clear();
            ++m_srcIndex;
        }
        Uml::PrimitiveTypes::Enum foundReturn;
        const QString bodyCode(skipBody(&foundReturn));
        if (returnTypeName.isEmpty() && foundReturn != Uml::PrimitiveTypes::Reserved) {
            switch (foundReturn) {
            case Uml::PrimitiveTypes::Boolean :
                returnTypeName = QStringLiteral("bool");
                break;
            case Uml::PrimitiveTypes::Integer :
                returnTypeName = QStringLiteral("int");
                break;
            case Uml::PrimitiveTypes::Real :
                returnTypeName = QStringLiteral("float");
                break;
            case Uml::PrimitiveTypes::String :
                returnTypeName = QStringLiteral("str");
                break;
            default:
                break;
            }
        }
        Import_Utils::insertMethod(m_klass, op, visibility, returnTypeName,
                                   m_isStatic, false /*isAbstract*/, false /*isFriend*/,
                                   isConstructor, false, m_comment);
        op->setSourceCode(bodyCode);
        m_isStatic = false;

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
                if (lookAhead() == QStringLiteral("=")) {
                    parseAssignmentStmt(keyword);
                    // skip ; inserted by lexer
                    if (lookAhead() == QStringLiteral(";")) {
                        advance();
                        keyword = advance();
                    }
                } else {
                    skipStmt(QStringLiteral(";"));
                    keyword = advance();
                }
            }
            m_srcIndex = indexSave;
        }
        log(QStringLiteral("def ") + name);

        return true;
    }

    // parse class variables
    if (m_klass && lookAhead() == QStringLiteral("=")) {
        bool result = parseAssignmentStmt(keyword);
        log(QStringLiteral("class attribute ") + keyword);
        return result;
    }

    if (keyword == QStringLiteral("}")) {
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
