/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef POSTGRESQLWRITER_H
#define POSTGRESQLWRITER_H

#include "sqlwriter.h"

/**
  * class PostgreSQLWriter is a sql code generator for PostgreSQL
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate a sql source file for
  * that concept
  */
class PostgreSQLWriter : public SQLWriter
{
    Q_OBJECT
public:

    PostgreSQLWriter();
    virtual ~PostgreSQLWriter();

    Uml::ProgrammingLanguage::Enum language() const;

    QStringList defaultDatatypes() const;

protected:

    void printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList);

};

#endif // PostgreSQLWRITER_H
