/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef MYSQLWRITER_H
#define MYSQLWRITER_H

#include "sqlwriter.h"
#include "umlattributelist.h"

//forward declarations
class UMLClassifierListItemList;

/**
  * Class MySQLWriter is a sql code generator for MySQL.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate a sql source file for
  * that concept.
  */
class MySQLWriter : public SQLWriter
{
    Q_OBJECT
public:

    MySQLWriter();
    virtual ~MySQLWriter();

    Uml::ProgrammingLanguage language() const;

    QStringList defaultDatatypes();

    void printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList);
    void printCheckConstraints(QTextStream& sql,UMLClassifierListItemList constrList);

protected:

    virtual void printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList);

};

#endif // MySQLWRITER_H
