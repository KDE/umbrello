/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "mysqlwriter.h"

#include "entity.h"
#include "entityattribute.h"
#include "foreignkeyconstraint.h"
#include "umlclassifierlistitemlist.h"
#include "umlentityattributelist.h"

#include <klocale.h>

#include <QtCore/QList>
#include <QtCore/QTextStream>

MySQLWriter::MySQLWriter()
{
}

MySQLWriter::~MySQLWriter()
{
}

/**
 * Returns "MySQL".
 */
Uml::ProgrammingLanguage MySQLWriter::language() const
{
    return Uml::ProgrammingLanguage::MySQL;
}

/**
 * Reimplement method from CodeGenerator.
 */
QStringList MySQLWriter::defaultDatatypes()
{
    QStringList l;

    l.append( "ascii" );
    l.append( "bigint" );
    l.append( "bit" );
    l.append( "binary" );
    l.append( "blob" );
    l.append( "bool" );
    l.append( "char" );
    l.append( "charset" );
    l.append( "date" );
    l.append( "datetime" );
    l.append( "decimal" );
    l.append( "double" );
    l.append( "enum" );
    l.append( "float" );
    l.append( "integer" );
    l.append( "longblob" );
    l.append( "longtext" );
    l.append( "mediumblob" );
    l.append( "mediumint" );
    l.append( "mediumtext" );
    l.append( "varbinary" );
    l.append( "varchar" );
    l.append( "serial" );
    l.append( "set" );
    l.append( "smallint" );
    l.append( "timestamp" );
    l.append( "time" );
    l.append( "tinyblob" );
    l.append( "tinyint" );
    l.append( "tinytext" );
    l.append( "text" );
    l.append( "unicode" );

    return l;
}

/**
 * Reimplemented method from SQLWriter.
 */
void MySQLWriter::printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList)
{
    // we need to create an index on the referenced attributes before we can create a foreign key constraint in MySQL

    foreach( UMLClassifierListItem* cli, constrList ) {
        UMLForeignKeyConstraint* fkc = static_cast<UMLForeignKeyConstraint*>(cli);

        QMap<UMLEntityAttribute*, UMLEntityAttribute*> attributeMap = fkc->getEntityAttributePairs();

        // get the referenced attributes
        QList<UMLEntityAttribute*> eaList = attributeMap.values();

        // convert to UMLEntityAttributeList
        UMLEntityAttributeList refAttList;
        foreach( UMLEntityAttribute* ea, eaList ) {
            refAttList.append( ea );
        }

        // create an index on them
        SQLWriter::printIndex( sql, fkc->getReferencedEntity(), refAttList );
    }

    SQLWriter::printForeignKeyConstraints( sql, constrList );
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
    UMLEntityAttribute* autoIncrementEntAtt = NULL;
    foreach(UMLEntityAttribute* ea, entAttList) {
       if ( ea->getAutoIncrement() ) {
           autoIncrementEntAtt = ea;
           break;
       }
    }

    if ( autoIncrementEntAtt == NULL ) {
        return;
    }

    // create an index on this attribute
    UMLEntityAttributeList indexList;
    indexList.append( autoIncrementEntAtt );

    printIndex( sql, m_pEntity, indexList );

    // now alter the table and this column to add the auto increment
    sql<<"ALTER TABLE "<<cleanName( m_pEntity->name() )
       <<" CHANGE "<<cleanName( autoIncrementEntAtt->name() )
       <<" "<<cleanName( autoIncrementEntAtt->name() )
       <<" "<<cleanName( autoIncrementEntAtt->getTypeName() )
       <<" "<<cleanName( autoIncrementEntAtt->getAttributes() )
       <<" "<<" NOT NULL AUTO_INCREMENT ;";

    sql<<m_endl;

    // we don't support start values currently, but when we do, uncomment the following
    //sql<<" ALTER TABLE "<<cleanName( m_pEntity->getName() )
    //   <<" AUTO_INCREMENT = "<<theValue;
    //sql<<m_endl;
}

/**
 * Reimplemented from Base Class to print warning.
 */
void MySQLWriter::printCheckConstraints(QTextStream& sql,UMLClassifierListItemList constrList)
{
    sql<<m_endl;
    sql<<"-- CHECK Constraints are not supported in Mysql ( as of version 5.x )";
    sql<<m_endl;
    sql<<"-- But it'll parse the statements without error ";

    // call base class
    SQLWriter::printCheckConstraints( sql, constrList );
}

#include "mysqlwriter.moc"
