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

#include "../codegenerator.h"

#include <qlist.h>
#include <qstringlist.h>


class UMLOperation;
class UMLAttribute;

/**
  * class SQLWriter is a code generator for UMLConcept objects.
  * Create an instance of this class, and feed it a UMLConcept when
  * calling writeClass and it will generate a sql source file for
  * that concept
  */
class SQLWriter : public CodeGenerator {
public:

	SQLWriter( QObject* parent = 0, const char* name = 0 );
	virtual ~SQLWriter();

	/**
	 * call this method to generate sql code for a UMLConcept
	 * @param c the class to generate code for
	 */
	virtual void writeClass(UMLConcept *c);
private:

	/**
	 * write all attributes for a given class
	 * @param c the class for which we are generating code
	 * @param j the stream associated with the output file
	 */
	void writeAttributes(UMLConcept *c, QTextStream &j);

	//FIXME doc
	void printAttributes(QTextStream& sql, QList<UMLAttribute> attributeList, bool first);
};



#endif // SQLWRITER_H
