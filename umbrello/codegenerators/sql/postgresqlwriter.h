/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

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

    Uml::ProgrammingLanguage language() const;

    QStringList defaultDatatypes();

protected:

    void printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList);

};

#endif // PostgreSQLWRITER_H
