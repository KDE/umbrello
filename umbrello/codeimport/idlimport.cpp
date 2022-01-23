/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2005-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "idlimport.h"

// app includes
#include "attribute.h"
#include "classifier.h"
#include "codeimpthread.h"
#include "debug_utils.h"
#include "enum.h"
#include "import_utils.h"
#include "object_factory.h"
#include "operation.h"
#include "package.h"
#include "stereotype.h"
#include "uml.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// kde includes
#if QT_VERSION < 0x050000
#include <KStandardDirs>
#endif

// qt includes
#include <QProcess>
#include <QRegExp>
#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif
#include <QStringList>

#include <stdio.h>

QString IDLImport::m_preProcessor;
QStringList IDLImport::m_preProcessorArguments;
bool IDLImport::m_preProcessorChecked = false;

IDLImport::IDLImport(CodeImpThread* thread) : NativeImportBase(QLatin1String("//"), thread)
{
    m_doc = UMLApp::app()->document();
    m_isOneway = m_isReadonly = m_isAttribute = m_isUnionDefault = false;
    setMultiLineComment(QLatin1String("/*"), QLatin1String("*/"));

    // we do not want to find the executable on each imported file
    if (m_preProcessorChecked) {
        m_enabled = !m_preProcessor.isEmpty();
        return; 
    }

    QStringList arguments;
#if QT_VERSION >= 0x050000
    QString executable = QStandardPaths::findExecutable(QLatin1String("cpp"));
#else
    QString executable = KStandardDirs::findExe(QLatin1String("cpp"));
#endif
    if (!executable.isEmpty()) {
        arguments << QLatin1String("-C");   // -C means "preserve comments"
    }
#ifdef Q_OS_WIN
    else {
#if QT_VERSION >= 0x050000
        executable = QStandardPaths::findExecutable(QLatin1String("cl"));
#else
        executable = KStandardDirs::findExe(QLatin1String("cl"));
#endif
        if (executable.isEmpty()) {
            QString path = QLatin1String(qgetenv("VS100COMNTOOLS").constData());
            if (!path.isEmpty())
#if QT_VERSION >= 0x050000
                executable = QStandardPaths::findExecutable(QLatin1String("cl"), QStringList() << path + QLatin1String("/../../VC/bin"));
#else
                executable = KStandardDirs::findExe(QLatin1String("cl"), path + QLatin1String("/../../VC/bin"));
#endif
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
        log("Error: Cannot find any of the supported preprocessors (gcc, Microsoft Visual Studio 2010)");
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
        log("Error: no preprocessor installed, could not import file");
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
        log("Error: could not run preprocessor");
        return false;
    }

    if (!p.waitForFinished()) {
        log("Error: could not run preprocessor");
        return false;
    }
    int exitCode = p.exitCode();
    if (exitCode  != 0) {
        QString errMsg = QLatin1String("Error: preprocessor returned error");
        log(errMsg + QString::number(exitCode));
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
 * Skip to the end of struct/union/valuetype/interface declaration.
 *
 * @return True for success, false for non recoverable syntax error
 *         related to brace matching (missing opening or closing brace).
 */
bool IDLImport::skipStructure()
{
    bool status = skipToClosing(QLatin1Char('{'));  // skip to '}'
    // Skipping of ';' after '}' is done by NativeImportBase::parseFile
    return status;
}

/**
 * Returns true if the given text is a valid IDL scoped name.
 */
bool IDLImport::isValidScopedName(QString text) {
    QRegExp validScopedName(QLatin1String("^[A-Za-z_:][A-Za-z0-9_:]*$"));
    return text.contains(validScopedName);
}

/**
 * Implement abstract operation from NativeImportBase.
 * The function only returns false if an error is encountered from which
 * no recovery is possible.
 * On recoverable syntax errors, the function issues an error message,
 * skips to the end of the offending declaration, and returns true.
 * Returning true in spite of a local syntax error is done in the interest
 * of best effort (returning false would abort the entire code import).
 * A syntax error is typically unrecoverable when it involves imbalance of
 * braces, in particular when the closing "}" of an opening "{" is missing.
 */
bool IDLImport::parseStmt()
{
    QString keyword = m_source[m_srcIndex];
    const int srcLength = m_source.count();
    uDebug() << "keyword is " << keyword;
    if (keyword == QLatin1String("module")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Package,
                                                      name, currentScope(), m_comment);
        pushScope(ns->asUMLPackage());
        currentScope()->setStereotype(QLatin1String("idlModule"));
        if (advance() != QLatin1String("{")) {
            log(QLatin1String("Error: importIDL: unexpected: ") + m_source[m_srcIndex]);
            skipStmt(QLatin1String("{"));
        }
        return true;
    }
    if (keyword == QLatin1String("interface")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Class,
                                                      name, currentScope(), m_comment);
        m_klass = ns->asUMLClassifier();
        m_klass->setStereotype(QLatin1String("idlInterface"));
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
            log(QLatin1String("Error: importIDL: ignoring excess chars at ") + name);
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
            m_klass->setStereotype(QLatin1String("idlStruct"));
        else
            m_klass->setStereotype(QLatin1String("idlException"));
        if (advance() != QLatin1String("{")) {
            log(QLatin1String("Error: importIDL: expecting '{' at ") + name);
            skipStmt(QLatin1String("{"));
        }
        return true;
    }
    if (keyword == QLatin1String("union")) {
        const QString& name = advance();
        // create union type
        UMLObject *u = Import_Utils::createUMLObject(UMLObject::ot_Class,
                                                     name, currentScope(), m_comment,
                                                     QLatin1String("idlUnion"));
        if (!u) {
            log(QLatin1String("Error: importIDL: Could not create union ") + name);
            return skipStructure();
        }
        UMLClassifier *uc = u->asUMLClassifier();
        if (!uc) {
            log(QLatin1String("Error: importIDL(") + name +
                QLatin1String("): Expecting type ot_Class, actual type is ") +
                UMLObject::toString(u->baseType()));
            return skipStructure();
        }
        if (advance() != QLatin1String("switch")) {
            log(QLatin1String("Error: importIDL: expecting 'switch' after ") + name);
            return skipStructure();
        }
        if (advance() != QLatin1String("(")) {
            log(QLatin1String("Error: importIDL: expecting '(' after 'switch'"));
            return skipStructure();
        }
        const QString& switchType = advance();
        if (!isValidScopedName(switchType)) {
            log(QLatin1String("Error: importIDL: expecting typename after 'switch'"));
            return skipStructure();
        }
        // find or create discriminator type
        UMLObject *dt = Import_Utils::createUMLObject(UMLObject::ot_Class,
                         switchType, currentScope(), m_comment);
        if (!dt) {
            log(QLatin1String("Error: importIDL(") + name +
                                              QLatin1String("): Could not create switchtype ") + switchType);
            return skipStructure();
        }
        UMLClassifier *discrType = dt->asUMLClassifier();
        if (!discrType) {
            log(QLatin1String("Error: importIDL(") + name +
                QLatin1String(") swichtype: Expecting classifier type, found ") +
                UMLObject::toString(dt->baseType()));
            return skipStructure();
        }
        m_currentAccess = Uml::Visibility::Public;
        UMLAttribute *discr = Import_Utils::insertAttribute(uc, m_currentAccess,
                                                            QLatin1String("discriminator"),
                                                            discrType, m_comment);
        if (!discr) {
            log(QLatin1String("Error: importIDL(") + name +
                QLatin1String("): Could not create switch attribute"));
            return skipStructure();
        }
        discr->setStereotype(QLatin1String("idlSwitch"));
        if (advance() != QLatin1String(")")) {
            log(QLatin1String("Error: importIDL: expecting ')' after switch type"));
            return skipStructure();
        }
        if (advance() != QLatin1String("{")) {
            log(QLatin1String("Error: importIDL(") + name +
                QLatin1String("): expecting '{' after switch type"));
            return skipStructure();
        }
        m_klass = uc;
        pushScope(m_klass);
        m_unionCases.clear();
        m_isUnionDefault = false;
        return true;
    }
    if (keyword == QLatin1String("case")) {
        QString caseValue = advance();
        if (!isValidScopedName(caseValue)) {
            log(QLatin1String("Error: importIDL: expecting symbolic identifier after 'case'"));
            skipStmt(QLatin1String(":"));
        }
        m_unionCases.append(caseValue);
        if (advance() != QLatin1String(":")) {
            log(QLatin1String("Error: importIDL: expecting ':' after 'case'"));
        }
        return true;
    }
    if (keyword == QLatin1String("default")) {
        m_isUnionDefault = true;
        if (advance() != QLatin1String(":")) {
            log(QLatin1String("Error: importIDL: expecting ':' after 'default'"));
        }
        return true;
    }
    if (keyword == QLatin1String("enum")) {
        const QString& name = advance();
        UMLObject *ns = Import_Utils::createUMLObject(UMLObject::ot_Enum,
                        name, currentScope(), m_comment);
        UMLEnum *enumType = ns->asUMLEnum();
        if (enumType == 0)
            enumType = Import_Utils::remapUMLEnum(ns, currentScope());
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
        if (m_source[m_srcIndex + 2] == QLatin1String("<")) {
            keyword = advance();
            m_srcIndex += 2;
            QString oldType;
            QString bound;
            if (keyword == QLatin1String("sequence")) {
                oldType = joinTypename();
                if (advance() == QLatin1String(",")) {
                    bound = advance();
                    m_srcIndex++;  // position on ">"
                }
            } else if (keyword ==  QLatin1String("string")) {
                bound = m_source[m_srcIndex];
                m_srcIndex++;  // position on ">"
            } else {
                log(QLatin1String("Error: parseStmt: Expecting 'sequence' or 'string', found ") +
                    keyword);
                skipStmt();
                return true;
            }
            const QString& newType = advance();
            skipStmt();
            UMLObject::ObjectType ot = (oldType.length() ? UMLObject::ot_Class : UMLObject::ot_Datatype);
            UMLObject *pOld = m_doc->findUMLObject(oldType, UMLObject::ot_UMLObject, currentScope());
            if (pOld == 0) {
                pOld = Import_Utils::createUMLObject(ot, oldType, currentScope());
            }
            UMLObject *dt = Import_Utils::createUMLObject(ot, newType, currentScope(), m_comment);
            if (!dt) {
                log(QLatin1String("Error: importIDL(typedef ") + keyword +
                    QLatin1String("): Could not create datatype ") + newType);
                return true;
            }
            QString stereoName = (oldType.length() ? QLatin1String("idlSequence") : QLatin1String("idlString"));
            UMLStereotype *pStereo = m_doc->findStereotype(stereoName);
            if (pStereo == 0) {
                pStereo = m_doc->createStereotype(stereoName);
                UMLStereotype::AttributeDef tagDef(QLatin1String("bound"), Uml::PrimitiveTypes::UnlimitedNatural);
                // Empty bound stands for "unbounded".
                pStereo->getAttributeDefs().append(tagDef);
            }
            if (oldType.length()) {
                UMLAttribute *typeAttr = Object_Factory::createAttribute(dt, QLatin1String("members"), pOld);
                Q_UNUSED(typeAttr);
                /*
                UMLClassifier *cl = dt->asUMLClassifier();
                if (cl == 0) {
                    logError1("parseStmt(typedef %1) internal error: object returned by "
                              "Import_Utils::createUMLObject is not a Class", newType);
                    return false;
                } */
            }
            dt->setUMLStereotype(pStereo);
            dt->tags().append(bound);
            return true;
        }
        m_srcIndex++;
        const QString& oldType = joinTypename();
        const QString& newType = advance();
        uDebug() << "parseStmt(typedef) : oldType is " << oldType
                 << ", newType is " << newType
                 << ", scopeIndex is " << scopeIndex();
        UMLObject::ObjectType ot = UMLObject::ot_Class;
        UMLObject *pOld = m_doc->findUMLObject(oldType, UMLObject::ot_UMLObject, currentScope());
        if (pOld) {
            ot = pOld->baseType();
        } else {
            pOld = Import_Utils::createUMLObject(ot, oldType, currentScope());
        }
        UMLClassifier *oldClassifier = pOld->asUMLClassifier();
        UMLObject *pNew = Import_Utils::createUMLObject(ot, newType, currentScope(), m_comment,
                                                        QLatin1String("idlTypedef")); /* stereotype */
        UMLClassifier *newClassifier = pNew->asUMLClassifier();
        if (oldClassifier == 0) {
            log(QLatin1String("Error: importIDL(typedef ") + newType +
                QLatin1String("): Origin type ") + oldType + QLatin1String(" is not a classifier"));
        } else if (newClassifier == 0) {
            log(QLatin1String("Error: importIDL(typedef ") + newType +
                QLatin1String(") internal error: Import_Utils::createUMLObject did not return a classifier"));
        } else {
            Import_Utils::createGeneralization(newClassifier, oldClassifier);
        }
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
        m_klass->setStereotype(QLatin1String("idlValue"));
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
            log(QLatin1String("Error: importIDL: ignoring excess chars at ") + name);
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
            log(QLatin1String("Error: importIDL: too many }"));
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
        log(QLatin1String("Error: importIDL: ignoring ") + keyword);
        return false;
    }
    QString typeName = joinTypename();
    QString name = advance();
    if (name.contains(QRegExp(QLatin1String("\\W")))) {
        log(QLatin1String("Error: importIDL: expecting name in ") + name);
        return false;
    }
    // At this point we most definitely need a class.
    if (m_klass == 0) {
        log(QLatin1String("Error: importIDL: no class set for ") + name);
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
                log(QLatin1String("Error: importIDL: expecting parameter direction at ") +
                    direction);
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
        UMLAttribute *attr = Import_Utils::insertAttribute(m_klass, m_currentAccess, name, typeName, m_comment);
        if (m_isReadonly) {
            attr->setStereotype(QLatin1String("readonly"));
            m_isReadonly = false;
        }
        if (m_unionCases.count()) {
            const QString stereoName = QLatin1String("idlCase");
            UMLStereotype *pStereo = m_doc->findStereotype(stereoName);
            if (pStereo == 0) {
                pStereo = m_doc->createStereotype(stereoName);
                UMLStereotype::AttributeDef tagDef(QLatin1String("label"), Uml::PrimitiveTypes::String);
                pStereo->getAttributeDefs().append(tagDef);
            }
            attr->setUMLStereotype(pStereo);
            const QString caseLabels = m_unionCases.join(QChar(' '));
            attr->tags().append(caseLabels);
            m_unionCases.clear();
        } else if (m_isUnionDefault) {
            attr->setStereotype(QLatin1String("idlDefault"));
            m_isUnionDefault = false;
        }
        if (nextToken != QLatin1String(","))
            break;
        name = advance();
        nextToken = advance();
    }
    return true;
}

