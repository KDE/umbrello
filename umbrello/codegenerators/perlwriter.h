/***************************************************************************
                          perlwriter.h  -  description
                             -------------------
    begin                : Wed Jan 22 2003
    copyright            : (C) 2003 by David Hugh-Jones
    email                : hughjonesd@yahoo.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PERLWRITER_H
#define PERLWRITER_H

#include "../codegenerator.h"

#include <qlist.h>
#include <qstringlist.h>
class UMLOperation;
class UMLAttribute;

/**
  * class PerlWriter is a Perl code generator for UMLConcept objects
  * Just call writeClass and feed it a UMLConcept;
  */
class PerlWriter : public CodeGenerator {
public:

	PerlWriter( QObject* parent = 0, const char* name = 0 );
	virtual ~PerlWriter();

	/**
	  * call this method to generate Perl code for a UMLConcept
	  * @param c the class you want to generate code for.
	  */
	virtual void writeClass(UMLConcept *c);



private:

        /**
          * we do not want to write the comment "Private methods" twice
          * not sure whether this is php specific
          */
        bool bPrivateSectionCommentIsWritten;

	/**
	  * write all operations for a given class
	  *
	  * @param c the concept we are generating code for
	  * @param perl output stream for the Perl file
	  */
	void writeOperations(UMLConcept *c, QTextStream &perl);

	/**
	  * write a list of class operations
	  *
	  * @param classname the name of the class
	  * @param opList the list of operations
	  * @param perl output stream for the Perl file
	  */
	void writeOperations(QString classname, QList<UMLOperation> &opList,
	                     QTextStream &perl);

	/** write all the attributes of a class
	  * @param c the class we are generating code for
	  * @param perl output stream for the Perl file
	  */
	void writeAttributes(UMLConcept *c, QTextStream &perl);

	/** write a list of class attributes
	  * @param atList the list of attributes
	  * @param perl output stream for the Perl file
	  */
	void writeAttributes(QList<UMLAttribute> &atList, QTextStream &perl);


};


#endif //PERLWRITER
