/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003 Brian Thomas <brian.thomas@gsfc.nasa.gov>          *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "xmlschemawriter.h"

#include "classifier.h"
#include "debug_utils.h"
#include "operation.h"
#include "umldoc.h"

#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QRegExp>

// Constructor
XMLSchemaWriter::XMLSchemaWriter()
{
    packageNamespaceTag = "tns";
    packageNamespaceURI = "http://foo.example.com/";
    schemaNamespaceTag = "xs";
    schemaNamespaceURI = "http://www.w3.org/2001/XMLSchema";
}

// form of..."the Destructor"!!
XMLSchemaWriter::~XMLSchemaWriter()
{
}

/**
 * returns "XMLSchema"
 */
Uml::ProgrammingLanguage XMLSchemaWriter::language() const
{
    return Uml::ProgrammingLanguage::XMLSchema;
}

// main method for invoking..
void XMLSchemaWriter::writeClass(UMLClassifier *c)
{
    if (!c) {
        uDebug()<<"Cannot write class of NULL classifier!";
        return;
    }

    // find an appropriate name for our file
    QString fileName = findFileName(c,".xsd");

    if (fileName.isEmpty()) {
        emit codeGenerated(c, false);
        return;
    }

    // check that we may open that file for writing
    QFile file;
    if ( !openFile(file, fileName) ) {
        emit codeGenerated(c, false);
        return;
    }

    QTextStream XMLschema(&file);

    // set package namespace tag appropriately
    if(!c->package().isEmpty())
        packageNamespaceTag = c->package();

    // START WRITING

    // 0. FIRST THING: open the xml processing instruction. This MUST be
    // the first thing in the file
    XMLschema<<"<?xml version=\"1.0\"?>"<<m_endl;

    // 1. create the header
    QString headerText = getHeadingFile(".xsd");
    if(!headerText.isEmpty()) {
        headerText.replace(QRegExp("%filename%"),fileName);
        headerText.replace(QRegExp("%filepath%"),file.fileName());
    }
    if(!headerText.isEmpty())
        XMLschema<<headerText<<m_endl;

    // 2. Open schema element node with appropriate namespace decl
    XMLschema<<"<"<<makeSchemaTag("schema");
    // common namespaces we know will be in the file..
    XMLschema<<" targetNamespace=\""<<packageNamespaceURI+packageNamespaceTag<<"\""<<m_endl;
    XMLschema<<" xmlns:"<<schemaNamespaceTag<<"=\""<<schemaNamespaceURI<<"\"";
    XMLschema<<" xmlns:"<<packageNamespaceTag<<"=\""<<packageNamespaceURI+packageNamespaceTag<<"\"";

    XMLschema<<">"<<m_endl; // close opening declaration

    m_indentLevel++;

    // 3? IMPORT statements -- do we need to do anything here? I suppose if
    // our document has more than one package, which is possible, we are missing
    // the correct import statements. Leave that for later at this time.
    /*
    //only import classes in a different package as this class
    UMLPackageList imports;
    findObjectsRelated(c,imports);
    for(UMLPackage *con = imports.first(); con ; con = imports.next())
        if(con->getPackage() != c->getPackage())
                XMLschema<<"import "<<con->getPackage()<<"."<<cleanName(con->getName())<<";"<<m_endl;
    */

    // 4. BODY of the schema.
    // start the writing by sending this classifier, the "root" for this particular
    // schema, to writeClassifier method, which will subsequently call itself on all
    // related classifiers and thus populate the schema.
    writeClassifier(c, XMLschema);

    // 5. What remains is to make the root node declaration
    XMLschema<<m_endl;
    writeElementDecl(getElementName(c), getElementTypeName(c), XMLschema);

    // 6. Finished: now we may close schema decl
    m_indentLevel--;
    XMLschema<<indent()<<"</"<<makeSchemaTag("schema")<<">"<<m_endl; // finished.. close schema node

    // bookeeping for code generation
    emit codeGenerated(c, true);

    // tidy up. no dangling open files please..
    file.close();

    // need to clear HERE, NOT in the destructor because we want each
    // schema that we write to have all related classes.
    writtenClassifiers.clear();
}

void XMLSchemaWriter::writeElementDecl( const QString &elementName, const QString &elementTypeName, QTextStream &XMLschema)
{
    if(forceDoc())
        writeComment(elementName+" is the root element, declared here.", XMLschema);

    XMLschema<<indent()<<"<"<<makeSchemaTag("element")
    <<" name=\""<<elementName<<"\""
    <<" type=\""<<makePackageTag(elementTypeName)<<"\""
    <<"/>"<<m_endl;
}

void XMLSchemaWriter::writeClassifier (UMLClassifier *c, QTextStream &XMLschema)
{
    // NO doing this 2 or more times.
    if(hasBeenWritten(c))
        return;

    XMLschema<<m_endl;

    // write documentation for class, if any, first
    if(forceDoc() || !c->doc().isEmpty())
        writeComment(c->doc(),XMLschema);

    if(c->isAbstract() || c->isInterface() )
        writeAbstractClassifier(c,XMLschema); // if it is an interface or abstract class
    else
        writeConcreteClassifier(c,XMLschema);
}

UMLAttributeList XMLSchemaWriter::findAttributes (UMLClassifier *c)
{
    // sort attributes by Scope
    UMLAttributeList attribs;

    if (!c->isInterface()) {
        UMLAttributeList atl = c->getAttributeList();
        foreach(UMLAttribute *at ,  atl ) {
            switch(at->visibility())
            {
              case Uml::Visibility::Public:
              case Uml::Visibility::Protected:
                attribs.append(at);
                break;
              case Uml::Visibility::Private:
                // DO NOTHING! no way to print in the schema
                break;
              default:
                break;
            }
        }
    }
    return attribs;
}

// We have to do 2 things with abstract classifiers (e.g. abstract classes and interfaces)
// which is to:
// 1) declare it as a complexType so it may be inherited (I can see an option here: to NOT write
//    this complexType declaration IF the classifier itself isnt inherited by or is inheriting
//    from anything because no other element will use this complexType).
// 2) Create a group so that elements, which obey the abstract class /interface may be placed in
//    aggregation with other elements (again, and option here to NOT write the group if no other
//    element use the interface in element aggregation)
//
void XMLSchemaWriter::writeAbstractClassifier (UMLClassifier *c, QTextStream &XMLschema)
{
    // preparations
    UMLClassifierList subclasses = c->findSubClassConcepts(); // list of what inherits from us
    UMLClassifierList superclasses = c->findSuperClassConcepts(); // list of what we inherit from

    // write the main declaration
    writeConcreteClassifier (c, XMLschema);
    writeGroupClassifierDecl (c, subclasses, XMLschema);

    markAsWritten(c);

    // now go back and make sure all sub-classing nodes are declared
    if(subclasses.count() > 0)
    {
        QString elementName = getElementName(c);
        UMLAttributeList attribs = findAttributes(c);
        QStringList attribGroups = findAttributeGroups(c);

        writeAttributeGroupDecl(elementName, attribs, XMLschema);

        // now write out inheriting classes, as needed
        foreach (UMLClassifier * classifier , subclasses )
            writeClassifier(classifier, XMLschema);
    }

    // write out any superclasses as needed
    foreach (UMLClassifier *classifier , superclasses )
        writeClassifier(classifier, XMLschema);
}

void XMLSchemaWriter::writeGroupClassifierDecl (UMLClassifier *c,
        UMLClassifierList subclasses,
        QTextStream &XMLschema)
{
    // name of class, subclassing classifiers
    QString elementTypeName = getElementGroupTypeName(c);

    // start Writing node but only if it has subclasses? Nah..right now put in empty group
    XMLschema<<indent()<<"<"<<makeSchemaTag("group")<<" name=\""<<elementTypeName<<"\">"<<m_endl;
    m_indentLevel++;

    XMLschema<<indent()<<"<"<<makeSchemaTag("choice")<<">"<<m_endl;
    m_indentLevel++;

    foreach(UMLClassifier *classifier , subclasses ) {
        writeAssociationRoleDecl(classifier, "1", XMLschema);
    }

    m_indentLevel--;
    XMLschema<<indent()<<"</"<<makeSchemaTag("choice")<<">"<<m_endl;

    m_indentLevel--;

    // finish node
    XMLschema<<indent()<<"</"<<makeSchemaTag("group")<<">"<<m_endl;
}

void XMLSchemaWriter::writeComplexTypeClassifierDecl (UMLClassifier *c,
        UMLAssociationList associations,
        UMLAssociationList aggregations,
        UMLAssociationList compositions,
        UMLClassifierList superclasses,
        QTextStream &XMLschema)
{
    // Preparations
    //

    // sort attributes by Scope
    UMLAttributeList attribs = findAttributes(c);
    QStringList attribGroups = findAttributeGroups(c);

    // test for relevant associations
    bool hasAssociations = determineIfHasChildNodes(c);
    bool hasSuperclass = superclasses.count()> 0;
    bool hasAttributes = attribs.count() > 0 || attribGroups.count() > 0;

    // START WRITING

    // start body of element
    QString elementTypeName = getElementTypeName(c);

    XMLschema<<indent()<<"<"<<makeSchemaTag("complexType")<<" name=\""<<elementTypeName<<"\"";

    if(hasAssociations || hasAttributes || hasSuperclass)
    {
        XMLschema<<">"<<m_endl;

        m_indentLevel++;

        if(hasSuperclass)
        {
            QString superClassName = getElementTypeName(superclasses.first());
            XMLschema<<indent()<<"<"<<makeSchemaTag("complexContent")<<">"<<m_endl;

            //PROBLEM: we only treat ONE superclass for inheritence.. bah.
            m_indentLevel++;
            XMLschema<<indent()<<"<"<<makeSchemaTag("extension")<<" base=\""<<makePackageTag(superClassName)
            <<"\"";
            if(hasAssociations || hasAttributes )
                XMLschema<<">"<<m_endl;
            else
                XMLschema<<"/>"<<m_endl;

            m_indentLevel++;
        }

        if(hasAssociations)
        {
            // Child Elements (from most associations)
            //
            bool didFirstOne = false;
            didFirstOne = writeAssociationDecls(associations, true, didFirstOne, c->id(), XMLschema);
            didFirstOne = writeAssociationDecls(aggregations, false, didFirstOne, c->id(), XMLschema);
            didFirstOne = writeAssociationDecls(compositions, false, didFirstOne, c->id(), XMLschema);

            if (didFirstOne) {
                m_indentLevel--;
                XMLschema<<indent()<<"</"<<makeSchemaTag("sequence")<<">"<<m_endl;
            }
        }

        // ATTRIBUTES
        //
        if(hasAttributes)
        {
            writeAttributeDecls(attribs, XMLschema);
            for (int i= 0; i < attribGroups.count(); ++i) {
                XMLschema<<indent()<<"<"<<makeSchemaTag("attributeGroup")<<" ref=\""
                <<makePackageTag(attribGroups[i])<<"\"/>"<<m_endl;
            }
        }

        if(hasSuperclass)
        {
            m_indentLevel--;

            if(hasAssociations || hasAttributes )
                XMLschema<<indent()<<"</"<<makeSchemaTag("extension")<<">"<<m_endl;

            m_indentLevel--;
            XMLschema<<indent()<<"</"<<makeSchemaTag("complexContent")<<">"<<m_endl;
        }

        // close this element decl
        m_indentLevel--;
        XMLschema<<indent()<<"</"<<makeSchemaTag("complexType")<<">"<<m_endl;

    } else
        XMLschema<<"/>"<<m_endl; // empty node. just close this element decl
}

void XMLSchemaWriter::writeConcreteClassifier (UMLClassifier *c, QTextStream &XMLschema)
{
    // preparations.. gather information about this classifier
    //
    UMLClassifierList superclasses = c->findSuperClassConcepts(); // list of what inherits from us
    UMLClassifierList subclasses = c->findSubClassConcepts(); // list of what we inherit from
    UMLAssociationList aggregations = c->getAggregations();
    UMLAssociationList compositions = c->getCompositions();
    // BAD! only way to get "general" associations.
    UMLAssociationList associations = c->getSpecificAssocs(Uml::AssociationType::Association);

    // write the main declaration
    writeComplexTypeClassifierDecl(c, associations, aggregations, compositions, superclasses, XMLschema);

    markAsWritten(c);

    // Now write out any child def's
    writeChildObjsInAssociation(c, associations, XMLschema);
    writeChildObjsInAssociation(c, aggregations, XMLschema);
    writeChildObjsInAssociation(c, compositions, XMLschema);

    // write out any superclasses as needed
    foreach(UMLClassifier *classifier , superclasses)
        writeClassifier(classifier, XMLschema);

    // write out any subclasses as needed
    foreach(UMLClassifier *classifier , subclasses )
        writeClassifier(classifier, XMLschema);
}

// these exist for abstract classes only (which become xs:group nodes)
QStringList XMLSchemaWriter::findAttributeGroups (UMLClassifier *c)
{
    // we need to look for any class we inherit from. IF these
    // have attributes, then we need to notice
    QStringList list;
    UMLClassifierList superclasses = c->findSuperClassConcepts(); // list of what inherits from us
    foreach (UMLClassifier *classifier , superclasses )
    {
        if(classifier->isAbstract())
        {
            // only classes have attributes..
            if (!classifier->isInterface()) {
                UMLAttributeList attribs = c->getAttributeList();
                if (attribs.count() > 0)
                    list.append(getElementName(classifier)+"AttribGroupType");
            }
        }
    }
    return list;
}

bool XMLSchemaWriter::determineIfHasChildNodes( UMLClassifier *c)
{
    UMLObjectList aggList = findChildObjsInAssociations (c, c->getAggregations());
    UMLObjectList compList = findChildObjsInAssociations (c, c->getCompositions());
    UMLAssociationList associations = c->getSpecificAssocs(Uml::AssociationType::Association); // BAD! only way to get "general" associations.
    UMLObjectList assocList = findChildObjsInAssociations (c, associations);
    return aggList.count() > 0 || compList.count() > 0 || assocList.count() > 0;
}

void XMLSchemaWriter::writeChildObjsInAssociation (UMLClassifier *c,
        UMLAssociationList assoc,
        QTextStream &XMLschema)
{
    UMLObjectList list = findChildObjsInAssociations (c, assoc);
    foreach(UMLObject* obj, list ) {
        UMLClassifier * thisClassifier = dynamic_cast<UMLClassifier*>(obj);
        if(thisClassifier)
            writeClassifier(thisClassifier, XMLschema);
    }
}

bool XMLSchemaWriter::hasBeenWritten(UMLClassifier *c)
{
    if (writtenClassifiers.contains(c))
        return true;
    else
        return false;
}

void XMLSchemaWriter::markAsWritten(UMLClassifier *c)
{
    writtenClassifiers.append(c);
}

void XMLSchemaWriter::writeAttributeDecls(UMLAttributeList &attribs, QTextStream &XMLschema )
{
    foreach ( UMLAttribute* at , attribs ) {
        writeAttributeDecl(at,XMLschema);
    }

}

void XMLSchemaWriter::writeAttributeDecl(UMLAttribute *attrib, QTextStream &XMLschema )
{
    QString documentation = attrib->doc();
    QString typeName = fixTypeName(attrib->getTypeName());
    bool isStatic = attrib->isStatic();
    QString initialValue = fixInitialStringDeclValue(attrib->getInitialValue(), typeName);

    if(!documentation.isEmpty())
        writeComment(documentation, XMLschema);

    XMLschema<<indent()<<"<"<<makeSchemaTag("attribute")
    <<" name=\""<<cleanName(attrib->name())<<"\""
    <<" type=\""<<typeName<<"\"";

    // default value?
    if(!initialValue.isEmpty())
    {
        // IF it is static, then we use "fixed", otherwise, we use "default" decl.
        // For the default decl, we _must_ use "optional" decl
        if(isStatic)
            XMLschema<<" use=\"required\" fixed=\""<<initialValue<<"\"";
        else
            XMLschema<<" use=\"optional\" default=\""<<initialValue<<"\"";
    }

    // finish decl
    XMLschema<<"/>"<<m_endl;
}

void XMLSchemaWriter::writeAttributeGroupDecl (const QString &elementName, UMLAttributeList &attribs, QTextStream &XMLschema )
{
    if (attribs.count()> 0) {

        // write a little documentation
        writeComment("attributes for element "+elementName,XMLschema);

        // open attribute group
        XMLschema<<indent()<<"<"<<makeSchemaTag("attributeGroup")<<" name=\""<<elementName+"AttribGroupType"<<"\">"<<m_endl;

        m_indentLevel++;

        foreach( UMLAttribute *at , attribs )
        {
            writeAttributeDecl(at,XMLschema);
        }

        m_indentLevel--;

        // close attrib group node
        XMLschema<<indent()<<"</"<<makeSchemaTag("attributeGroup")<<">"<<m_endl;
    }
}

void XMLSchemaWriter::writeComment( const QString &comment, QTextStream &XMLschema )
{
    // in the case we have several line comment..
    // NOTE: this part of the method has the problem of adopting UNIX newline,
    // need to resolve for using with MAC/WinDoze eventually I assume
    QString indnt = indent();
    XMLschema<<indnt<<"<!-- ";
    if (comment.contains(QRegExp("\n"))) {
        XMLschema<<m_endl;
        QStringList lines = comment.split( '\n' );
        for (int i= 0; i < lines.count(); i++)
            XMLschema<<indnt<<"     "<<lines[i]<<m_endl;

        XMLschema<<indnt<<"-->"<<m_endl;
    } else {
        // this should be more fancy in the future, breaking it up into 80 char
        // lines so that it doesn't look too bad
        XMLschema<<comment<<" -->"<<m_endl;
    }
}

// all that matters here is roleA, the role served by the children of this class
// in any composition or aggregation association. In full associations, I have only
// considered the case of "self" association, so it shouldn't matter if we use role A or
// B to find the child class as long as we don't use BOTH roles. I bet this will fail
// badly for someone using a plain association between 2 different classes. THAT should
// be done, but isnt yet (this is why I have left role b code in for now). -b.t.
bool XMLSchemaWriter::writeAssociationDecls(UMLAssociationList associations,
        bool noRoleNameOK, bool didFirstOne, Uml::IDType id, QTextStream &XMLschema)
{
    if( !associations.isEmpty() )
    {
        bool printRoleA = false, printRoleB = false;

        foreach (UMLAssociation *a , associations )
        {
            // it may seem counter intuitive, but you want to insert the role of the
            // *other* class into *this* class.

            if (a->getObjectId(Uml::A) == id && a->getVisibility(Uml::B) != Uml::Visibility::Private)
                printRoleB = true;

            if (a->getObjectId(Uml::B) == id && a->getVisibility(Uml::A) != Uml::Visibility::Private)
                printRoleA = true;

            // First: we insert documentaion for association IF it has either role
            // AND some documentation (!)
            if ((printRoleA || printRoleB) && !(a->doc().isEmpty()))
                writeComment(a->doc(), XMLschema);

            // opening for sequence
            if(!didFirstOne && (printRoleA || printRoleB))
            {
                didFirstOne = true;
                XMLschema<<indent()<<"<"<<makeSchemaTag("sequence")<<">"<<m_endl;
                m_indentLevel++;
            }

            // print RoleB decl
            /*
            // As mentioned in the header comment for this method: this block of code is
            // commented out for now as it will only be needed if/when plain associations
            // between different classes are to be treated
            if (printRoleB)
            {
                UMLClassifier *classifierB = dynamic_cast<UMLClassifier*>(a->getObjectB());
                if (classifierB) {
                        // ONLY write out IF there is a rolename given
                        // otherwise it is not meant to be declared
                        if (!a->getRoleNameB().isEmpty() || noRoleNameOK)
                                writeAssociationRoleDecl(classifierB, a->getMultiB(), XMLschema);
                }
            }
            */

            // print RoleA decl
            if (printRoleA)
            {
                UMLClassifier *classifierA = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
                if (classifierA) {
                    // ONLY write out IF there is a rolename given
                    // otherwise it is not meant to be declared
                    if (!a->getRoleName(Uml::A).isEmpty() || noRoleNameOK )
                        writeAssociationRoleDecl(classifierA, a->getMulti(Uml::A), XMLschema);
                }
            }
        }

    }

    return didFirstOne;
}

UMLObjectList XMLSchemaWriter::findChildObjsInAssociations (UMLClassifier *c, UMLAssociationList associations)
{
    Uml::IDType id = c->id();
    UMLObjectList list;
    foreach (UMLAssociation *a , associations )
    {
        if (a->getObjectId(Uml::A) == id
                && a->getVisibility(Uml::B) != Uml::Visibility::Private
                && !a->getRoleName(Uml::B).isEmpty()
           )
            list.append(a->getObject(Uml::B));

        if (a->getObjectId(Uml::B) == id
                && a->getVisibility(Uml::A) != Uml::Visibility::Private
                && !a->getRoleName(Uml::A).isEmpty()
           )
            list.append(a->getObject(Uml::A));
    }
    return list;
}

void XMLSchemaWriter::writeAssociationRoleDecl( UMLClassifier *c, const QString &multi, QTextStream &XMLschema)
{
    bool isAbstract = c->isAbstract();
    bool isInterface = c->isInterface();

    QString elementName = getElementName(c);
    QString doc = c->doc();

    if (!doc.isEmpty())
        writeComment(doc, XMLschema);

    // Min/Max Occurs is based on whether it is this a single element
    // or a List (maxoccurs>1). One day this will be done correctly with special
    // multiplicity object that we don't have to figure out what it means via regex.
    QString minOccurs = QChar('0');
    QString maxOccurs = "unbounded";
    if (multi.isEmpty())
    {
        // in this case, association will only specify ONE element can exist
        // as a child
        minOccurs = '1';
        maxOccurs = '1';
    }
    else
    {
        QStringList values = multi.split( QRegExp("[^\\d{1,}|\\*]") );

        // could use some improvement here.. for sequences like "0..1,3..5,10" we
        // don't capture the whole "richness" of the multi. Instead we translate it
        // now to minOccurs="0" maxOccurs="10"
        if (values.count() > 0)
        {
            // populate both with the actual value as long as our value isnt an asterix
            // In that case, use special value (from above)
            if(values[0].contains(QRegExp("\\d{1,}")))
                minOccurs = values[0]; // use first number in sequence

            if(values[values.count()-1].contains(QRegExp("\\d{1,}")))
                maxOccurs = values[values.count()-1]; // use only last number in sequence
        }
    }

    // Now declare the class in the association as an element or group.
    //
    // In a semi-arbitrary way, we have decided to make abstract classes into
    // "groups" and concrete classes into "complexTypes".
    //
    // This is because about the only thing you can do with an abstract
    // class (err. node) is inherit from it with a "concrete" element. Therefore,
    // in this manner, we need a group node for abstract classes to lay out the child
    // element choices so that the child, concrete class may be plugged into whatever spot
    // it parent could go. The tradeoff is that "group" nodes may not be extended, so the
    // choices that you lay out here are it (e.g. no more nodes may inherit" from this group)
    //
    // The flipside for concrete classes is that we want to use them as elements in our document.
    // Unfortunately, about all you can do with complexTypes in terms of inheritance, is to
    // use these as the basis for a new node type. This is NOT full inheritence because the new
    // class (err. element node) wont be able to go into the document where it parent went without
    // you heavily editing the schema.
    //
    // Therefore, IF a group is abstract, but has no inheriting sub-classes, there are no choices, and it is nigh
    // on pointless to declare it as a group, in this special case, abstract classes get declared
    // as complexTypes.
    //
    // Of course, in OO methodology, you should be able to inherit from
    // any class, but schema just don't allow use to have full inheritence using either groups
    // or complexTypes. Thus we have taken a middle rode. If someone wants to key me into a
    // better way to represent this, I'd be happy to listen. =b.t.
    //
    // UPDATE: partial solution to the above: as of 13-Mar-2003 we now write BOTH a complexType
    //         AND a group declaration for interfaces AND classes which are inherited from.
    //
    if ((isAbstract || isInterface ) && c->findSubClassConcepts().count() > 0)
        XMLschema<<indent()<<"<"<<makeSchemaTag("group")
        <<" ref=\""<<makePackageTag(getElementGroupTypeName(c))<<"\"";
    else
        XMLschema<<indent()<<"<"<<makeSchemaTag("element")
        <<" name=\""<<getElementName(c)<<"\""
        <<" type=\""<<makePackageTag(getElementTypeName(c))<<"\"";

    // min/max occurs
    if (minOccurs != "1")
        XMLschema<<" minOccurs=\""<<minOccurs<<"\"";

    if (maxOccurs != "1")
        XMLschema<<" maxOccurs=\""<<maxOccurs<<"\"";

    // tidy up the node
    XMLschema<<"/>"<<m_endl;
}

// IF the type is "string" we need to declare it as
// the XMLSchema Object "String" (there is no string primative in XMLSchema).
// Same thing again for "bool" to "boolean"
QString XMLSchemaWriter::fixTypeName(const QString& string)
{
    //  string.replace(QRegExp("^string$"),schemaNamespaceTag+":string");
    //  string.replace(QRegExp("^bool$"),schemaNamespaceTag+":boolean");
    return schemaNamespaceTag + ':' + string;
}

QString XMLSchemaWriter::fixInitialStringDeclValue(QString value, const QString &type)
{
    // check for strings only
    if (!value.isEmpty() && type == "xs:string") {
        if (!value.startsWith('"'))
            value.remove(0,1);
        if (!value.endsWith('"'))
            value.remove(value.length(),1);
    }
    return value;
}

QString XMLSchemaWriter::getElementName(UMLClassifier *c)
{
    return cleanName(c->name());
}

QString XMLSchemaWriter::getElementTypeName(UMLClassifier *c)
{
    QString elementName = getElementName(c);
    return elementName + "ComplexType";
}

QString XMLSchemaWriter::getElementGroupTypeName(UMLClassifier *c)
{
    QString elementName = getElementName(c);
    return elementName + "GroupType";
}

QString XMLSchemaWriter::makePackageTag (QString tagName)
{
    tagName.prepend( packageNamespaceTag + ':');
    return tagName;
}

QString XMLSchemaWriter::makeSchemaTag (QString tagName)
{
    tagName.prepend( schemaNamespaceTag + ':');
    return tagName;
}

QStringList XMLSchemaWriter::reservedKeywords() const
{
    static QStringList keywords;

    if (keywords.isEmpty()) {
        keywords << "ATTLIST"
        << "CDATA"
        << "DOCTYPE"
        << "ELEMENT"
        << "ENTITIES"
        << "ENTITY"
        << "ID"
        << "IDREF"
        << "IDREFS"
        << "NMTOKEN"
        << "NMTOKENS"
        << "NOTATION"
        << "PUBLIC"
        << "SHORTREF"
        << "SYSTEM"
        << "USEMAP";
    }

    return keywords;
}

#include "xmlschemawriter.moc"
