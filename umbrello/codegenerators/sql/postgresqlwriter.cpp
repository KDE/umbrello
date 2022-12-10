/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "postgresqlwriter.h"

#include "entity.h"
#include "umlentityattributelist.h"

#include <KLocalizedString>

#include <QList>
#include <QTextStream>

PostgreSQLWriter::PostgreSQLWriter()
{
}

PostgreSQLWriter::~PostgreSQLWriter()
{
}

/**
 * Returns "PostgreSQL".
 */
Uml::ProgrammingLanguage::Enum PostgreSQLWriter::language() const
{
    return Uml::ProgrammingLanguage::PostgreSQL;
}

/**
 * Reimplement method from CodeGenerator.
 */
QStringList PostgreSQLWriter::defaultDatatypes() const
{
    QStringList l;

    l.append(QLatin1String("bigint"));
    l.append(QLatin1String("bigserial"));
    l.append(QLatin1String("bit"));
    l.append(QLatin1String("bit varying"));
    l.append(QLatin1String("boolean"));
    l.append(QLatin1String("box"));
    l.append(QLatin1String("bytea"));
    l.append(QLatin1String("character varying"));
    l.append(QLatin1String("character"));
    l.append(QLatin1String("cidr"));
    l.append(QLatin1String("circle"));
    l.append(QLatin1String("date"));
    l.append(QLatin1String("decimal"));
    l.append(QLatin1String("double precision"));
    l.append(QLatin1String("inet"));
    l.append(QLatin1String("integer"));
    l.append(QLatin1String("interval"));
    l.append(QLatin1String("line"));
    l.append(QLatin1String("lseg"));
    l.append(QLatin1String("macaddr"));
    l.append(QLatin1String("money"));
    l.append(QLatin1String("numeric"));
    l.append(QLatin1String("path"));
    l.append(QLatin1String("point"));
    l.append(QLatin1String("polygon"));
    l.append(QLatin1String("real"));
    l.append(QLatin1String("serial"));
    l.append(QLatin1String("smallint"));
    l.append(QLatin1String("time without time zone"));
    l.append(QLatin1String("time with time zone"));
    l.append(QLatin1String("timestamp without time zone"));
    l.append(QLatin1String("timestamp with time zone"));

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
    // each attribute to the nextval() of its very own sequence

    foreach(UMLEntityAttribute* ea, entAttList) {
        if (!ea->getAutoIncrement())
            continue;

        QString sequenceName;
        // we keep the sequence name as entityName + '_' + entityAttributeName + '_seq'
        sequenceName = m_pEntity->name() + QLatin1Char('_') + ea->name() + QLatin1String("_seq");

        // we assume the sequence count starts with 1 and interval is 1 too
        // change the values when we start supporting different start values and
        // interval values

        sql<<"CREATE SEQUENCE "<<cleanName(sequenceName)
           <<" START 1 INCREMENT 1 ;";

        sql<<m_endl;

        // alter the table column (set not null)
        sql<<"ALTER TABLE "<<cleanName(m_pEntity->name())
           <<" ALTER COLUMN "<<cleanName(ea->name())
           <<" SET NOT 0;";

        sql<<m_endl;

        // alter the table column
        sql<<"ALTER TABLE "<<cleanName(m_pEntity->name())
           <<" ALTER COLUMN "<<cleanName(ea->name())
           <<" SET DEFAULT nextval('"<<cleanName(sequenceName)
           <<"');";

        sql<<m_endl;
    }

}

