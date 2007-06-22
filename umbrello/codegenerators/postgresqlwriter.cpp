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

#include "postgresqlwriter.h"

#include <kdebug.h>

#include <klocale.h>
#include <qlist.h>

PostgreSQLWriter::PostgreSQLWriter() {
}

PostgreSQLWriter::~PostgreSQLWriter() {}


Uml::Programming_Language PostgreSQLWriter::getLanguage() {
    return Uml::pl_PostgreSQL;
}

QStringList PostgreSQLWriter::defaultDatatypes() {
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


#include "postgresqlwriter.moc"
