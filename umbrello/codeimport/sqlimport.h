/*
    SPDX-FileCopyrightText: 2015 Ralf Habacker <ralf.habacker@freenet.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SQLIMPORT_H
#define SQLIMPORT_H

#include "nativeimportbase.h"

class UMLEntity;
class UMLObject;

/**
 * Postgresql/mysql code import
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>

 * Bugs and comments to umbrello-devel@kde.org or https://bugs.kde.org
 */
class SQLImport : public QObject, public NativeImportBase {
    Q_OBJECT
public:
    explicit SQLImport(CodeImpThread* thread = nullptr);
    virtual ~SQLImport();

    bool parseStmt();

protected:
    class ColumnConstraints {
    public:
        ColumnConstraints()
          : primaryKey(false),
            uniqueKey(false),
            notNullConstraint(false),
            autoIncrement(false)
        {
        }

        bool primaryKey;
        bool uniqueKey;
        bool notNullConstraint;
        bool autoIncrement;
        QString collate;
        QString defaultValue;
        QString constraintName;
        QString characterSet;
    };

    class TableConstraints {
    public:
        TableConstraints()
          : primaryKey(false),
            uniqueKeys(false),
            notNullConstraint(false),
            autoIncrement(false),
            checkConstraint(false)
        {
        }

        bool primaryKey;
        bool uniqueKeys;
        bool notNullConstraint;
        bool autoIncrement;
        bool checkConstraint;
        QString checkExpression;
        QStringList primaryKeyFields;
        QString uniqueKeyName;
        QStringList uniqueKeysFields;
        QString collate;
        QString defaultValue;
        QString constraintName;
    };

    virtual QString advance();

    void fillSource(const QString &word);
    bool parseCreateTable(QString &token);
    bool parseAlterTable(QString &token);
    QString parseIdentifier(QString &token);
    QString parseDefaultExpression(QString &token);
    QStringList parseFieldType(QString &token);
    QStringList parseIdentifierList(QString &token);
    ColumnConstraints parseColumnConstraints(QString &token);
    TableConstraints parseTableConstraints(QString &token);
    bool parseCreateDefinition(QString &token, UMLEntity *entity);
    UMLObject *addDatatype(const QStringList &type);
    bool addPrimaryKey(UMLEntity *entity, const QString &name, const QStringList &fields);
    bool addUniqueConstraint(UMLEntity *entity, const QString &name, const QStringList &fields);
    bool addForeignConstraint(UMLEntity *entityA, const QString &name, const QStringList &fieldNames,
                              const QString &referencedTable, const QStringList &referencedFields);
};

#endif

