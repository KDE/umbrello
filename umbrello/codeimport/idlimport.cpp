/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "idlimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "operation.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QProcess>
#include <QRegExp>
#include <QStandardPaths>
#include <QStringList>

#include <stdio.h>

QString IDLImport::m_preProcessor;
QStringList IDLImport::m_preProcessorArguments;
bool IDLImport::m_preProcessorChecked = false;

IDLImport::IDLImport(CodeImpThread* thread) : NativeImportBase(QLatin1String("//"), thread)
{
    m_isOneway = m_isReadonly = m_isAttribute = false;
    setMultiLineComment(QLatin1String("/*"), QLatin1String("*/"));

    // we do not want to find the executable on each imported file
    if (m_preProcessorChecked) {
        m_enabled = !m_preProcessor.isEmpty();
        return; 
    }

    QStringList arguments;
    QString executable = QStandardPaths::findExecutable(QLatin1String("cpp"));
    if (!executable.isEmpty()) {
        arguments << QLatin1String("-C");   // -C means "preserve comments"
    }
#ifdef Q_OS_WIN
    else {
        executable = QStandardPaths::findExecutable(QLatin1String("cl"));
        if (executable.isEmpty()) {
            QString path = QLatin1String(qgetenv("VS100COMNTOOLS").constData());
            if (!path.isEmpty())
                executable = QStandardPaths::findExecutable(QLatin1String("cl"), QStringList() << path + QLatin1String("/../../VC/bin"));
        }
        if (!executable.isEmpty()) {
            arguments << QLatin1String("-E");   // -E means "preprocess to stdout"
        }
    }
#endif
    if (!executable.isEmpty()) {
        m_preProcessor = executable;
        m_preProcessorArguments = arguments;
    }
    else {
        uError() << "Cannot find any of the supported preprocessors (gcc, Microsoft Visual Studio 2010)";
        m_enabled = false;
    }
    m_preProcessorChecked = true;
}

IDLImport::~IDLImport()
{
}

/// Check for split type names (e.g. unsigned long long)
QString IDLImport::joinTypename()
{
    QString typeName = m_source[m_srcIndex];
    if (m_source[m_srcIndex] == QLatin1String("unsigned"))
        typeName += QLatin1Char(' ') + advance();
    if (m_source[m_srcIndex] == QLatin1String("long") &&
            (m_source[m_srcIndex + 1] == QLatin1String("long") ||
             m_source[m_srcIndex + 1] == QLatin1String("double")))
        typeName += QLatin1Char(' ') + advance();
    return typeName;
}

/**
 * Override operation from NativeImportBase.
 */
bool IDLImport::preprocess(QString& line)
{
    // Ignore C preprocessor generated lines.
    if (line.startsWith(QLatin1Char('#')))
        return true;  // done

/**
 * Override operation from NativeImportBase.
 */
    return NativeImportBase::preprocess(line);
}

/**
 * Implement abstract operation from NativeImportBase.
 */
void IDLImport::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        QChar c = word[i];
        if (c.isLetterOrNumber() || c == QLatin1Char('_')) {
            lexeme += c;
        } else if (c == QLatin1Char(':') && i < len-1 && word[i + 1] == QLatin1Char(':')) {
            // compress scoped name into lexeme
            lexeme += QLatin1String("::");
            i++;
        } else if (c == QLatin1Char('<')) {
            // compress sequence or bounded string into lexeme
            do {
                lexeme += word[i];
            } while (word[i] != QLatin1Char('>') && ++i < len);
        } else {
            if (!lexeme.isEmpty()) {
                m_source.append(lexeme);
                lexeme.clear();
            }
            m_source.append(QString(c));
        }
    }
    if (!lexeme.isEmpty())
        m_source.append(lexeme);
}

/**
 * Reimplement operation from NativeImportBase.
 * Need to do this because we use the external C preprocessor.
 */
bool IDLImport::parseFile(const QString& filename)
{
    if (filename.contains(QLatin1Char('/'))) {
        QString path = filename;
        path.remove(QRegExp(QLatin1String("/[^/]+$")));
        uDebug() << "adding path " << path;
        Import_Utils::addIncludePath(path);
    }
    const QStringList includePaths = Import_Utils::includePathList();

    if (m_preProcessor.isEmpty()) { 
        uError() << "no preprocessor installed, could not import file";
        return false;
    }
    QStringList arguments(m_preProcessorArguments);

    QProcess p(UMLApp::app());
    for (QStringList::ConstIterator pathIt = includePaths.begin();
            pathIt != includePaths.end(); ++pathIt) {
        QString path = (*pathIt);
        arguments << QLatin1String("-I") + path;
    }
    arguments << filename;
    uDebug() << "importIDL: " << m_preProcessor << arguments;
    p.start(m_preProcessor, arguments);
    if (!p.waitForStarted()) {
        uError() << "could not run preprocessor";
        return false;
    }

    if (!p.waitForFinished()) {
        uError() << "could not run preprocessor";
        return false;
    }
    int exitCode = p.exitCode();
    if (exitCode  != 0) {
        uError() << "preprocessor returned error" << exitCode;
        return false;
    }

    QByteArray out = p.readAllStandardOutput();
    QTextStream data(out);

    // Scan the input file into the QStringList m_source.
    m_source.clear();
    while (!data.atEnd()) {
        NativeImportBase::scan(data.readLine());
    }

    // Parse the QStringList m_source.
    m_scope.clear();
    pushScope(0); // global scope
    const int srcLength = m_source.count();
    for (m_srcIndex = 0; m_srcIndex < srcLength; ++m_srcIndex) {
        const QString& keyword = m_source[m_srcIndex];
        //uDebug() << QLatin1Char('"') << keyword << QLatin1Char('"');
        if (keyword.startsWith(m_singleLineCommentIntro)) {
            m_comment = keyword.mid(m_singleLineCommentIntro.length());
            continue;
        }
        if (! parseStmt())
            skipStmt();
        m_currentAccess = Uml::Visibility::Public;
        m_comment.clear();
    }
    return true;
}

/**
 * Implement abstract operation from NativeImportBase.
 */
bool IDLImport::parseStmt()
{
    const QString& keyword = m_source[m_srcIndex];
    const int srcLength = m_source.count();
    uDebug() << "keyword is " << keyword;
    if (keyword == QLatin1String("module")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                        name, currentScope(), m_comment);
        pushScope(ns->asUMLPackage());
        currentScope()->setStereotype(QLatin1String("CORBAModule"));
        if (advance() != QLatin1String("{")) {
            uError() << "importIDL: unexpected: " << m_source[m_srcIndex];
            skipStmt(QLatin1String("{"));
        }
        return true;
    }
    if (keyword == QLatin1String("interface")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class,
                        name, currentScope(), m_comment);
        m_klass = ns->asUMLClassifier();
        m_klass->setStereotype(QLatin1String("CORBAInterface"));
        m_klass->setAbstract(m_isAbstract);
        m_isAbstract = false;
        m_comment.clear();
        if (advance() == QLatin1String(";"))   // forward declaration
            return true;
        pushScope(m_klass);
        if (m_source[m_srcIndex] == QLatin1String(":")) {
            while (++m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String("{")) {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != QLatin1String(","))
                    break;
            }
        }
        if (m_source[m_srcIndex] != QLatin1String("{")) {
            uError() << "importIDL: ignoring excess chars at " << name;
            skipStmt(QLatin1String("{"));
        }
        return true;
    }
    if (keyword == QLatin1String("struct") || keyword == QLatin1String("exception")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class,
                        name, currentScope(), m_comment);
        m_klass = ns->asUMLClassifier();
        pushScope(m_klass);
        if (keyword == QLatin1String("struct"))
            m_klass->setStereotype(QLatin1String("CORBAStruct"));
        else
            m_klass->setStereotype(QLatin1String("CORBAException"));
        if (advance() != QLatin1String("{")) {
            uError() << "importIDL: expecting '{' at " << name;
            skipStmt(QLatin1String("{"));
        }
        return true;
    }
    if (keyword == QLatin1String("union")) {
        // mostly TBD.
        const QString& name = advance();
        Import_Utils::createUMLObject(UMLObject::ot_Class,
                        name, currentScope(), m_comment, QLatin1String("CORBAUnion"));
        skipStmt(QLatin1String("}"));
        m_srcIndex++;  // advance to ';'
        return true;
    }
    if (keyword == QLatin1String("enum")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                        name, currentScope(), m_comment);
        UMLEnum *enumType = ns->asUMLEnum();
        if (enumType == 0)
            enumType = Import_Utils::remapUMLEnum(ns, enumType);
        m_srcIndex++;  // skip name
        while (++m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String("}")) {
            if (enumType != 0)
                Import_Utils::addEnumLiteral(enumType, m_source[m_srcIndex]);
            if (advance() != QLatin1String(","))
                break;
        }
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("typedef")) {
        const QString& oldType = advance();
        const QString& newType = advance();
        uDebug() << "oldType is " << oldType
                 << ", newType is " << newType
                 << ", scopeIndex is " << scopeIndex();
        Import_Utils::createUMLObject(UMLObject::ot_Class, newType, currentScope(),
                                     m_comment, QLatin1String("CORBATypedef") /* stereotype */);
        // @todo How do we convey the existingType ?
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("const")) {
        skipStmt();
        return true;
    }
    if (keyword == QLatin1String("custom")) {
        return true;
    }
    if (keyword == QLatin1String("abstract")) {
        m_isAbstract = true;
        return true;
    }
    if (keyword == QLatin1String("valuetype")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class,
                        name, currentScope(), m_comment);
        m_klass = ns->asUMLClassifier();
        m_klass->setAbstract(m_isAbstract);
        m_isAbstract = false;
        if (advance() == QLatin1String(";"))   // forward declaration
            return true;
        pushScope(m_klass);
        if (m_source[m_srcIndex] == QLatin1String(":")) {
            if (advance() == QLatin1String("truncatable"))
                m_srcIndex++;
            while (m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String("{")) {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != QLatin1String(","))
                    break;
                m_srcIndex++;
            }
        }
        if (m_source[m_srcIndex] != QLatin1String("{")) {
            uError() << "importIDL: ignoring excess chars at "
            << name;
            skipStmt(QLatin1String("{"));
        }
        return true;
    }
    if (keyword == QLatin1String("public")) {
        return true;
    }
    if (keyword == QLatin1String("private")) {
        m_currentAccess = Uml::Visibility::Private;
        return true;
    }
    if (keyword == QLatin1String("readonly")) {
        m_isReadonly = true;
        return true;
    }
    if (keyword == QLatin1String("attribute")) {
        m_isAttribute = true;
        return true;
    }
    if (keyword == QLatin1String("oneway")) {
        m_isOneway = true;
        return true;
    }
    if (keyword == QLatin1String("}")) {
        if (scopeIndex())
            m_klass = popScope()->asUMLClassifier();
        else
            uError() << "importIDL: too many }";
        m_srcIndex++;  // skip ';'
        return true;
    }
    if (keyword == QLatin1String(";"))
        return true;
    // At this point, we expect `keyword' to be a type name
    // (of a member of struct or valuetype, or return type
    // of an operation.) Up next is the name of the attribute
    // or operation.
    if (! keyword.contains(QRegExp(QLatin1String("^\\w")))) {
        uError() << "importIDL: ignoring " << keyword;
        return false;
    }
    QString typeName = joinTypename();
    QString name = advance();
    if (name.contains(QRegExp(QLatin1String("\\W")))) {
        uError() << "importIDL: expecting name in " << name;
        return false;
    }
    // At this point we most definitely need a class.
    if (m_klass == 0) {
        uError() << "importIDL: no class set for " << name;
        return false;
    }
    QString nextToken = advance();
    if (nextToken == QLatin1String("(")) {
        // operation
        UMLOperation *op = Import_Utils::makeOperation(m_klass, name);
        m_srcIndex++;
        while (m_srcIndex < srcLength && m_source[m_srcIndex] != QLatin1String(")")) {
            const QString &direction = m_source[m_srcIndex++];
            QString typeName = joinTypename();
            const QString &parName = advance();
            UMLAttribute *att = Import_Utils::addMethodParameter(op, typeName, parName);
            Uml::ParameterDirection::Enum dir;
            if (Model_Utils::stringToDirection(direction, dir))
                att->setParmKind(dir);
            else
                uError() << "importIDL: expecting parameter direction at "
                << direction;
            if (advance() != QLatin1String(","))
                break;
            m_srcIndex++;
        }
        Import_Utils::insertMethod(m_klass, op, Uml::Visibility::Public, typeName,
                                  false, false, false, false, false, m_comment);
        if (m_isOneway) {
            op->setStereotype(QLatin1String("oneway"));
            m_isOneway = false;
        }
        skipStmt();  // skip possible "raises" clause
        return true;
    }
    // At this point we know it's some kind of attribute declaration.
    while (1) {
        while (nextToken != QLatin1String(",") && nextToken != QLatin1String(";")) {
            name += nextToken;  // add possible array dimensions to `name'
            nextToken = advance();
        }
        UMLObject *o = Import_Utils::insertAttribute(m_klass, m_currentAccess, name, typeName, m_comment);
        UMLAttribute *attr = o->asUMLAttribute();
        if (m_isReadonly) {
            attr->setStereotype(QLatin1String("readonly"));
            m_isReadonly = false;
        }
        if (nextToken != QLatin1String(","))
            break;
        name = advance();
        nextToken = advance();
    }
    return true;
}

