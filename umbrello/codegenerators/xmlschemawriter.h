/***************************************************************************
                          xmlschemawriter.h  -  description
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

#ifndef XMLSCHEMAWRITER_H
#define XMLSCHEMAWRITER_H

#include "../codegenerator.h"
#include "../attribute.h"
#include "../association.h"

#include <qlist.h>
#include <qstringlist.h>


class UMLOperation;

/**
  * class XMLSchemaWriter is a code generator for UMLConcept objects.
  * Create an instance of this class, and feed it a UMLConcept when
  * calling writeClass and it will generate a XMLschema source file for
  * that concept
  *
  * Our basic approach is to map UMLConcepts (classes) in to XML elements (or 
  * nodes). We declare these element in the schema either as complexType or
  * as groups based on whether they are concrete or abstract in nature.
  * This is not a perfect decision, but thats life with XML Schema... you 
  * cant fully represent Objects in the XML world ..yet. -b.t.
  */

class XMLSchemaWriter : public CodeGenerator {
public:

	/**
	 * Constructor, initialises a couple of variables
	 */
	XMLSchemaWriter(QObject* parent = 0, const char* name = 0);

	/**
	 * Destructor, empty
	 */
	virtual ~XMLSchemaWriter();

	/**
	 * call this method to generate XMLschema code for a UMLConcept
	 * @param c the class to generate code for
	 */
	virtual void writeClass(UMLConcept *c);

private:

	/**
	 * Writes concept's documentation then  guts
	 */
	void writeConcept(UMLConcept *c, QTextStream &XMLSchema); 
	void writeAbstractConcept(UMLConcept *c, QTextStream &XMLSchema); 
	void writeConcreteConcept(UMLConcept *c, QTextStream &XMLSchema); 

	bool determineIfHasChildNodes( UMLConcept *c); 

	/**
	 * write all attributes for a given class
	 * @param c the class for which we are generating code
	 * @param j the stream associated with the output file
	 */
	void writeAttributes(UMLConcept *c, QTextStream &j);

	/**
	 * write an element declaration.
	 */
	void writeElementDecl(QString elementName, QString elementTypeName, QTextStream &XMLschema); 

	/**
	 * writes the Attribute declarations
	 * @param attribs List of attributes
	 * @param XMLschema text stream
	 */
	void writeAttributeDecls(QPtrList<UMLAttribute> &attribs, QTextStream &XMLschema ); 

	/**
	 * write an element attribute.
	 */
	void writeAttributeDecl(UMLAttribute *attrib, QTextStream &XMLschema ); 

	/**
	 * Find all attributes for this concept.
	 */
	QPtrList <UMLAttribute> findAttributes (UMLConcept *c); 

	/**
	 * Discover the string name of all the attribute groups (which are child nodes)
	 * of this concept (err.. element)
	 */
	QStringList findAttributeGroups (UMLConcept *c); 

	/**
	 * Searches a list of associations for appropriate ones to write out as attributes.
	 * This works well for compositions, aggregations and self-associations but will 
	 * not work right for plain associations between 2 different classes.
	 */
	bool writeAssociationDecls(QPtrList<UMLAssociation> associations, bool noRoleOK, bool didOne, int id, QTextStream &XMLschema);

	/**
	 * Find all attributes that  belong in group
	 */
	void writeAttributeGroupDecl(QString elementName, QPtrList<UMLAttribute> &attribs, QTextStream &XMLschema ); 

	/**
	 * Writes out an association as an attribute using Vector
	 */
	void writeAssociationRoleDecl(UMLConcept *c, QString multi, QTextStream &XMLschema); 

	/**
	 * Construct an element tag with the schema namespace 
	 */
	QString makeSchemaTag (QString tagName);

	/**
	 * Construct an element tag with the package namespace 
	 */
	QString makePackageTag (QString tagName);

	/**
	 * Writes a comment
	 */
	void writeComment(QString text, QTextStream &XMLschema); 

	QPtrList<UMLObject> findChildObjsInAssociations (UMLConcept *c, QPtrList<UMLAssociation> associations);

	/**
	 * Returns the current indent string
	 */
	QString getIndent (); 
		
	/**
	 * Lowers the case of the first letter in the given string
	 */
	QString lowerFirstLetterCase(QString string); 

	/**
	 * Replaces `string' with `String' and `bool' with `boolean'
	 */
	QString fixTypeName(QString string); 

	/**
	 * check that initial values of strings DONT have quotes around them
	 * (we get double quoting then!!
	 */
	QString fixInitialStringDeclValue(QString value, QString type);

	/**
	 * Find the element node name for this concept.
	 */
	QString getElementName(UMLConcept *c);

	/**
	 * Find the element node "type" name. Used in the "complexType" which
	 * might define that element node.
	 */
	QString getElementTypeName(UMLConcept *c);

	/**
	 * Find all the child objects in this association and make sure they get 
	 * written out (if they havent already been)
	 */
	void writeChildObjsInAssociation (UMLConcept *c, QPtrList<UMLAssociation> assoc, QTextStream &s); 

	/**
	 * Quick check to see if we have written the declaration for this concept yet.
	 */
	bool hasBeenWritten(UMLConcept *c);

	/**
	 * mark a concept as written, so it is not repeatedly re-declared in the schema
	 */ 
	void markAsWritten(UMLConcept *c);

	/**
	 * Basic unit of whitespace used to indent code
	 */
	QString indent;

	/**
	 * Parameter to record the amount of indent we have.
	 */
	int indentLevel;

	/**
	 * The basic schemaNamespace tag
	 */
	QString schemaNamespaceTag;

	/**
	 * The basic schemaNamespace tag
	 */
	QString packageNamespaceTag;

	/*
	 * The basic schemaNamespace URI
	 */
	QString schemaNamespaceURI;

	/**
	 * The basic schemaNamespace URI
	 */
	QString packageNamespaceURI;

	/**
	 * A \n, used at the end of each line
	 */
	QString startline;

	/**
	 * a list of UMLConcepts we have already written
	 */
	QPtrList <UMLConcept> writtenConcepts;

};



#endif // XMLSCHEMAWRITER_H
