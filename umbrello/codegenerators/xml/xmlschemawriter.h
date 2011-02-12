/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <brian.thomas@gsfc.nasa.gov>     *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef XMLSCHEMAWRITER_H
#define XMLSCHEMAWRITER_H

#include "attribute.h"
#include "association.h"
#include "codegenerator.h"
#include "umlclassifierlist.h"
#include "umlattributelist.h"
#include "umlobjectlist.h"
#include "umlassociationlist.h"
#include "simplecodegenerator.h"

/**
  * Class XMLSchemaWriter is a code generator for UMLClassifier objects.
  * Create an instance of this class, and feed it a UMLClassifier when
  * calling writeClass and it will generate a XMLschema source file for
  * that concept
  *
  * Our basic approach is to map UMLClassifiers (classes/interfaces) into
  * XML elements (or nodes). We declare these element in the schema either
  * as complexType or as groups based on whether they are concrete or abstract
  * in nature. This is not a perfect decision, but thats life with XML Schema...
  * you cant fully represent Objects in the XML world ..yet. -b.t.
  */

class XMLSchemaWriter : public SimpleCodeGenerator
{
    Q_OBJECT
public:

    /**
     * Constructor, initialises a couple of variables
     */
    XMLSchemaWriter();

    /**
     * Destructor, empty
     */
    virtual ~XMLSchemaWriter();

    /**
     * call this method to generate XMLschema code for a UMLClassifier
     * @param c the class to generate code for
     */
    virtual void writeClass(UMLClassifier *c);

    /**
     * returns "XMLSchema"
     */
    virtual Uml::ProgrammingLanguage language() const;

    /**
     * get list of reserved keywords
     */
    virtual QStringList reservedKeywords() const;

private:

    /**
     * Writes concept's documentation then  guts
     */
    void writeClassifier(UMLClassifier *c, QTextStream &XMLSchema);
    void writeAbstractClassifier(UMLClassifier *c, QTextStream &XMLSchema);
    void writeConcreteClassifier(UMLClassifier *c, QTextStream &XMLSchema);

    /**
     * write a <complexType> declaration for this classifier
     */
    void writeComplexTypeClassifierDecl(UMLClassifier *c,
                                        UMLAssociationList associations,
                                        UMLAssociationList aggregations,
                                        UMLAssociationList compositions,
                                        UMLClassifierList superclassifiers,
                                        QTextStream &XMLSchema);

    /**
     * write a <group> declaration for this classifier. Used for interfaces to classes with
     * inheriting children.
     */
    void writeGroupClassifierDecl(UMLClassifier *c,
                                  UMLClassifierList superclassifiers,
                                  QTextStream &XMLSchema);

    /**
     * find if the classifier would have any Child elements.
     */
    bool determineIfHasChildNodes( UMLClassifier *c);

    /**
     * write all attributes for a given class
     * @param c the class for which we are generating code
     * @param j the stream associated with the output file
     */
    void writeAttributes(UMLClassifier *c, QTextStream &j);

    /**
     * write an element declaration.
     */
    void writeElementDecl( const QString &elementName, const QString &elementTypeName, QTextStream &XMLschema);

    /**
     * writes the Attribute declarations
     * @param attribs List of attributes
     * @param XMLschema text stream
     */
    void writeAttributeDecls(UMLAttributeList &attribs, QTextStream &XMLschema );

    /**
     * write an element attribute.
     */
    void writeAttributeDecl(UMLAttribute *attrib, QTextStream &XMLschema );

    /**
     * Find all attributes for this concept.
     */
    UMLAttributeList findAttributes (UMLClassifier *c);

    /**
     * Discover the string name of all the attribute groups (which are child nodes)
     * of this concept (err.. element)
     */
    QStringList findAttributeGroups (UMLClassifier *c);

    /**
     * Searches a list of associations for appropriate ones to write out as attributes.
     * This works well for compositions, aggregations and self-associations but will
     * not work right for plain associations between 2 different classes.
     */
    bool writeAssociationDecls(UMLAssociationList associations, bool noRoleOK, bool didOne,
                               Uml::IDType id, QTextStream &XMLschema);

    /**
     * Find all attributes that  belong in group
     */
    void writeAttributeGroupDecl(const QString &elementName, UMLAttributeList &attribs, QTextStream &XMLschema );

    /**
     * Writes out an association as an attribute using Vector
     */
    void writeAssociationRoleDecl(UMLClassifier *c, const QString &multi, QTextStream &XMLschema);

    /**
     * Construct an element tag with the schema namespace
     */
    QString makeSchemaTag ( QString tagName );

    /**
     * Construct an element tag with the package namespace
     */
    QString makePackageTag ( QString tagName );

    /**
     * Writes a comment
     */
    void writeComment(const QString &text, QTextStream &XMLschema);

    /**
     * Find and return a list of child UMLObjects pointed to by the associations
     * in this UMLClassifier.
     */
    UMLObjectList findChildObjsInAssociations (UMLClassifier *c, UMLAssociationList associations);

    /**
     * Replaces `string' with `String' and `bool' with `boolean'
     */
    QString fixTypeName(const QString& string);

    /**
     * check that initial values of strings DON'T have quotes around them
     * (we get double quoting then!!
     */
    QString fixInitialStringDeclValue( QString value, const QString &type);

    /**
     * Find the element node name for this concept.
     */
    QString getElementName(UMLClassifier *c);

    /**
     * Find the element node "type" name. Used in the "complexType" which
     * might define that element node.
     */
    QString getElementTypeName(UMLClassifier *c);

    /**
     * Find the group node "type" name. Used for elements which define an interface/are abstract.
     */
    QString getElementGroupTypeName(UMLClassifier *c);

    /**
     * Find all the child objects in this association and make sure they get
     * written out (if they havent already been)
     */
    void writeChildObjsInAssociation (UMLClassifier *c, UMLAssociationList assoc, QTextStream &s);

    /**
     * Quick check to see if we have written the declaration for this concept yet.
     */
    bool hasBeenWritten(UMLClassifier *c);

    /**
     * mark a concept as written, so it is not repeatedly re-declared in the schema
     */
    void markAsWritten(UMLClassifier *c);

    /**
     * The basic schemaNamespace tag
     */
    QString schemaNamespaceTag;

    /**
     * The basic schemaNamespace tag
     */
    QString packageNamespaceTag;

    /**
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
     * a list of UMLClassifiers we have already written
     */
    UMLClassifierList writtenClassifiers;
};

#endif // XMLSCHEMAWRITER_H
