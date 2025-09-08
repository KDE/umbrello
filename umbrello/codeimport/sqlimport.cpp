/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

// own header
#include "sqlimport.h"

// app includes
#include "umlassociation.h"
#include "umlattribute.h"
#include "umlcheckconstraint.h"
#include "umlclassifier.h"
#define DBG_SRC QStringLiteral("SqlImport")
#include "debug_utils.h"
#include "umlenum.h"
#include "umlfolder.h"
#include "import_utils.h"
#include "umloperation.h"
#include "umlpackage.h"
#include "uml.h"
#include "umlentity.h"
#include "umlentityattribute.h"
#include "umlforeignkeyconstraint.h"
#include "uniqueconstraint.h"
#include "umldoc.h"
#include "umlpackagelist.h"

// qt includes
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>

#include <stdio.h>

DEBUG_REGISTER(SQLImport)

/**
 * Constructor.
 *
 * @param thread                  thread in which the code import runs
 */
SQLImport::SQLImport(CodeImpThread* thread) : NativeImportBase(QStringLiteral("--"), thread)
{
    setMultiLineComment(QStringLiteral("/*"), QStringLiteral("*/"));
}

/**
 * Destructor.
 */
SQLImport::~SQLImport()
{
}

/**
 * Implement abstract operation from NativeImportBase.
 */
void SQLImport::fillSource(const QString& word)
{
    QString lexeme;
    const uint len = word.length();
    for (uint i = 0; i < len; ++i) {
        QChar c = word[i];
        if (c.isLetterOrNumber() || c == QLatin1Char('_')) {
            lexeme += c;
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
 * Strip qoutes from identifier.
 *
 * @param token string with current token
 * @return stripped string
 */
QString &stripQuotes(QString &token)
{
    if (token.contains(QStringLiteral("\"")))
        token.replace(QStringLiteral("\""), QStringLiteral(""));
    else if (token.contains(QStringLiteral("`")))
        token.replace(QStringLiteral("`"), QStringLiteral(""));
    else if (token.contains(QStringLiteral("'")))
        token.replace(QStringLiteral("'"), QStringLiteral(""));
    return token;
}

/**
 * Parse identifier.
 *
 * @param token string with current token
 * @return parsed identifier
 */
QString SQLImport::parseIdentifier(QString &token)
{
    QString value;
    if (token == QStringLiteral("`")) { // mysql
        value = advance();
        token = advance();
    } else
        value = token;
    token = advance();
    if (token == QStringLiteral(".")) {
        // FIXME we do not support packages yet
#if 0
        value += token;
        value += advance();
        token = advance();
#else
        value = advance();
        token = advance();
#endif
    }

    return stripQuotes(value);
}

/**
 * Parse identifier list.
 *
 * @param token string with current token
 * @return string list with identifiers
 */
QStringList SQLImport::parseIdentifierList(QString &token)
{
    QStringList values;
    if (token == QStringLiteral("(")) {
        for (token = advance(); token != QStringLiteral(")");) {
            if (token == QStringLiteral(",")) {
                token = advance();
                continue;
            }
            QString value = parseIdentifier(token);
            values.append(value);
        }
        token = advance();
    }
    else {
        ;// error;
    }
    return values;
}

/**
 * Parse field type.
 *
 * @param token string with current token
 * @return string list containing field type (index 0), size/count (index 1) and optional values (index > 2)
 */
QStringList SQLImport::parseFieldType(QString &token)
{
    QString type = token;
    QString typeLength;

    // handle type extensions
    token = advance();

    // schema.type
    if (token == QStringLiteral(".")) {
        type += token;
        type += advance();
        token = advance();
        if (equiv(token, QStringLiteral("precision"))) {
            type += token;
            token = advance();
        }
    }

    if (equiv(type, QStringLiteral("enum"))) {
        QStringList values = parseIdentifierList(token);
        return QStringList() << type << QString() << values;
    }

    if (equiv(token, QStringLiteral("varying"))) {
        type += QStringLiteral(" ") + token;
        token = advance(); // '('
    }
    // (number) | (number,number)
    if (token == QStringLiteral("(")) {
        typeLength = advance(); // value
        token = advance();
        if (token == QStringLiteral(",")) {
            typeLength += token;
            typeLength += advance();
            token = advance();
        }
        token = advance();
    } else if (equiv(token, QStringLiteral("precision"))) {
        type += QStringLiteral(" ") + token;
        token = advance();
    }

    if (token == QStringLiteral("[")) {
        token = advance();
        if (token == QStringLiteral("]")) {
            type += QStringLiteral("[]");
            token = advance();
        }
    } else if (token.toLower().startsWith(QStringLiteral("with"))) {
        type += QStringLiteral(" ") + token;
        token = advance();
        type += QStringLiteral(" ") + token;
        token = advance();
        type += QStringLiteral(" ") + token;
        token = advance();
    } else if (equiv(token, QStringLiteral("unsigned"))) { // mysql
        token = advance();
    }
    return QStringList() << type << typeLength;
}

/**
 * Parse default expression.
 *
 * The expression could be in the form
 *    (expression)\::\<type\>
 *    function(expression)
 *
 * @param token string with current token
 * @return string with default expression
 */
QString SQLImport::parseDefaultExpression(QString &token)
{
    QString defaultValue;

    if (token == QStringLiteral("(")) {
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('('));
        token = advance();
        for (int i = index; i < m_srcIndex; i++)
            defaultValue += m_source[i];
    } else {
        defaultValue += token;
        token = advance();
    }
    if (token == (QStringLiteral(":"))) {
        defaultValue += token;
        token = advance();
        if (token == (QStringLiteral(":"))) {
            defaultValue += token;
            token = advance();
            defaultValue += parseFieldType(token).first();
        }
    }

    if (token == QStringLiteral("(")) {
        int index = m_srcIndex;
        skipToClosing(QLatin1Char('('));
        token = advance();
        for (int i = index; i < m_srcIndex; i++)
            defaultValue += m_source[i];
    }

    return defaultValue;
}

/**
 * Parse column constraint.
 *
 *  pgsql:
 *  [ CONSTRAINT constraint_name ]
 *  { NOT NULL |
 *    NULL |
 *    CHECK ( expression ) |
 *    COLLATE collation |
 *    DEFAULT default_expr |
 *    UNIQUE index_parameters |
 *    PRIMARY KEY index_parameters |
 *    REFERENCES reftable [ ( refcolumn ) ] [ MATCH FULL | MATCH PARTIAL | MATCH SIMPLE ]
 *      [ ON DELETE action ] [ ON UPDATE action ] }
 *  [ DEFERRABLE | NOT DEFERRABLE ] [ INITIALLY DEFERRED | INITIALLY IMMEDIATE ]
 *
 * mysql:
 *  [ PRIMARY KEY index_parameters |
 *    KEY key_name ( fields )
 *    CHARACTER SET charset_name |
 *    COLLATE collation ]
 *
 * @param token string with current token
 * @return column constraints
 */
SQLImport::ColumnConstraints SQLImport::parseColumnConstraints(QString &token)
{
    ColumnConstraints constraints;

    while (token != QStringLiteral(",") && token != QStringLiteral(")") && !equiv(token, QStringLiteral("comment"))) {
        const int origIndex = m_srcIndex;

        if (equiv(token, QStringLiteral("character"))) { // mysql
            token = advance(); // set
            if (equiv(token, QStringLiteral("set"))) {
                constraints.characterSet = advance(); // <value>
                token = advance();
            }
            else {
                m_srcIndex--; // take back
                token = m_source[m_srcIndex];
            }
        }

        if (equiv(token, QStringLiteral("collate"))) { // mysql
            constraints.collate = advance();
            token = advance();
        }

        // [ CONSTRAINT constraint_name ]
        if (equiv(token, QStringLiteral("constraint"))) {
            constraints.constraintName = advance();
            token = advance();
        }

        // NOT NULL
        if (equiv(token, QStringLiteral("not"))) {
            token = advance();
            if (equiv(token, QStringLiteral("null"))) {
                constraints.notNullConstraint = true;
                token = advance();
            }
        }

        // NULL
        if (equiv(token, QStringLiteral("null"))) {
            constraints.notNullConstraint = false;
            token = advance();
        }

        // CHECK ( expression )
        if (equiv(token, QStringLiteral("check"))) {
            skipStmt(QStringLiteral(")"));
            token = advance();
        }

        // DEFAULT default_expr
        if (equiv(token, QStringLiteral("default"))) {
            token = advance();
            constraints.defaultValue = parseDefaultExpression(token);
        }

        // UNIQUE index_parameters
        if (equiv(token, QStringLiteral("unique"))) {
            constraints.uniqueKey = true;
            token = advance();
            // WITH ( storage_parameter [= value] [, ... ] )
            if (equiv(token, QStringLiteral("with"))) {
                skipStmt(QStringLiteral(")"));
                token = advance();
            }
            // USING INDEX TABLESPACE tablespace
            if (equiv(token, QStringLiteral("using"))) {
                token = advance();
                token = advance();
                token = advance();
                token = advance();
            }
        }

        // PRIMARY KEY index_parameters
        if (equiv(token, QStringLiteral("primary"))) {
            token = advance();
            if (equiv(token, QStringLiteral("key"))) {
                constraints.primaryKey = true;
                token = advance();
                // WITH ( storage_parameter [= value] [, ... ] )
                if (equiv(token, QStringLiteral("with"))) {
                    skipStmt(QStringLiteral(")"));
                    token = advance();
                }
                // USING INDEX TABLESPACE tablespace
                if (equiv(token, QStringLiteral("using"))) {
                    token = advance();  // INDEX
                    token = advance();  // TABLESPACE
                    token = advance();  // tablespace
                    token = advance();
                }
            }
        }

        // REFERENCES reftable [ ( refcolumn ) ]
        if (equiv(token, QStringLiteral("references"))) {
            token = advance();
            token = advance();
            if (token == QStringLiteral("(")) {
                skipStmt(QStringLiteral(")"));
                token = advance();
            }

            // [ MATCH FULL | MATCH PARTIAL | MATCH SIMPLE ]
            if (equiv(token, QStringLiteral("match"))) {
                token = advance();
                token = advance();
            }

            // [ ON DELETE action ]
            if (equiv(token, QStringLiteral("on"))) {
                token = advance();
                token = advance();
                token = advance();
            }

            // [ ON UPDATE action ]
            if (equiv(token, QStringLiteral("on"))) {
                token = advance();
                token = advance();
                token = advance();
            }
        }

        // [ DEFERRABLE | NOT DEFERRABLE ]
        if (equiv(token, QStringLiteral("deferrable"))) {
            token = advance();
        }
        else if (equiv(token, QStringLiteral("not"))) {
            token = advance();
            token = advance();
        }

        // [ INITIALLY DEFERRED | INITIALLY IMMEDIATE ]
        if (equiv(token, QStringLiteral("initially"))) {
            token = advance();
            token = advance();
        }

        if (equiv(token, QStringLiteral("auto_increment"))) { // mysql
            constraints.autoIncrement = true;
            token = advance();
        }

        if (m_srcIndex == origIndex) {
            log(m_parsedFiles.first(), QStringLiteral("could not parse column constraint '") + token + QStringLiteral("'"));
            token = advance();
        }
    }
    if (equiv(token, QStringLiteral("comment"))) {
        while (token != QStringLiteral(",") && token != QStringLiteral(")")) {
            token = advance();
        }
    }
    return constraints;
}

/**
 * Parse table constraint.
 *
 * pgsql:
 *
 * [ CONSTRAINT constraint_name ]
 * { CHECK ( expression ) |
 *   UNIQUE ( column_name [, ... ] ) index_parameters |
 *   PRIMARY KEY ( column_name [, ... ] ) index_parameters |
 *   EXCLUDE [ USING index_method ] ( exclude_element WITH operator [, ... ] ) index_parameters [ WHERE ( predicate ) ] |
 *   FOREIGN KEY ( column_name [, ... ] ) REFERENCES reftable [ ( refcolumn [, ... ] ) ]
 *     [ MATCH FULL | MATCH PARTIAL | MATCH SIMPLE ] [ ON DELETE action ] [ ON UPDATE action ] }
 * [ DEFERRABLE | NOT DEFERRABLE ] [ INITIALLY DEFERRED | INITIALLY IMMEDIATE ]
 *
 * mysql:
 *   PRIMARY KEY (`uid`, `pid`) |
 *   KEY `t3ver_oid` (`t3ver_oid`,`t3ver_wsid`)
 *   UNIQUE KEY `entry_identifier` (`entry_namespace`,`entry_key`)
 *
 * @param token string with current token
 * @return table constraints
 */
SQLImport::TableConstraints SQLImport::parseTableConstraints(QString &token)
{
    TableConstraints constraints;

    if (equiv(token, QStringLiteral("constraint"))) {
        constraints.constraintName = advance();
        token = advance();
    }

    // CHECK ( expression )
    if (equiv(token, QStringLiteral("check"))) {
        token = advance();
        if (token == QStringLiteral("(")) {
            int index = m_srcIndex;
            skipToClosing(QLatin1Char('('));
            token = advance();
            constraints.checkConstraint = true;
            for (int i = index; i < m_srcIndex; i++)
                constraints.checkExpression += m_source[i];
        }
    }

    // PRIMARY KEY (`uid`, `pid`),
    if (equiv(token, QStringLiteral("primary"))) {
        token = advance(); // key
        token = advance(); // (
        constraints.primaryKey = true;
        constraints.primaryKeyFields = parseIdentifierList(token);
    }

    // UNIQUE KEY `entry_identifier` (`entry_namespace`,`entry_key`)
    else if (equiv(token, QStringLiteral("unique"))) {
        token = advance();
        token = advance();
        constraints.uniqueKeys = true;
        constraints.uniqueKeyName = parseIdentifier(token);
        constraints.uniqueKeysFields = parseIdentifierList(token);
    }

    // KEY `t3ver_oid` (`t3ver_oid`,`t3ver_wsid`) // mysql
    else if (equiv(token, QStringLiteral("key"))) {
        if (m_source[m_srcIndex+4] == QStringLiteral("(") ) {
            token = advance();
            constraints.uniqueKeys = true;
            constraints.uniqueKeyName = parseIdentifier(token);
            constraints.uniqueKeysFields = parseIdentifierList(token);
        }
    }

    return constraints;
}

/**
 * Parse table create definition.
 *
 * @param token string with current token
 * @param entity entity to save the definition into
 * @return true on success
 * @return false on error
 */
bool SQLImport::parseCreateDefinition(QString &token, UMLEntity *entity)
{
    if (token != QStringLiteral("(")) {
        skipStmt(QStringLiteral(";"));
        return false;
    }

    while (m_source.count() > m_srcIndex) {
        token = advance();

        if (token == QStringLiteral(")")) {
            break;
        }

        TableConstraints tableConstraints = parseTableConstraints(token);

        if (tableConstraints.primaryKey) {
            if (!addPrimaryKey(entity, tableConstraints.constraintName, tableConstraints.primaryKeyFields)) {
                ; // log error
            }
        }

        if (tableConstraints.uniqueKeys) {
            if (!addUniqueConstraint(entity, tableConstraints.uniqueKeyName, tableConstraints.uniqueKeysFields)) {
                ; // log error
            }
        }

        if (tableConstraints.checkConstraint) {
            if (entity) {
                QString name;
                if (!tableConstraints.constraintName.isEmpty())
                    name = tableConstraints.constraintName;
                else
                    name = entity->name() + QStringLiteral("_check");
                UMLCheckConstraint *cc = new UMLCheckConstraint(entity, name);
                cc->setCheckCondition(tableConstraints.checkExpression);
                entity->addConstraint(cc);
            } else {
                logError1("SQLImport::parseCreateDefinition: Could not add check constraint '%1' because of zero entity.",
                          tableConstraints.constraintName);
            }
        }

        if (token == QStringLiteral(","))
            continue;
        else if (token == QStringLiteral(")"))
            break;

        // handle field name
        QString fieldName = parseIdentifier(token);

        // handle field type
        QStringList fieldType = parseFieldType(token);
        SQLImport::ColumnConstraints constraints = parseColumnConstraints(token);

        logDebug2("SQLImport::parseCreateDefinition: field %1 type %2", fieldName, fieldType.at(0));
        if (entity && !fieldName.isEmpty()) {
            UMLObject *type = addDatatype(fieldType);
            UMLEntityAttribute *a = new UMLEntityAttribute(nullptr, fieldName,
                    Uml::ID::None,
                    Uml::Visibility::Public,
                    type);
            if (constraints.primaryKey)
                a->setIndexType(UMLEntityAttribute::Primary);
            a->setNull(!constraints.notNullConstraint);
            // convert index to value if present, see https://dev.mysql.com/doc/refman/8.0/en/enum.html
            if (UMLApp::app()->activeLanguage() == Uml::ProgrammingLanguage::MySQL && type->isUMLEnum()) {
                bool ok;
                int index = constraints.defaultValue.toInt(&ok);
                if (!ok) // string (not checked if valid) or empty
                    a->setInitialValue(constraints.defaultValue);
                else if (index > 0) {
                    index--; // 0 is empty
                    const UMLEnum *_enum = type->asUMLEnum();
                    UMLClassifierListItemList enumLiterals = _enum->getFilteredList(UMLObject::ot_EnumLiteral);
                    if (index < enumLiterals.size())
                        a->setInitialValue(enumLiterals.at(index)->name());
                }
            } else {
                a->setInitialValue(constraints.defaultValue);
            }
            a->setValues(fieldType.at(1));
            a->setAutoIncrement(constraints.autoIncrement);
            if (constraints.primaryKey) {
                UMLUniqueConstraint *pkey = new UMLUniqueConstraint(a, a->name() + QStringLiteral("_pkey"));
                entity->setAsPrimaryKey(pkey);
            }
            else if (constraints.uniqueKey) {
                UMLUniqueConstraint *uc = new UMLUniqueConstraint(a, a->name() + QStringLiteral("_unique"));
                entity->addConstraint(uc);
            }

            QStringList attributes;
            if (!constraints.characterSet.isEmpty())
                attributes.append(QStringLiteral("CHARACTER SET ") + constraints.characterSet);
            if (!constraints.collate.isEmpty())
                attributes.append(QStringLiteral("COLLATE ") + constraints.collate);
            if (attributes.size() > 0)
                a->setAttributes(attributes.join(QStringLiteral(" ")));

            entity->addEntityAttribute(a);
        } else if (!entity) {
            logError1("SQLImport::parseCreateDefinition: Could not add field '%1' because of zero entity.", fieldName);
        }
        if (token == QStringLiteral(","))
            continue;
        else if (token == QStringLiteral(")"))
            break;
    }
    token = advance();
    return true;
}

/**
 * Parse create table statement.
 *
 * @param token string with current token
 * @return true on success
 * @return false on error
 */
bool SQLImport::parseCreateTable(QString &token)
{
    bool returnValue = true;
    QString tableName = parseIdentifier(token);
    logDebug1("SQLImport::parseCreateTable: parsing create table %1", tableName);

    UMLFolder *folder = UMLApp::app()->document()->rootFolder(Uml::ModelType::EntityRelationship);
    UMLObject *o = Import_Utils::createUMLObject(UMLObject::ot_Entity,
                   tableName, folder, m_comment);
    UMLEntity *entity = o->asUMLEntity();
    m_comment.clear();
    if (equiv(token, QStringLiteral("as"))) {
        skipStmt(QStringLiteral(";"));
        return false;
    } else if (token == QStringLiteral("(")) {
        parseCreateDefinition(token, entity);
    } else {
        skipStmt(QStringLiteral(";"));
        return false;
    }
    if (equiv(token, QStringLiteral("inherits"))) {
        token = advance(); // (
        const QString &baseTable = advance();
        token = advance(); // )
        UMLObject *b = Import_Utils::createUMLObject(UMLObject::ot_Entity,
                       baseTable, folder, m_comment);
        UMLAssociation *a = new UMLAssociation(Uml::AssociationType::Generalization, o, b);
        if (entity)
            entity->addAssocToConcepts(a);
        else {
            logError1("SQLImport::parseCreateTable: Could not add generalization '%1' because of zero entity.",
                      baseTable);
            returnValue = false;
        }
    }

    skipStmt(QStringLiteral(";"));
    return returnValue;
}

/**
 * Parse alter table statement.
 *
 * @param token string with current token
 * @return true on success
 * @return false on error
 */
bool SQLImport::parseAlterTable(QString &token)
{
    if (equiv(token, QStringLiteral("only")))
        token = advance();

    QString tableName = token;
    token = advance();

    if (token == QStringLiteral(".")) {
        tableName += token;
        token = advance();
        if (token.contains(QStringLiteral("\"")))
            token.replace(QStringLiteral("\""), QStringLiteral(""));
        tableName += token;
        token = advance();
    }

    if (equiv(token, QStringLiteral("add"))) {
        token = advance();
        if (equiv(token, QStringLiteral("constraint"))) {
            const QString &constraintName = advance();
            token = advance();
            UMLFolder *folder = UMLApp::app()->document()->rootFolder(Uml::ModelType::EntityRelationship);
            UMLObject *o = UMLApp::app()->document()->findUMLObject(tableName, UMLObject::ot_Entity, folder);
            if (equiv(token, QStringLiteral("primary"))) {
                token = advance(); // key
                token = advance();
                const QStringList &fieldNames = parseIdentifierList(token);
                if (!o) {
                    // report error
                }
                UMLEntity *entity = o->asUMLEntity();
                if (!addPrimaryKey(entity, constraintName, fieldNames)) {
                    ; // reporter error
                }
            }
            else if (equiv(token, QStringLiteral("unique"))) {
                token = advance();
                const QStringList &fieldNames = parseIdentifierList(token);
                if (!o) {
                    // report error
                }
                UMLEntity *entity = o->asUMLEntity();
                if (!addUniqueConstraint(entity, constraintName, fieldNames)) {
                    ; // report error
                }
            }
            // FOREIGN KEY (<NAME>) REFERENCES <TABLE> (<FIELD>)
            else if (equiv(token, QStringLiteral("foreign"))) {
                token = advance(); // key
                token = advance();
                const QStringList &localFieldNames = parseIdentifierList(token);
                token = advance(); // references
                const QString &referencedTableName = parseIdentifier(token);
                const QStringList &referencedFieldNames = parseIdentifierList(token);
                // ON DELETE SET NULL DEFERRABLE INITIALLY DEFERRED;
                // use parseColumnConstraint()
                if (equiv(token, QStringLiteral("on"))) {
                    token = advance();
                    token = advance(); // delete/update
                    if (equiv(token, QStringLiteral("cascade")))
                        token = advance();
                }
                else if (equiv(token, QStringLiteral("match"))) {
                    token = advance();
                    token = advance(); // full
                }

                if (!o) {
                    // report error
                }
                UMLEntity *entity = o->asUMLEntity();
                if (!addForeignConstraint(entity, constraintName, localFieldNames, referencedTableName, referencedFieldNames)) {
                    ; // report error
                }
            }
        }
    } else
        skipStmt(QStringLiteral(";"));

    return true;
}

/**
 * Implement abstract operation from NativeImportBase.
 */
bool SQLImport::parseStmt()
{
    const QString& keyword = m_source[m_srcIndex];
    if (equiv(keyword, QStringLiteral("set"))) {
        skipStmt(QStringLiteral(";"));
        return true;
    }
    // CREATE [ [ GLOBAL | LOCAL ] { TEMPORARY | TEMP } | UNLOGGED ] TABLE [ IF NOT EXISTS ]
    else if (equiv(keyword, QStringLiteral("create"))) {
        QString type = advance();
        // [ GLOBAL | LOCAL ]
        if (equiv(type, QStringLiteral("global")))
            type = advance();
        else if (equiv(type, QStringLiteral("local")))
            type = advance();

        // [ { TEMPORARY | TEMP } | UNLOGGED ]
        if (equiv(type, QStringLiteral("temp")))
            type = advance();
        else if (equiv(type, QStringLiteral("temporary")))
            type = advance();

        if (equiv(type, QStringLiteral("unlogged")))
            type = advance();

        // TABLE
        if (equiv(type, QStringLiteral("table"))) {
            QString token = advance();
            // [ IF NOT EXISTS ]
            if (equiv(token, QStringLiteral("if"))) {
                token = advance();
                token = advance();
                token = advance();
            }
            return parseCreateTable(token);
        } else if (m_source[m_srcIndex] != QStringLiteral(";")) {
            skipStmt(QStringLiteral(";"));
            return true;
        }
    } else if (equiv(keyword, QStringLiteral("alter"))) {
        QString type = advance();
        if (equiv(type, QStringLiteral("table"))) {
            QString token = advance();
            return parseAlterTable(token);
        } else if (m_source[m_srcIndex] != QStringLiteral(";")) {
            skipStmt(QStringLiteral(";"));
            return true;
        }
    }
    return true;
}

/**
 * Implement virtual method
 * @return string with next token
 */
QString SQLImport::advance()
{
    QString token = NativeImportBase::advance();
    logDebug2("SQLImport::advance : index %1 token %2", m_srcIndex, token);
    return token;
}

UMLObject *SQLImport::addDatatype(const QStringList &type)
{
    UMLObject  *datatype = nullptr;
    UMLPackage *parent = UMLApp::app()->document()->datatypeFolder();
    if (equiv(type.at(0), QStringLiteral("enum"))) {
        QString name = Model_Utils::uniqObjectName(UMLObject::ot_Enum, parent, type.at(0));
        datatype = Import_Utils::createUMLObject(UMLObject::ot_Enum, name, parent);
        UMLEnum *enumType = datatype->asUMLEnum();
        if (enumType == nullptr)
            enumType = Import_Utils::remapUMLEnum(datatype, currentScope());
        if (enumType) {
            for (int i = 2; i < type.size(); i++) {
                Import_Utils::addEnumLiteral(enumType, type.at(i));
            }
        } else {
            logError0("SQLImport::addDatatype: Invalid dynamic cast to UMLEnum from datatype.");
        }
    } else {
        datatype = Import_Utils::createUMLObject(UMLObject::ot_Datatype, type.at(0), parent);
    }
    return datatype;
}

bool SQLImport::addPrimaryKey(UMLEntity *entity, const QString &_name, const QStringList &fields)
{
    if (!entity) {
        logError1("SQLImport::addPrimaryKey: Could not add primary key '%1' because of zero entity.", _name);
        return false;
    }

    QString name;
    if (_name.isEmpty())
        name = entity->name() + QStringLiteral("_pkey");
    else
        name = _name;

    for(UMLObject  *a : entity->getFilteredList(UMLObject::ot_EntityConstraint)) {
        if (a->name() == name)
            return false;
    }

    UMLUniqueConstraint *pkey = new UMLUniqueConstraint(entity, name);
    for(const QString &field: fields) {
        for(UMLEntityAttribute  *a : entity->getEntityAttributes()) {
            if (a->name() == field)
                pkey->addEntityAttribute(a);
        }
    }
    // update list view item to see 'P'
    bool state = UMLApp::app()->document()->loading();
    UMLApp::app()->document()->setLoading(false);

    bool result = entity->setAsPrimaryKey(pkey);

    UMLApp::app()->document()->setLoading(state);
    return result;
}

/**
 * Add UML object for unique constraint.
 *
 * @param entity entity object
 * @param _name unique constraint name
 * @param fields field list
 * @return true on success
 * @return false on error
 */
bool SQLImport::addUniqueConstraint(UMLEntity *entity, const QString &_name, const QStringList &fields)
{
    if (!entity) {
        logError1("SQLImport::addUniqueConstraint: Could not add unique constraint '%1' because of zero entity.",
                  _name);
        return false;
    }

    QString name;
    if (_name.isEmpty())
        name = entity->name() + QStringLiteral("_unique");
    else
        name = _name;

    for(UMLObject  *a : entity->getFilteredList(UMLObject::ot_EntityConstraint)) {
        if (a->name() == name)
            return false;
    }

    UMLUniqueConstraint *uc = new UMLUniqueConstraint(entity, name);
    for(const QString &field: fields) {
        for(UMLEntityAttribute  *a : entity->getEntityAttributes()) {
            if (a->name() == field)
                uc->addEntityAttribute(a);
        }
    }
    return entity->addConstraint(uc);
}

/**
 * Add UML object foreign constraint.
 *
 * @param entityA entity object the foreign constraint belongs
 * @param _name name of foreign constraint
 * @param fieldNames list of field names
 * @param referencedTable referenced table name
 * @param referencedFields list of referenced field names
 * @return true on success
 * @return false on error
 */
bool SQLImport::addForeignConstraint(UMLEntity *entityA, const QString &_name, const QStringList &fieldNames,
                                     const QString &referencedTable, const QStringList &referencedFields)
{
    if (!entityA) {
        logError1("SQLImport::addForeignConstraint: Could not add foreign constraint '%1' because of zero entity.",
                  _name);
        return false;
    }

    QString name;
    if (_name.isEmpty())
        name = entityA->name() + QStringLiteral("_foreign");
    else
        name = _name;

    for(UMLObject  *a : entityA->getFilteredList(UMLObject::ot_EntityConstraint)) {
        if (a->name() == name)
            return false;
    }

    UMLFolder *root = UMLApp::app()->document()->rootFolder(Uml::ModelType::EntityRelationship);
    UMLObject *o = UMLApp::app()->document()->findUMLObject(referencedTable, UMLObject::ot_Entity, root);
    UMLEntity *entityB = o->asUMLEntity();
    if (!entityB)
        return false;

    UMLForeignKeyConstraint *fc = new UMLForeignKeyConstraint(entityA, name);

    if (fieldNames.size() != referencedFields.size()) {
        return false;
    }

    fc->setReferencedEntity(entityB);

    for(int i = 0; i < fieldNames.size(); i++) {
        const QString &fieldA = fieldNames.at(i);
        const QString &fieldB = referencedFields.at(i);
        UMLEntityAttribute *aA = nullptr;
        UMLEntityAttribute *aB = nullptr;
        for(UMLEntityAttribute  *a : entityA->getEntityAttributes()) {
            if (a->name() == fieldA) {
                aA = a;
                break;
            }
        }

        for(UMLEntityAttribute  *a : entityB->getEntityAttributes()) {
            if (a->name() == fieldB) {
                aB = a;
                break;
            }
        }
        if (!aA || !aB)
            return false;
        fc->addEntityAttributePair(aA, aB);
    }
    return entityA->addConstraint(fc);
}

bool SQLImport::equiv(const QString &word, const QString &stringlit) {
    return word.compare(stringlit, Qt::CaseInsensitive) == 0;
}
