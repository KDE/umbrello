/***************************************************************************
                          pythonwriter.h  -  description
                             -------------------
    begin                : Sat Dec 21 2002
    author               : Vincent Decorges
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PYTHONWRITER_H
#define PYTHONWRITER_H

#include "simplecodegenerator.h"
#include "../umloperationlist.h"

#include <qptrlist.h>
#include <qstringlist.h>

enum Access {PRIVATE, PUBLIC, PROTECTED};

/**
  * class PythonWriter is a python code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class PythonWriter : public SimpleCodeGenerator {
	Q_OBJECT
public:

	PythonWriter( UMLDoc * parent, const char* name = 0 );
	virtual ~PythonWriter();

	/**
	  * call this method to generate C++ code for a UMLClassifier
	  * @param c the class you want to generate code for.
	  */
	virtual void writeClass(UMLClassifier *c);

	virtual bool isType (QString & type);
	virtual QString getLanguage();

	void setSpaceIndent(int number);
	int getSpaceIndent(void);

private:

  QString spaceIndent;

	/**
	  * write all operations for a given class
	  *
	  * @param c the concept we are generating code for
	  * @param h output stream for the header file
	  */
	void writeOperations(UMLClassifier *c, QTextStream &h);

	/**
	  * write a list of class operations
	  *
	  * @param classname the name of the class
	  * @param opList the list of operations
	  * @param h output stream for the header file
	  */
	void writeOperations(QString classname, UMLOperationList &opList,
	                     QTextStream &h, Access access);


};


#endif //PYTHONWRITER
