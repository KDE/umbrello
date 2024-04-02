/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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

    l.append(QStringLiteral("bigint"));
    l.append(QStringLiteral("bigserial"));
    l.append(QStringLiteral("bit"));
    l.append(QStringLiteral("bit varying"));
    l.append(QStringLiteral("boolean"));
    l.append(QStringLiteral("box"));
    l.append(QStringLiteral("bytea"));
    l.append(QStringLiteral("character varying"));
    l.append(QStringLiteral("character"));
    l.append(QStringLiteral("cidr"));
    l.append(QStringLiteral("circle"));
    l.append(QStringLiteral("date"));
    l.append(QStringLiteral("decimal"));
    l.append(QStringLiteral("double precision"));
    l.append(QStringLiteral("inet"));
    l.append(QStringLiteral("integer"));
    l.append(QStringLiteral("interval"));
    l.append(QStringLiteral("line"));
    l.append(QStringLiteral("lseg"));
    l.append(QStringLiteral("macaddr"));
    l.append(QStringLiteral("money"));
    l.append(QStringLiteral("numeric"));
    l.append(QStringLiteral("path"));
    l.append(QStringLiteral("point"));
    l.append(QStringLiteral("polygon"));
    l.append(QStringLiteral("real"));
    l.append(QStringLiteral("serial"));
    l.append(QStringLiteral("smallint"));
    l.append(QStringLiteral("time without time zone"));
    l.append(QStringLiteral("time with time zone"));
    l.append(QStringLiteral("timestamp without time zone"));
    l.append(QStringLiteral("timestamp with time zone"));

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

    Q_FOREACH(UMLEntityAttribute* ea, entAttList) {
        if (!ea->getAutoIncrement())
            continue;

        QString sequenceName;
        // we keep the sequence name as entityName + '_' + entityAttributeName + '_seq'
        sequenceName = m_pEntity->name() + QLatin1Char('_') + ea->name() + QStringLiteral("_seq");

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

