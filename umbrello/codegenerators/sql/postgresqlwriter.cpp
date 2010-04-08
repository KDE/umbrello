/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2010                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "postgresqlwriter.h"

#include "entity.h"
#include "umlentityattributelist.h"

#include <kdebug.h>
#include <klocale.h>

#include <QtCore/QList>

PostgreSQLWriter::PostgreSQLWriter()
{
}

PostgreSQLWriter::~PostgreSQLWriter()
{
}

/**
 * Returns "PostgreSQL".
 */
Uml::Programming_Language PostgreSQLWriter::language() const
{
    return Uml::pl_PostgreSQL;
}

/**
 * Reimplement method from CodeGenerator.
 */
QStringList PostgreSQLWriter::defaultDatatypes()
{
    QStringList l;

    l.append( "bigint" );
    l.append( "bigserial" );
    l.append( "bit" );
    l.append( "bit varying" );
    l.append( "boolean" );
    l.append( "box" );
    l.append( "bytea" );
    l.append( "character varying" );
    l.append( "character" );
    l.append( "cidr" );
    l.append( "circle" );
    l.append( "date" );
    l.append( "decimal" );
    l.append( "double precision" );
    l.append( "inet" );
    l.append( "integer" );
    l.append( "interval" );
    l.append( "line" );
    l.append( "lseg" );
    l.append( "macaddr" );
    l.append( "money" );
    l.append( "numeric" );
    l.append( "path" );
    l.append( "point" );
    l.append( "polygon" );
    l.append( "real" );
    l.append( "serial" );
    l.append( "smallint" );
    l.append( "time without timezone" );
    l.append( "time with timezone" );
    l.append( "timestamp without timezone" );
    l.append( "time with timezone" );

    return l;
}

/**
 * Reimplement printAutoIncrement statements from Base Class for PostgreSQL
 */
void PostgreSQLWriter::printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList)
{
    // rules
    // postgres has no such thing as auto increment
    // instead it uses sequences. For simulating auto increment, set default value of
    // each attribute to the nextval() of it's very own sequence

    foreach( UMLEntityAttribute* ea, entAttList ) {
        if ( !ea->getAutoIncrement() )
            continue;

        QString sequenceName;
        // we keep the sequence name as entityName + '_' + entityAttributeName
        sequenceName = m_pEntity->getName() + '_' + ea->getName();

        // we assume the sequence count starts with 1 and interval is 1 too
        // change the values when we start supporting different start values and
        // interval values

        sql<<"CREATE SEQUENCE "<<cleanName( sequenceName )
           <<" START 1 INCREMENT 1 ;";

        sql<<m_endl;

        // alter the table column ( set not null )
        sql<<"ALTER TABLE "<<cleanName( m_pEntity->getName() )
           <<" ALTER COLUMN "<<cleanName( ea->getName() )
           <<" SET NOT NULL ";

        sql<<m_endl;

        // alter the table column
        sql<<"ALTER TABLE "<<cleanName( m_pEntity->getName() )
           <<" ALTER COLUMN "<<cleanName( ea->getName() )
           <<" SET DEFAULT nextval('"<<cleanName( sequenceName )
           <<"');";

        sql<<m_endl;
    }

}

#include "postgresqlwriter.moc"
