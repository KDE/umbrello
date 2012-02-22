/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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

// kde includes
#include <KStandardDirs>

// qt includes
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtCore/QRegExp>

#include <stdio.h>

QString IDLImport::m_preProcessor;
QStringList IDLImport::m_preProcessorArguments;
bool IDLImport::m_preProcessorChecked = false;

IDLImport::IDLImport(CodeImpThread* thread) : NativeImportBase("//", thread)
{
    m_isOneway = m_isReadonly = m_isAttribute = false;
    setMultiLineComment("/*", "*/");

    // we do not want to find the executable on each imported file
    if (m_preProcessorChecked) {
        m_enabled = !m_preProcessor.isEmpty();
        return; 
    }

    QStringList arguments;
    QString executable = KStandardDirs::findExe("cpp");
    if (!executable.isEmpty()) {
        arguments << "-C";   // -C means "preserve comments"
    }
#ifdef Q_WS_WIN
    else {
        executable = KStandardDirs::findExe("cl");
        if (executable.isEmpty()) {
	        QString path = qgetenv("VS100COMNTOOLS");
            if (!path.isEmpty())
                executable = KStandardDirs::findExe("cl", path + "/../../VC/bin");
        }
        if (!executable.isEmpty()) {
            arguments << "-E";   // -E means "preprocess to stdout"
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
    if (m_source[m_srcIndex] == "unsigned")
        typeName += ' ' + advance();
    if (m_source[m_srcIndex] == "long" &&
            (m_source[m_srcIndex + 1] == "long" || m_source[m_srcIndex + 1] == "double"))
        typeName += ' ' + advance();
    return typeName;
}

/**
 * Override operation from NativeImportBase.
 */
bool IDLImport::preprocess(QString& line)
{
    // Ignore C preprocessor generated lines.
    if (line.startsWith('#'))
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
        if (c.isLetterOrNumber() || c == '_') {
            lexeme += c;
        } else if (c == ':' && i < len-1 && word[i + 1] == ':') {
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
    if (filename.contains('/')) {
        QString path = filename;
        path.remove( QRegExp("/[^/]+$") );
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
        arguments << "-I" + path;
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
    m_scopeIndex = 0;
    m_scope[0] = NULL;
    const int srcLength = m_source.count();
    for (m_srcIndex = 0; m_srcIndex < srcLength; ++m_srcIndex) {
        const QString& keyword = m_source[m_srcIndex];
        //uDebug() << '"' << keyword << '"';
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
    if (keyword == "module") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                        name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = static_cast<UMLPackage*>(ns);
        m_scope[m_scopeIndex]->setStereotype("CORBAModule");
        if (advance() != "{") {
            uError() << "importIDL: unexpected: " << m_source[m_srcIndex];
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "interface") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class,
                        name, m_scope[m_scopeIndex], m_comment);
        m_klass = static_cast<UMLClassifier*>(ns);
        m_klass->setStereotype("CORBAInterface");
        m_klass->setAbstract(m_isAbstract);
        m_isAbstract = false;
        m_comment.clear();
        if (advance() == ";")   // forward declaration
            return true;
        m_scope[++m_scopeIndex] = m_klass;
        if (m_source[m_srcIndex] == ":") {
            while (++m_srcIndex < srcLength && m_source[m_srcIndex] != "{") {
                const QString& baseName = m_source[m_srcIndex];
                Import_Utils::createGeneralization(m_klass, baseName);
                if (advance() != ",")
                    break;
            }
        }
        if (m_source[m_srcIndex] != "{") {
            uError() << "importIDL: ignoring excess chars at " << name;
            skipStmt("{");
        }
        return true;
    }
    if (keyword == "struct" || keyword == "exception") {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class,
                        name, m_scope[m_scopeIndex], m_comment);
        m_scope[++m_scopeIndex] = m_klass = static_cast<UMLClassifier*>(ns);
        if (keyword == "struct")
            m_klass->setStereotype("CORBAStruct");
        else
            m_klass->setStereotype("CORBAException");
        if (advance() != "{") {
            uError() << "importIDL: expecting '{' at " << name;
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
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
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
        const QString& newType = advance();
        Import_Utils::createUMLObject(UMLObject::ot_Class, newType, m_scope[m_scopeIndex],
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
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class,
                        name, m_scope[m_scopeIndex], m_comment);
        m_klass = static_cast<UMLClassifier*>(ns);
        m_klass->setAbstract(m_isAbstract);
        m_isAbstract = false;
        if (advance() == ";")   // forward declaration
            return true;
        m_scope[++m_scopeIndex] = m_klass;
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
            uError() << "importIDL: ignoring excess chars at "
            << name;
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
            uError() << "importIDL: too many }";
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
        uError() << "importIDL: ignoring " << keyword;
        return false;
    }
    QString typeName = joinTypename();
    QString name = advance();
    if (name.contains( QRegExp("\\W") )) {
        uError() << "importIDL: expecting name in " << name;
        return false;
    }
    // At this point we most definitely need a class.
    if (m_klass == NULL) {
        uError() << "importIDL: no class set for " << name;
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
                uError() << "importIDL: expecting parameter direction at "
                << direction;
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

