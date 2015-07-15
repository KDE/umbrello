/*
    Copyright 2015  Ralf Habacker  <ralf.habacker@freenet.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SQLIMPORT_H
#define SQLIMPORT_H

#include "nativeimportbase.h"

class UMLEntity;

/**
 * Postgresql/mysql code import
 *
 * @author Ralf Habacker <ralf.habacker@freenet.de>

 * Bugs and comments to umbrello-devel@kde.org or http://bugs.kde.org
 */
class SQLImport : public QObject, public NativeImportBase {
    Q_OBJECT
public:
    explicit SQLImport(CodeImpThread* thread = 0);
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
    };

    class TableConstraints {
    public:
        TableConstraints()
          : primaryKey(false),
            uniqueKeys(false),
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
    bool addPrimaryKey(UMLEntity *entity, const QString &name, const QStringList &fields);
    bool addUniqueConstraint(UMLEntity *entity, const QString &name, const QStringList &fields);
    bool addForeignConstraint(UMLEntity *entityA, const QString &name, const QStringList &fieldNames,
                              const QString &referencedTable, const QStringList &referencedFields);
};

#endif

