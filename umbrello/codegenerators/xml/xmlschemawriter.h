/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <brian.thomas@gsfc.nasa.gov>     *
 *   copyright (C) 2004-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
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

    XMLSchemaWriter();
    virtual ~XMLSchemaWriter();

    virtual void writeClass(UMLClassifier *c);

    virtual Uml::ProgrammingLanguage::Enum language() const;

    virtual QStringList reservedKeywords() const;

private:

    void writeClassifier(UMLClassifier *c, QTextStream &XMLSchema);
    void writeAbstractClassifier(UMLClassifier *c, QTextStream &XMLSchema);
    void writeConcreteClassifier(UMLClassifier *c, QTextStream &XMLSchema);

    void writeComplexTypeClassifierDecl(UMLClassifier *c,
                                        UMLAssociationList associations,
                                        UMLAssociationList aggregations,
                                        UMLAssociationList compositions,
                                        UMLClassifierList superclassifiers,
                                        QTextStream &XMLSchema);

    void writeGroupClassifierDecl(UMLClassifier *c,
                                  UMLClassifierList superclassifiers,
                                  QTextStream &XMLSchema);

    bool determineIfHasChildNodes(UMLClassifier *c);

    /**
     * write all attributes for a given class
     * @param c the class for which we are generating code
     * @param j the stream associated with the output file
     */
//    void writeAttributes(UMLClassifier *c, QTextStream &j);

    void writeElementDecl(const QString &elementName, const QString &elementTypeName, QTextStream &XMLschema);

    void writeAttributeDecls(UMLAttributeList &attribs, QTextStream &XMLschema);

    void writeAttributeDecl(UMLAttribute *attrib, QTextStream &XMLschema);

    UMLAttributeList findAttributes(UMLClassifier *c);

    QStringList findAttributeGroups(UMLClassifier *c);

    bool writeAssociationDecls(UMLAssociationList associations, bool noRoleOK, bool didOne,
                               Uml::ID::Type id, QTextStream &XMLschema);

    void writeAttributeGroupDecl(const QString &elementName, UMLAttributeList &attribs, QTextStream &XMLschema);

    void writeAssociationRoleDecl(UMLClassifier *c, const QString &multi, QTextStream &XMLschema);

    QString makeSchemaTag(QString tagName);
    QString makePackageTag(QString tagName);

    void writeComment(const QString &text, QTextStream &XMLschema);

    UMLObjectList findChildObjsInAssociations(UMLClassifier *c, UMLAssociationList associations);

    QString fixTypeName(const QString& string);

    QString fixInitialStringDeclValue(QString value, const QString &type);

    QString getElementName(UMLClassifier *c);
    QString getElementTypeName(UMLClassifier *c);
    QString getElementGroupTypeName(UMLClassifier *c);

    void writeChildObjsInAssociation(UMLClassifier *c, UMLAssociationList assoc, QTextStream &s);

    bool hasBeenWritten(UMLClassifier *c);
    void markAsWritten(UMLClassifier *c);

    QString schemaNamespaceTag;    ///< basic schemaNamespace tag
    QString packageNamespaceTag;   ///< basic packageNamespace tag
    QString schemaNamespaceURI;    ///< basic schemaNamespace URI
    QString packageNamespaceURI;   ///< basic packageNamespace URI
    QString startline;             ///< a \n, used at the end of each line
    UMLClassifierList writtenClassifiers;  ///< a list of UMLClassifiers we have already written
};

#endif // XMLSCHEMAWRITER_H
