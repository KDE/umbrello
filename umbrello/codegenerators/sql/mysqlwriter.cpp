/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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

    l.append(QLatin1String("ascii"));
    l.append(QLatin1String("bigint"));
    l.append(QLatin1String("bit"));
    l.append(QLatin1String("binary"));
    l.append(QLatin1String("blob"));
    l.append(QLatin1String("bool"));
    l.append(QLatin1String("char"));
    l.append(QLatin1String("charset"));
    l.append(QLatin1String("date"));
    l.append(QLatin1String("datetime"));
    l.append(QLatin1String("decimal"));
    l.append(QLatin1String("double"));
    l.append(QLatin1String("enum"));
    l.append(QLatin1String("float"));
    l.append(QLatin1String("integer"));
    l.append(QLatin1String("longblob"));
    l.append(QLatin1String("longtext"));
    l.append(QLatin1String("mediumblob"));
    l.append(QLatin1String("mediumint"));
    l.append(QLatin1String("mediumtext"));
    l.append(QLatin1String("serial"));
    l.append(QLatin1String("set"));
    l.append(QLatin1String("smallint"));
    l.append(QLatin1String("timestamp"));
    l.append(QLatin1String("time"));
    l.append(QLatin1String("tinyblob"));
    l.append(QLatin1String("tinyint"));
    l.append(QLatin1String("tinytext"));
    l.append(QLatin1String("text"));
    l.append(QLatin1String("unicode"));
    l.append(QLatin1String("varbinary"));
    l.append(QLatin1String("varchar"));

    return l;
}

/**
 * Reimplemented method from SQLWriter.
 */
void MySQLWriter::printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
    // we need to create an index on the referenced attributes before we can create a foreign key constraint in MySQL

    foreach(UMLClassifierListItem* cli, constrList) {
        UMLForeignKeyConstraint* fkc = cli->asUMLForeignKeyConstraint();

        QMap<UMLEntityAttribute*, UMLEntityAttribute*> attributeMap = fkc->getEntityAttributePairs();

        // get the attributes
        QList<UMLEntityAttribute*> eaList = attributeMap.keys();

        // convert to UMLEntityAttributeList
        UMLEntityAttributeList refAttList;
        foreach(UMLEntityAttribute* ea, eaList) {
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
    foreach(UMLEntityAttribute* ea, entAttList) {
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

