/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Nikolaus Gradwohl  <guru@local-guru.net>      *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef SQLWRITER_H
#define SQLWRITER_H

#include "simplecodegenerator.h"
#include "umlattributelist.h"

//forward declarations
class UMLEntity;
class UMLEntityAttributeList;
class UMLClassifierListItemList;

/**
 * Class SQLWriter is a code generator for UMLClassifier objects.
 * Create an instance of this class, and feed it a UMLClassifier when
 * calling writeClass and it will generate a sql source file for
 * that concept.
 */
class SQLWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    SQLWriter();
    virtual ~SQLWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage language() const;

    virtual QStringList defaultDatatypes();

    virtual QStringList reservedKeywords() const;

protected:

    UMLEntity* m_pEntity;

    virtual void printEntityAttributes(QTextStream& sql, UMLEntityAttributeList entityAttributeList);
    virtual void printUniqueConstraints(QTextStream& sql, UMLClassifierListItemList constrList);
    virtual void printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList);
    virtual void printCheckConstraints(QTextStream& sql,UMLClassifierListItemList constrList);
    virtual void printIndex(QTextStream& sql, UMLEntity* ent, UMLEntityAttributeList entAttList);
    virtual void printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList );
};

#endif // SQLWRITER_H
