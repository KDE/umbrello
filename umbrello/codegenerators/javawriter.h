/***************************************************************************
                          javawriter.h  -  description
                             -------------------
    copyright            : (C) 2003 Brian Thomas
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
#include "../attribute.h"
#include "../association.h"

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

	/**
	 * Constructor, initialises a couple of variables
	 */
	JavaWriter(QObject* parent = 0, const char* name = 0);

	/**
	 * Destructor, empty
	 */
	virtual ~JavaWriter();

	/**
	 * call this method to generate java code for a UMLConcept
	 * @param c the class to generate code for
	 */
	virtual void writeClass(UMLConcept *c);

private:

	/**
	 * Writes class's documentation then the class header
	 * public abstract class Foo extents {
	 */
	void writeClassDecl(UMLConcept *c, QTextStream &java); 

	/**
	 * Writes the comment and class constructor
	 */
	void writeConstructor(UMLConcept *c, QTextStream &java); 

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

	/**
	 * writes the Attribute declarations
	 * @param atpub List of public attributes
	 * @param atprot list of protected attributes
	 * @param atpriv list of private attributes
	 * @param java text stream
	 */
	void writeAttributeDecls(QList<UMLAttribute> &atpub, QList<UMLAttribute> &atprot,
				 QList<UMLAttribute> &atpriv, QTextStream &java ); 

	/**
	 * Searches a list of associations for appropriate ones to write out as attributes
	 */
	void writeAssociationDecls(QPtrList<UMLAssociation> associations, int id, QTextStream &java);

	/**
	 * Writes out an association as an attribute using Vector
	 */
	void writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi, Scope visib, 
				      QTextStream &java); 

	/**
	 * calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in atpub
	 */
	void writeAttributeMethods(QList<UMLAttribute> &atpub, Scope visibility, QTextStream &java);

	/**
	 * calls @ref writeAssociationRoleMethod() on each of the associations in the given list
	 */
	void writeAssociationMethods(QPtrList<UMLAssociation> associations, UMLConcept *thisClass,
				     QTextStream &java); 
	
	/**
	 * calls @ref writeSingleAttributeAccessorMethods() or @ref
	 * writeVectorAttributeAccessorMethods() on the assocaition
	 * role
	 */
	void writeAssociationRoleMethod(QString fieldClassName, QString roleName, QString multi, 
					QString description, Scope visib, Changeability_Type change,  
					QTextStream &java); 

	/**
	 * Writes getFoo() and setFoo() accessor methods for the attribute
	 */
	void writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
						 QString fieldName, QString description, 
						 Scope visibility, Changeability_Type change,
						 bool isFinal, QTextStream &java);

	/**
	 * Writes addFoo() and removeFoo() accessor methods for the Vector attribute
	 */
	void writeVectorAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
						 QString fieldName, QString description, 
						 Scope visibility, Changeability_Type change,
						 QTextStream &java); 

	/**
	 * Writes a // style comment
	 */
	void writeComment(QString text, QString indent, QTextStream &java); 

	/**
	 * Writes a /** documentation comment
	 */
	void writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &java); 

	/**
	 * Returns the name of the given object (if it exists)
	 */
	QString getUMLObjectName(UMLObject *obj);

	/**
	 * Raises the case of the first letter in the given string
	 */
	QString capitaliseFirstLetter(QString string); 

	/**
	 * Replaces `string' with `String' and `bool' with `boolean'
	 */
	QString fixTypeName(QString string); 

	/**
	 * check that initial values of strings have quotes around them
	 */
	QString fixInitialStringDeclValue(QString value, QString type);

	/**
	 * a little method for converting scope to string value
	 */
	QString scopeToJavaDecl(Uml::Scope scope); 
	
	/**
	 * A tab, used to indent code
	 */
	QString indent;

	/**
	 * A \n, used at the end of each line
	 */
	QString startline;

};



#endif // JAVAWRITER_H
