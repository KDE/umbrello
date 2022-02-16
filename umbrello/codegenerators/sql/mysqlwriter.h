/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

    Uml::ProgrammingLanguage::Enum language() const;

    QStringList defaultDatatypes() const;

    void printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList);
    void printCheckConstraints(QTextStream& sql, UMLClassifierListItemList constrList);

protected:

    virtual void printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList);

};

#endif // MySQLWRITER_H
