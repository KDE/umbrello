/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2011                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "postgresqlwriter.h"

#include "entity.h"
#include "umlentityattributelist.h"

#include <klocale.h>

#include <QtCore/QList>
#include <QtCore/QTextStream>

PostgreSQLWriter::PostgreSQLWriter()
{
}

PostgreSQLWriter::~PostgreSQLWriter()
{
}

/**
 * Returns "PostgreSQL".
 */
Uml::ProgrammingLanguage PostgreSQLWriter::language() const
{
    return Uml::ProgrammingLanguage::PostgreSQL;
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
    l.append( "time without time zone" );
    l.append( "time with time zone" );
    l.append( "timestamp without time zone" );
    l.append( "time with time zone" );

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
        sequenceName = m_pEntity->name() + '_' + ea->name();

        // we assume the sequence count starts with 1 and interval is 1 too
        // change the values when we start supporting different start values and
        // interval values

        sql<<"CREATE SEQUENCE "<<cleanName( sequenceName )
           <<" START 1 INCREMENT 1 ;";

        sql<<m_endl;

        // alter the table column ( set not null )
        sql<<"ALTER TABLE "<<cleanName( m_pEntity->name() )
           <<" ALTER COLUMN "<<cleanName( ea->name() )
           <<" SET NOT NULL ";

        sql<<m_endl;

        // alter the table column
        sql<<"ALTER TABLE "<<cleanName( m_pEntity->name() )
           <<" ALTER COLUMN "<<cleanName( ea->name() )
           <<" SET DEFAULT nextval('"<<cleanName( sequenceName )
           <<"');";

        sql<<m_endl;
    }

}

#include "postgresqlwriter.moc"
