/***************************************************************************
                          jswriter.h  -  description
                             -------------------
    begin                : Sat Feb 08 2003
    copyright            : (C) 2003 by Alexander Blum
    email                : blum@kewbee.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JSWRITER_H
#define JSWRITER_H

#include "../codegenerator.h"

#include <qptrlist.h>
#include <qstringlist.h>
class UMLOperation;
class UMLAttribute;

/**
  * class JSWriter is a JavaScript code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class JSWriter : public CodeGenerator {
public:

	JSWriter( QObject* parent = 0, const char* name = 0 );
	virtual ~JSWriter();

	/**
	  * call this method to generate Actionscript code for a UMLClassifier
	  * @param c the class you want to generate code for.
	  */
	virtual void writeClass(UMLClassifier *c);

private:

        /**
          * we do not want to write the comment "Private methods" twice
          */
        bool bPrivateSectionCommentIsWritten;

	/**
	  * write a list of class operations
	  *
	  * @param classname the name of the class
	  * @param opList the list of operations
	  * @param js output stream for the JS file
	  */
	void writeOperations(QString classname, QPtrList<UMLOperation> *opList,
	                     QTextStream &js);

};


#endif //JSWRITER
