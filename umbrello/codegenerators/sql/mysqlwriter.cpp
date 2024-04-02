/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "mysqlwriter.h"

#include "entity.h"
#include "entityattribute.h"
#include "foreignkeyconstraint.h"
#include "umlclassifierlistitemlist.h"
#include "umlentityattributelist.h"

#include <KLocalizedString>

#include <QList>
#include <QTextStream>

MySQLWriter::MySQLWriter()
{
}

MySQLWriter::~MySQLWriter()
{
}

/**
 * Returns "MySQL".
 */
Uml::ProgrammingLanguage::Enum MySQLWriter::language() const
{
    return Uml::ProgrammingLanguage::MySQL;
}

/**
 * Reimplement method from CodeGenerator.
 */
QStringList MySQLWriter::defaultDatatypes() const
{
    QStringList l;

    l.append(QStringLiteral("ascii"));
    l.append(QStringLiteral("bigint"));
    l.append(QStringLiteral("bit"));
    l.append(QStringLiteral("binary"));
    l.append(QStringLiteral("blob"));
    l.append(QStringLiteral("bool"));
    l.append(QStringLiteral("char"));
    l.append(QStringLiteral("charset"));
    l.append(QStringLiteral("date"));
    l.append(QStringLiteral("datetime"));
    l.append(QStringLiteral("decimal"));
    l.append(QStringLiteral("double"));
    l.append(QStringLiteral("enum"));
    l.append(QStringLiteral("float"));
    l.append(QStringLiteral("integer"));
    l.append(QStringLiteral("longblob"));
    l.append(QStringLiteral("longtext"));
    l.append(QStringLiteral("mediumblob"));
    l.append(QStringLiteral("mediumint"));
    l.append(QStringLiteral("mediumtext"));
    l.append(QStringLiteral("serial"));
    l.append(QStringLiteral("set"));
    l.append(QStringLiteral("smallint"));
    l.append(QStringLiteral("timestamp"));
    l.append(QStringLiteral("time"));
    l.append(QStringLiteral("tinyblob"));
    l.append(QStringLiteral("tinyint"));
    l.append(QStringLiteral("tinytext"));
    l.append(QStringLiteral("text"));
    l.append(QStringLiteral("unicode"));
    l.append(QStringLiteral("varbinary"));
    l.append(QStringLiteral("varchar"));

    return l;
}

/**
 * Reimplemented method from SQLWriter.
 */
void MySQLWriter::printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
    // we need to create an index on the referenced attributes before we can create a foreign key constraint in MySQL

    Q_FOREACH(UMLClassifierListItem* cli, constrList) {
        UMLForeignKeyConstraint* fkc = cli->asUMLForeignKeyConstraint();

        QMap<UMLEntityAttribute*, UMLEntityAttribute*> attributeMap = fkc->getEntityAttributePairs();

        // get the attributes
        QList<UMLEntityAttribute*> eaList = attributeMap.keys();

        // convert to UMLEntityAttributeList
        UMLEntityAttributeList refAttList;
        Q_FOREACH(UMLEntityAttribute* ea, eaList) {
            refAttList.append(ea);
        }

        // create an index on them
        SQLWriter::printIndex(sql, m_pEntity, refAttList);
    }

    SQLWriter::printForeignKeyConstraints(sql, constrList);
}

/**
 * Reimplement printAutoIncrements from Base Class for MySQL
 */
void MySQLWriter::printAutoIncrements(QTextStream& sql, const UMLEntityAttributeList entAttList)
{
    // rules
    // only one attribute can have an auto increment in a table in MySQL
    // and that attribute should have an index on it :/

    // get the first attribute of list with auto increment
    UMLEntityAttribute* autoIncrementEntAtt = 0;
    Q_FOREACH(UMLEntityAttribute* ea, entAttList) {
       if (ea->getAutoIncrement()) {
           autoIncrementEntAtt = ea;
           break;
       }
    }

    if (autoIncrementEntAtt == 0) {
        return;
    }

    // create an index on this attribute
    UMLEntityAttributeList indexList;
    indexList.append(autoIncrementEntAtt);

    printIndex(sql, m_pEntity, indexList);

    // now alter the table and this column to add the auto increment
    sql << "ALTER TABLE " << cleanName(m_pEntity->name())
        << " CHANGE " << cleanName(autoIncrementEntAtt->name())
        << " " << cleanName(autoIncrementEntAtt->name())
        << " " << cleanName(autoIncrementEntAtt->getTypeName())
        << " " << cleanName(autoIncrementEntAtt->getAttributes())
        << " " << " NOT NULL AUTO_INCREMENT ;";

    sql << m_endl;

    // we don't support start values currently, but when we do, uncomment the following
    //sql << " ALTER TABLE " << cleanName(m_pEntity->getName())
    //   <<" AUTO_INCREMENT = " << theValue;
    //sql << m_endl;
}

/**
 * Reimplemented from Base Class to print warning.
 */
void MySQLWriter::printCheckConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
    sql << m_endl;
    sql << "-- CHECK Constraints are not supported in Mysql (as of version 5.x)";
    sql << m_endl;
    sql << "-- But it'll parse the statements without error ";
    sql << m_endl;

    // call base class
    SQLWriter::printCheckConstraints(sql, constrList);
}

