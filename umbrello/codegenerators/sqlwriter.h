/***************************************************************************
                          sqlwriter.h  -  description
                             -------------------
    begin                : 10.02.2003
    copyright            : (C) 2003 by Nikolaus Gradwohl
    email                : guru@local-guru.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SQLWRITER_H
#define SQLWRITER_H

#include "simplecodegenerator.h"
#include "../umlattributelist.h"

//forward declarations
class UMLEntity;
class UMLEntityAttributeList;
class UMLClassifierListItemList;

/**
  * class SQLWriter is a code generator for UMLClassifier objects.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate a sql source file for
  * that concept
  */
class SQLWriter : public SimpleCodeGenerator {
    Q_OBJECT
public:

    SQLWriter();
    virtual ~SQLWriter();

    /**
     * call this method to generate sql code for a UMLClassifier
     * @param c the class to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "SQL"
     */
    virtual Uml::Programming_Language getLanguage();

    /**
     * Reimplement method from CodeGenerator.
     */
    virtual QStringList defaultDatatypes();

    /**
     * get list of reserved keywords
     */
    virtual const QStringList reservedKeywords() const;

protected:

    UMLEntity* m_pEntity;

    /**
     * Prints out attributes as columns in the table
     *
     * @param sql the stream we should print to
     * @param entityAttributeList the attributes to be printed
     */
    virtual void printEntityAttributes(QTextStream& sql, UMLEntityAttributeList entityAttributeList);
   
    /**
     * Prints out unique constraints (including primary key ) as "ALTER TABLE" statements
     * @param sql the stream we should print to
     * @param constrList the unique constraints to be printed
     */
    virtual void printUniqueConstraints(QTextStream& sql, UMLClassifierListItemList constrList); 

    /**
     * Prints out foreign key constraints as "ALTER TABLE" statements
     * @param sql the stream we should print to
     * @param constrList the foreignkey constraints to be printed
     */
    virtual void printForeignKeyConstraints(QTextStream& sql, UMLClassifierListItemList constrList);


    /**
     * Prints out Check Constraints as "ALTER TABLE" statements
     * @param sql The stream we should print to
     * @param constrList The checkConstraints to be printed
     */
    virtual void printCheckConstraints(QTextStream& sql,UMLClassifierListItemList constrList);
    
    /**
     * Prints out Indexes as "CREATE INDEX " statements
     * @param sql The Stream we should print to
     * @param ent The Entity's attributes on which we want to create an Index
     * @param entAttList The list of entityattributes to create an index upon
     */
    virtual void printIndex(QTextStream& sql, UMLEntity* ent, UMLEntityAttributeList entAttList);

    /**
     * Handles AutoIncrements
     * The derived classes provide the actual body
     * @param sql The Stream we should print to
     * @param entAttList The List of Entity Attributes that we want to auto increment
     */
    virtual void printAutoIncrements(QTextStream& sql, UMLEntityAttributeList entAttList );
};

#endif // SQLWRITER_H
