/***************************************************************************
                          cppwriter.h  -  description
    This is the "old" code generator that does not support code editing
    in the Modeller but uses significantly less file space because the
    source code is not replicated in the XMI file.
                             -------------------
    copyright            : (C) 2003 Brian Thomas
         (C) 2004  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
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

#include "simplecodegenerator.h"
#include "../umloperationlist.h"
#include "../umlattributelist.h"
#include "../umlassociationlist.h"

class QFile;
class QTextStream;
class ClassifierInfo;

/**
  * class CppWriter is a code generator for UMLClassifier objects.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate both a header (.h) and 
  * source (.cpp) file for that classifier.
  */
class CppWriter : public SimpleCodeGenerator {
public:

	/**
	 * Constructor, initialises a couple of variables
	 */
	CppWriter(UMLDoc* parent = 0, const char* name = 0);

	/**
	 * Destructor, empty
	 */
	virtual ~CppWriter();

	/**
	 * call this method to generate cpp code for a UMLClassifier
	 * @param c the class to generate code for
	 */
	virtual void writeClass(UMLClassifier *c);

	/**
	 * checks whether type is "CppWriter"
	 *
	 * @param type
	 */
	virtual bool isType (QString & type);

	/**
	 * returns "C++"
	 */
	virtual QString getLanguage();

private:

	/**
	 * Writes class's documentation then the class header
	 * public abstract class Foo extents {
	 */
	void writeClassDecl(UMLClassifier *c, QTextStream &cpp); 

	/**
	 * Writes the comment and class constructor declaration or methods 
	 */
	void writeConstructorDecls(QTextStream &h); 
	void writeConstructorMethods(QTextStream &cpp); 

	/**
	 * write all field declarations, for both attributes and associations for the
         * given permitted scope.
         */
	void writeFieldDecl(UMLClassifier *c, Uml::Scope permitScope, QTextStream &stream); 

        /**
	 * write all method declarations, for attributes and associations 
         * for the given permitted scope.
         */
	void writeAccessorMethodDecl(UMLClassifier *c, Uml::Scope permitScope, QTextStream &stream); 

	/**
	 * write all operations for a given class
	 * @param c the class for which we are generating code
	 * @param i whether or not we are writing this to a source or header file
	 * @param j what type of method to write (by Scope)
	 * @param k the stream associated with the output file
	 */
	void writeOperations(UMLClassifier *c, bool isHeaderMethod, Uml::Scope permitScope, QTextStream &j);

	/**
	 * write a list of operations for a given class
	 * @param list the list of operations you want to write
	 * @param i whether or not we are writing this to a source or header file
	 * @param j the stream associated with the output file
	 */
	void writeOperations(UMLOperationList &list, bool isHeaderMethod, QTextStream &j);

	/**
	 * write all attributes for a given class
	 * @param c the class for which we are generating code
	 * @param j the stream associated with the output file
	 */
	void writeAttributes(UMLClassifier *c, QTextStream &j);

	/**
	 * writes the Attribute declarations
	 * @param visibility the visibility of the attribs to print out
	 * @param writeStatic whether to write static or non-static attributes out
	 * @param stream text stream
	 */
	void writeAttributeDecls (Uml::Scope visibility, bool writeStatic, QTextStream &stream ); 

	/**
	 * Write out fields and operations for this class selected on a particular
	 * visibility.
	 */
	void writeHeaderFieldDecl(UMLClassifier *c, Uml::Scope permitVisibility, QTextStream &stream); 

	void writeHeaderAttributeAccessorMethods (Uml::Scope visibility, bool writeStatic, QTextStream &stream ); 

	void writeHeaderAttributeAccessorMethodDecls(UMLClassifier *c, Uml::Scope permitVisibility, QTextStream &stream); 
        void writeHeaderAccessorMethodDecl(UMLClassifier *c, Uml::Scope permitScope, QTextStream &stream); 


	/**
	 * Searches a list of associations for appropriate ones to write out as attributes
	 */
	void writeAssociationDecls(UMLAssociationList associations, Uml::Scope permit, Uml::IDType id, QTextStream &stream);

	/**
	 * Writes out an association as an attribute using Vector
	 */
	void writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi, 
				      QString doc, QTextStream &stream); 

	/**
	 * calls @ref writeSingleAttributeAccessorMethods() on each of the attributes in attribs list.
	 */
	void writeAttributeMethods(UMLAttributeList *attribs, Uml::Scope visib, bool isHeaderMethod, 
				bool isStatic,
				bool writeMethodBody, QTextStream &stream);

	/**
	 * calls @ref writeAssociationRoleMethod() on each of the associations in the given list
	 */
	void writeAssociationMethods(UMLAssociationList associations, Uml::Scope permitVisib, 
					bool isHeaderMethod,
					bool writeMethodBody, bool writePointerVar, Uml::IDType id, QTextStream &stream); 
	
	/**
	 * calls @ref writeSingleAttributeAccessorMethods() or @ref
	 * writeVectorAttributeAccessorMethods() on the assocaition
	 * role
	 */
	void writeAssociationRoleMethod(QString fieldClassName, bool isHeaderMethod, bool writeMethodBody, QString roleName, QString multi, 
					QString description, Uml::Changeability_Type change,  
					QTextStream &stream); 

	/**
	 * Writes getFoo() and setFoo() accessor methods for the attribute
	 */
	void writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
						 QString fieldName, QString description, 
						 Uml::Changeability_Type change,
						 bool isHeaderMethod,
						 bool isStatic, bool writeMethodBody, QTextStream &cpp);

	/**
	 * Writes addFoo() and removeFoo() accessor methods for the Vector attribute
	 */
	void writeVectorAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
						 QString fieldName, QString description, 
						 Uml::Changeability_Type change,
						 bool isHeaderMethod,
						 bool writeMethodBody,
						 QTextStream &cpp); 

	/**
	 * Writes a // style comment
	 */
	void writeComment(QString text, QString indent, QTextStream &cpp); 

	/**
	 * Writes a documentation comment
	 */
	void writeDocumentation(QString header, QString body, QString end, QTextStream &cpp); 


	/**
	 * write the header file for this classifier.
	 */
	void writeHeaderFile (UMLClassifier *c, QFile &file);

	/**
	 * write the source code file for this classifier.
	 */
	void writeSourceFile (UMLClassifier *c, QFile &file);

	/**
	 * utility method to break up a block of text, which has embedded newline chars,
	 * and print them to a stream as separate lines of text, indented as directed.
	 */
	void printTextAsSeparateLinesWithIndent (QString text, QString indent, QTextStream &stream); 

	/**
	 * Intellegently print out header include/forward decl. for associated classes.
	 */
	void printAssociationIncludeDecl (UMLAssociationList list, Uml::IDType this_id, QTextStream &stream); 

	/**
	 * If needed, write out the method to initialize attributes of our class.
	 */
	void writeInitAttibuteMethod (QTextStream &stream); 

	/**
	 * If needed, write out the declaration for the method to initialize attributes of our class.
	 */
	void writeInitAttibuteDecl (QTextStream &stream); 

	/**
	 * How much indent to use (current, based on amount of indentLevel).
	 */
	QString getIndent (); 

	/**
	 * current level of indentation for code
	 */
	int IndentLevel;

	/**
	 * Returns the name of the given object (if it exists)
	 */
	QString getUMLObjectName(UMLObject *obj);

	/**
	 * Raises the case of the first letter in the given string
	 */
	QString capitalizeFirstLetter(QString string); 

	/**
	 * Replaces `string' with STRING_TYPENAME.
	 */
	QString fixTypeName(QString string); 

	/**
	 * check that initial values of strings have quotes around them
	 */
	QString fixInitialStringDeclValue(QString value, QString type);

	/**
	 * Determine what the variable name of this attribute should be.
	 */
	QString getAttributeVariableName (UMLAttribute *at); 

	/**
	 * Write a blank line
	 */
	void writeBlankLine(QTextStream &stream); 

	/**
	 * a little method for converting scope to string value
	 */
	QString scopeToCPPDecl(Uml::Scope scope); 
	
	/**
	 * Basic "unit" of indentation 
	 */
	QString INDENT;

	/**
	 * A \n, used at the end of each line
	 */
	QString startline;

	/**
	 * Summary information about current classifier.
	 */
	ClassifierInfo * classifierInfo;

	/**
	 * When we create a string variable, what do we use? this is the default..
	 */
	QString STRING_TYPENAME;
	QString STRING_TYPENAME_INCLUDE;

	/**
	 * When we create a 'vector' variable, what do we use? this is the default..
	 */
	QString VECTOR_TYPENAME;
	QString VECTOR_TYPENAME_INCLUDE;
	QString VECTOR_METHOD_APPEND;
	QString VECTOR_METHOD_REMOVE;
	QString VECTOR_METHOD_INIT;
	QString OBJECT_METHOD_INIT;

	/**
	 * Create various methods for class attributes/associations/operations as inline decl in header.
	 */
	bool INLINE_ATTRIBUTE_METHODS;
	bool INLINE_ASSOCIATION_METHODS;
	bool INLINE_OPERATION_METHODS;

	/**
	 * Constrol whether or not we want to write the package name as a namespace
	 * in our class.
	 */
	bool WRITE_PACKAGE_NAMESPACE;
	/**
	 * Constrol whether or not we want to have a constructor/destructor written
	 */
	bool WRITE_EMPTY_CONSTRUCTOR;
	bool WRITE_EMPTY_DESTRUCTOR;

	/**
	 * Control whether we want accessor methods generated for attributes.
	 */
	bool WRITE_ATTRIBUTE_ACCESSOR_METHODS;

	/**
	 * Should destuctors be declared as virtual?
	 */
	bool WRITE_VIRTUAL_DESTRUCTORS;

	QStringList ObjectFieldVariables;
	QStringList VectorFieldVariables;

};



#endif // CPPWRITER_H
