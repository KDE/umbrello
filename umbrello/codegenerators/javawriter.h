/***************************************************************************
                          javawriter.h  -  description
                             -------------------
    begin                : Mon Jun 17 2002
    copyright            : (C) 2002 by Luis De la Parra Blum
    email                : luis@delaparra.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JAVAWRITER_H
#define JAVAWRITER_H

#include "../codegenerator.h"

#include <qlist.h>
#include <qstringlist.h>


class UMLOperation;

/**
  * class JavaWriter is a code generator for UMLConcept objects.
  * Create an instance of this class, and feed it a UMLConcept when
  * calling writeClass and it will generate a java source file for
  * that concept
  */
class JavaWriter : public CodeGenerator {
public:

	JavaWriter( QObject* parent = 0, const char* name = 0 );
	virtual ~JavaWriter();

	/**
	 * call this method to generate java code for a UMLConcept
	 * @param c the class to generate code for
	 */
	virtual void writeClass(UMLConcept *c);
private:

	/**
	 * write all operations for a given class
	 * @param c the class for which we are generating code
	 * @param j the stream associated with the output file
	 */
	void writeOperations(UMLConcept *c, QTextStream &j);

	/**
	 * write a list of operations for a given class
	 * @param list the list of operations you want to write
	 * @param j the stream associated with the output file
	 */
	void writeOperations(QList<UMLOperation> &list, QTextStream &j);

	/**
	 * write all attributes for a given class
	 * @param c the class for which we are generating code
	 * @param j the stream associated with the output file
	 */
	void writeAttributes(UMLConcept *c, QTextStream &j);


};



#endif // JAVAWRITER_H
