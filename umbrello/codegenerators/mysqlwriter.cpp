/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2007                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "mysqlwriter.h"

#include <kdebug.h>

#include <klocale.h>
#include <qlist.h>
#include <qlist.h>

#include "../foreignkeyconstraint.h"
#include "../entityattribute.h"
#include "../umlentityattributelist.h"
#include "../umlclassifierlistitemlist.h"

MySQLWriter::MySQLWriter() {
}

MySQLWriter::~MySQLWriter() {}


Uml::Programming_Language MySQLWriter::getLanguage() {
    return Uml::pl_MySQL;
}

QStringList MySQLWriter::defaultDatatypes() {
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


void MySQLWriter::printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList) {

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

#include "mysqlwriter.moc"
