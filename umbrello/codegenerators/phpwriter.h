/***************************************************************************
                          phpwriter.h  -  description
                             -------------------
    begin                : Thu Oct 17 2002
    copyright            : (C) 2002 by Heiko Nardmann
    email                : h.nardmann@secunet.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PHPWRITER_H
#define PHPWRITER_H

#include "../codegenerator.h"

#include <qptrlist.h>
#include <qstringlist.h>
class UMLOperation;
class UMLAttribute;

/**
  * class PhpWriter is a PHP code generator for UMLClassifier objects
  * Just call writeClass and feed it a UMLClassifier;
  */
class PhpWriter : public CodeGenerator {
public:

	PhpWriter( QObject* parent = 0, const char* name = 0 );
	virtual ~PhpWriter();

	/**
	  * call this method to generate Php code for a UMLClassifier
	  * @param c the class you want to generate code for.
	  */
	virtual void writeClass(UMLClassifier *c);

private:

        /**
          * we do not want to write the comment "Private methods" twice
          */
        bool bPrivateSectionCommentIsWritten;

	/**
	  * write all operations for a given class
	  *
	  * @param c the concept we are generating code for
	  * @param php output stream for the PHP file
	  */
	void writeOperations(UMLClassifier *c, QTextStream &php);

	/**
	  * write a list of class operations
	  *
	  * @param classname the name of the class
	  * @param opList the list of operations
	  * @param php output stream for the PHP file
	  */
	void writeOperations(QString classname, QPtrList<UMLOperation> &opList,
	                     QTextStream &php);

	/** write all the attributes of a class
	  * @param c the class we are generating code for
	  * @param php output stream for the PHP file
	  */
	void writeAttributes(UMLClass *c, QTextStream &php);

	/** write a list of class attributes
	  * @param atList the list of attributes
	  * @param php output stream for the PHP file
	  */
	void writeAttributes(QPtrList<UMLAttribute> &atList, QTextStream &php);


};


#endif //PHPWRITER
