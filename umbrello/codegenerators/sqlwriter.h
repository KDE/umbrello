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

private:

    /**
     * write all attributes for a given class
     * @param c the class for which we are generating code
     * @param j the stream associated with the output file
     */
    void writeAttributes(UMLClassifier *c, QTextStream &j);

    /**
     * Prints out attributes as columns in the table
     *
     * @param sql the stream we should print to
     * @param attributeList the attributes to be printed
     * @param first if the attributes are the first one
     */
    void printAttributes(QTextStream& sql, UMLAttributeList attributeList, bool first);
};

#endif // SQLWRITER_H
