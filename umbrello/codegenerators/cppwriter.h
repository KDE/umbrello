/***************************************************************************
                          cppwriter.h  -  description
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

#ifndef CPPWRITER_H
#define CPPWRITER_H

#include "../codegenerator.h"

#include <qlist.h>
#include <qstringlist.h>
class UMLOperation;
class UMLAttribute;

/**
  * class CppWriter is a C++ code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class CppWriter : public CodeGenerator {
public:

	CppWriter( QObject* parent = 0, const char* name = 0 );
	virtual ~CppWriter();

	/**
	  * call this method to generate C++ code for a UMLClassifier
	  * @param c the class you want to generate code for.
	  */
	virtual void writeClass(UMLClassifier *c);

private:

	/**
	  * write all operations for a given class
	  *
	  * @param c the concept we are generating code for
	  * @param h output stream for the header file
	  * @param cpp output stream for the C++ file
	  */
	void writeOperations(UMLClassifier *c, QTextStream &h, QTextStream &cpp);

	/**
	  * write a list of class operations
	  *
	  * @param classname the name of the class
	  * @param opList the list of operations
	  * @param h output stream for the header file
	  * @param cpp output stream for the C++ file
	  */
	void writeOperations(QString classname, QList<UMLOperation> &opList,
	                     QTextStream &h, QTextStream &cpp);

	/** write all the attributes of a class
	  * @param c the class we are generating code for
	  * @param h output stream for the header file
	  */
	void writeAttributes(UMLClass *c, QTextStream &h);

	/** write a list of class attributes
	  * @param atList the list of attributes
	  * @param h output stream for the header file
	  */
	void writeAttributes(QList<UMLAttribute> &atList, QTextStream &h);


};


#endif //CPPWRITER
