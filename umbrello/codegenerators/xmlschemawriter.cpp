/***************************************************************************
                          xmlschemawriter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 Brian Thomas brian.thomas@gsfc.nasa.gov
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "xmlschemawriter.h"

#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "../umldoc.h"
#include "../class.h"
#include "../interface.h"
#include "../operation.h"
#include "../umlnamespace.h"

// Constructor
XMLSchemaWriter::XMLSchemaWriter( QObject *parent, const char *name ) : CodeGenerator(parent, name) {

	packageNamespaceTag = "tns";
	packageNamespaceURI = "http://change.this.value/";
	schemaNamespaceTag = "xs";
	schemaNamespaceURI = "http://www.w3.org/2001/XMLSchema";
	indent = "\t";
	indentLevel = 0;
	startline = "\n" + indent; // using UNIX newLine standard.. bad

}

// form of..."the Destructor"!!
XMLSchemaWriter::~XMLSchemaWriter() {
}

// main method for invoking..
void XMLSchemaWriter::writeClass(UMLClassifier *c)
{

	if (!c) {
		kdDebug()<<"Cannot write class of NULL classifier!\n";
		return;
	}

	// find an appropiate name for our file
	QString fileName = findFileName(c,".xsd");

	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	// check that we may open that file for writing
	QFile file;
	if ( !openFile(file, fileName+".xsd") ) {
		emit codeGenerated(c, false);
		return;
	}

	QTextStream XMLschema(&file);

	// set package namespace tag appropriately
        if(!c->getPackage().isEmpty())
	       packageNamespaceTag = c->getPackage();

	// START WRITING

	// 1. create the header
	QString headerText = getHeadingFile(".xsd");
	if(!headerText.isEmpty()) {
		headerText.replace(QRegExp("%filename%"),fileName+".xsd");
		headerText.replace(QRegExp("%filepath%"),file.name());
	}
	if(!headerText.isEmpty())
		XMLschema<<headerText<<endl;

	indentLevel++;

        // 2? IMPORT statements -- do we need to do anything here? I suppose if
	// our document has more than one package, which is possible, we are missing
	// the correct import statements. Leave that for later at this time.
        /*
        //only import classes in a different package as this class
        QList<UMLClassifier> imports;
        findObjectsRelated(c,imports);
        for(UMLClassifier *con = imports.first(); con ; con = imports.next())
                if(con->getPackage() != c->getPackage())
                        XMLschema<<"import "<<con->getPackage()<<"."<<cleanName(con->getName())<<";"<<endl;
        */

	// 3. BODY of the schema.
	// start the writing by sending this classifier, the "root" for this particular
	// schema, to writeClassifier method, which will subsequently call itself on all
	// related classifiers and thus populate the schema.
	writeClassifier(c, XMLschema);

	// 4. What remains is to make the root node declaration
	XMLschema<<endl;
	writeElementDecl(getElementName(c), getElementTypeName(c), XMLschema);

	// 5. Finished: now we may close schema decl
	indentLevel--;
	XMLschema<<getIndent()<<"</"<<makeSchemaTag("schema")<<">"<<endl; // finished.. close schema node

	// bookeeping for code generation
	emit codeGenerated(c, true);

	// tidy up. no dangling open files please..
	file.close();

	// need to clear HERE, NOT in the destructor because we want each
	// schema that we write to have all related classes.
	writtenClassifiers.clear();
}

void XMLSchemaWriter::writeElementDecl( QString elementName, QString elementTypeName, QTextStream &XMLschema)
{
        if(forceDoc())
		writeComment(elementName+" is the root element, declared here.", XMLschema);

	XMLschema<<getIndent()<<"<"<<makeSchemaTag("element")
		<<" name=\""<<elementName<<"\""
		<<" type=\""<<makePackageTag(elementTypeName)<<"\""
		<<"/>"<<endl;

}

void XMLSchemaWriter::writeClassifier (UMLClassifier *c, QTextStream &XMLschema)
{

	// NO doing this 2 or more times.
	if(hasBeenWritten(c))
		return;

	XMLschema<<endl;

	// write documentation for class, if any, first
        if(forceDoc() || !c->getDoc().isEmpty())
		writeComment(c->getDoc(),XMLschema);

	if(c->getAbstract() || dynamic_cast<UMLInterface*>(c) )
		writeAbstractClassifier(c,XMLschema); // if its an interface or abstract class
	else
		writeConcreteClassifier(c,XMLschema);

}

QPtrList <UMLAttribute> XMLSchemaWriter::findAttributes (UMLClassifier *c)
{
        // sort attributes by Scope
        QPtrList <UMLAttribute> attribs;
        attribs.setAutoDelete(false);

	UMLClass * myClass = dynamic_cast<UMLClass*>(c);
	if(myClass) {
	        QPtrList <UMLAttribute> *atl = myClass->getAttList();
	        for(UMLAttribute *at=atl->first(); at ; at=atl->next()) {
	                switch(at->getScope())
	                {
	                        case Uml::Public:
	                        case Uml::Protected:
	                                attribs.append(at);
	                                break;
	                        case Uml::Private:
	                                // DO NOTHING! no way to print in the schema
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
	QPtrList <UMLClassifier> subclasses = c->findSubClassConcepts(m_doc); // list of what inherits from us
	QPtrList<UMLClassifier> superclasses = c->findSuperClassConcepts(m_doc); // list of what inherits from us

	// write the main declaration
	writeConcreteClassifier (c, XMLschema);
	writeGroupClassifierDecl (c, subclasses, XMLschema);

	markAsWritten(c);

	// now go back and make sure all sub-classing nodes are declared
	if(subclasses.count() > 0)
	{

		QString elementName = getElementName(c);
        	QPtrList <UMLAttribute> attribs = findAttributes(c);
		QStringList attribGroups = findAttributeGroups(c);

		writeAttributeGroupDecl(elementName, attribs, XMLschema);

		// now write out inheriting classes, as needed
		for(UMLClassifier * classifier = subclasses.first(); classifier; classifier = subclasses.next())
			writeClassifier(classifier, XMLschema);
	}

	// write out any superclasses as needed
	for(UMLClassifier *classifier = superclasses.first(); classifier; classifier = superclasses.next())
		writeClassifier(classifier, XMLschema);

}

void XMLSchemaWriter::writeGroupClassifierDecl (UMLClassifier *c,
						QPtrList <UMLClassifier> subclasses,
						QTextStream &XMLschema)
{

	// name of class, subclassing classifiers
	QString elementTypeName = getElementGroupTypeName(c);

	// start Writing node but only if it has subclasses? Nah..right now put in empty group
	XMLschema<<getIndent()<<"<"<<makeSchemaTag("group")<<" name=\""<<elementTypeName<<"\">"<<endl;
	indentLevel++;

	XMLschema<<getIndent()<<"<"<<makeSchemaTag("choice")<<">"<<endl;
	indentLevel++;

	for(UMLClassifier *classifier = subclasses.first(); classifier; classifier = subclasses.next()) {
		writeAssociationRoleDecl(classifier, "1", XMLschema);
	}

	indentLevel--;
	XMLschema<<getIndent()<<"</"<<makeSchemaTag("choice")<<">"<<endl;

	indentLevel--;

	// finish node
	XMLschema<<getIndent()<<"</"<<makeSchemaTag("group")<<">"<<endl;

}

void XMLSchemaWriter::writeComplexTypeClassifierDecl (UMLClassifier *c,
							QPtrList<UMLAssociation> associations,
							QPtrList<UMLAssociation> aggregations,
							QPtrList<UMLAssociation> compositions,
							QPtrList<UMLClassifier> superclasses,
							QTextStream &XMLschema)
{

	// Preparations
	//

	// sort attributes by Scope
	QPtrList <UMLAttribute> attribs = findAttributes(c);
	QStringList attribGroups = findAttributeGroups(c);

	// test for relevant associations
	bool hasAssociations = determineIfHasChildNodes(c);
	bool hasSuperclass = superclasses.count()> 0;
	bool hasAttributes = attribs.count() > 0 || attribGroups.count() > 0;

	// START WRITING

	// start body of element
	QString elementTypeName = getElementTypeName(c);

	XMLschema<<getIndent()<<"<"<<makeSchemaTag("complexType")<<" name=\""<<elementTypeName<<"\"";

	if(hasAssociations || hasAttributes || hasSuperclass)
	{

		XMLschema<<">"<<endl;

		indentLevel++;

		if(hasSuperclass)
		{
			QString superClassName = getElementTypeName(superclasses.first());
			XMLschema<<getIndent()<<"<"<<makeSchemaTag("complexContent")<<">"<<endl;

			//PROBLEM: we only treat ONE superclass for inheritence.. bah.
			indentLevel++;
			XMLschema<<getIndent()<<"<"<<makeSchemaTag("extension")<<" base=\""<<makePackageTag(superClassName)
				<<"\"";
			if(hasAssociations || hasAttributes )
				XMLschema<<">"<<endl;
			else
				XMLschema<<"/>"<<endl;

			indentLevel++;
		}

		if(hasAssociations)
		{
			// Child Elements (from most associations)
			//
			bool didFirstOne = false;
			didFirstOne = writeAssociationDecls(associations, true, didFirstOne, c->getID(), XMLschema);
			didFirstOne = writeAssociationDecls(aggregations, false, didFirstOne, c->getID(), XMLschema);
			didFirstOne = writeAssociationDecls(compositions, false, didFirstOne, c->getID(), XMLschema);

			if (didFirstOne) {
				indentLevel--;
				XMLschema<<getIndent()<<"</"<<makeSchemaTag("sequence")<<">"<<endl;
			}
		}

		// ATTRIBUTES
		//
		if(hasAttributes)
		{
			writeAttributeDecls(attribs, XMLschema);
			for(uint i= 0; i < attribGroups.count(); i++)
				XMLschema<<getIndent()<<"<"<<makeSchemaTag("attributeGroup")<<" ref=\""
					<<makePackageTag(attribGroups[i])<<"\"/>"<<endl;
		}

		if(hasSuperclass)
		{
			indentLevel--;

			if(hasAssociations || hasAttributes )
				XMLschema<<getIndent()<<"</"<<makeSchemaTag("extension")<<">"<<endl;

			indentLevel--;
			XMLschema<<getIndent()<<"</"<<makeSchemaTag("complexContent")<<">"<<endl;
		}

		// close this element decl
		indentLevel--;
		XMLschema<<getIndent()<<"</"<<makeSchemaTag("complexType")<<">"<<endl;

	} else
		XMLschema<<"/>"<<endl; // empty node. just close this element decl

}

void XMLSchemaWriter::writeConcreteClassifier (UMLClassifier *c, QTextStream &XMLschema)
{

	// preparations.. gather information about this classifier
	//
	QPtrList<UMLClassifier> superclasses = c->findSuperClassConcepts(m_doc); // list of what inherits from us
	QPtrList <UMLAssociation> aggregations = c->getAggregations();
	QPtrList <UMLAssociation> compositions = c->getCompositions();
	// BAD! only way to get "general" associations.
	QPtrList <UMLAssociation> associations = c->getSpecificAssocs(Uml::at_Association);

	// write the main declaration
	writeComplexTypeClassifierDecl(c, associations, aggregations, compositions, superclasses, XMLschema);

	markAsWritten(c);

	// Now write out any child def's
	writeChildObjsInAssociation(c, associations, XMLschema);
	writeChildObjsInAssociation(c, aggregations, XMLschema);
	writeChildObjsInAssociation(c, compositions, XMLschema);

	// write out any superclasses as needed
	for(UMLClassifier *classifier = superclasses.first(); classifier; classifier = superclasses.next())
		writeClassifier(classifier, XMLschema);
}

// these exist for abstract classes only (which become xs:group nodes)
QStringList XMLSchemaWriter::findAttributeGroups (UMLClassifier *c)
{
	// we need to look for any class we inherit from. IF these
	// have attributes, then we need to notice
	QStringList list;
	QPtrList<UMLClassifier> superclasses = c->findSuperClassConcepts(m_doc); // list of what inherits from us
	for(UMLClassifier *classifier = superclasses.first(); classifier; classifier = superclasses.next())
	{
		if(classifier->getAbstract())
		{
			// only classes have attributes..
			UMLClass * myClass = dynamic_cast<UMLClass*>(classifier);
			if(myClass) {
				QPtrList<UMLAttribute>* attribs = myClass->getAttList();
				if (attribs->count() > 0)
					list.append(getElementName(classifier)+"AttribGroupType");
			}
		}
	}
	return list;
}

bool XMLSchemaWriter::determineIfHasChildNodes( UMLClassifier *c)
{
	QPtrList<UMLObject> aggList = findChildObjsInAssociations (c, c->getAggregations());
	QPtrList<UMLObject> compList = findChildObjsInAssociations (c, c->getCompositions());
	QPtrList <UMLAssociation> associations = c->getSpecificAssocs(Uml::at_Association); // BAD! only way to get "general" associations.
	QPtrList<UMLObject> assocList = findChildObjsInAssociations (c, associations);
	return aggList.count() > 0 || compList.count() > 0 || assocList.count() > 0;
}

void XMLSchemaWriter::writeChildObjsInAssociation (UMLClassifier *c,
		QPtrList<UMLAssociation> assoc,
		QTextStream &XMLschema)
{

	QPtrList<UMLObject> list = findChildObjsInAssociations (c, assoc);
	for(UMLObject * obj = list.first(); obj; obj = list.next())
	{
		UMLClassifier * thisClassifier = dynamic_cast<UMLClassifier*>(obj);
		if(thisClassifier)
			writeClassifier(thisClassifier, XMLschema);
	}
}

bool XMLSchemaWriter::hasBeenWritten(UMLClassifier *c) {
	if (writtenClassifiers.contains(c))
	       return true;
	else
		return false;
}

void XMLSchemaWriter::markAsWritten(UMLClassifier *c) {
	writtenClassifiers.append(c);
}

void XMLSchemaWriter::writeAttributeDecls(QPtrList<UMLAttribute> &attribs, QTextStream &XMLschema )
{

	UMLAttribute *at;
	for(at=attribs.first(); at; at=attribs.next())
	{
		writeAttributeDecl(at,XMLschema);
	}

}

void XMLSchemaWriter::writeAttributeDecl(UMLAttribute *attrib, QTextStream &XMLschema )
{

	QString documentation = attrib->getDoc();
	QString typeName = fixTypeName(attrib->getTypeName());
	bool isStatic = attrib->getStatic();
	QString initialValue = fixInitialStringDeclValue(attrib->getInitialValue(), typeName);

	if(!documentation.isEmpty())
		writeComment(documentation, XMLschema);

	XMLschema<<getIndent()<<"<"<<makeSchemaTag("attribute")
       		<<" name=\""<<cleanName(attrib->getName())<<"\""
               	<<" type=\""<<typeName<<"\"";

	// default value?
	if(!initialValue.isEmpty())
	{
		// IF its static, then we use "fixed", otherwise, we use "default" decl.
		// For the default decl, we _must_ use "optional" decl
		if(isStatic)
			XMLschema<<" use=\"required\" fixed=\""<<initialValue<<"\"";
		else
			XMLschema<<" use=\"optional\" default=\""<<initialValue<<"\"";
	}

	// finish decl
	XMLschema<<"/>"<<endl;

}

void XMLSchemaWriter::writeAttributeGroupDecl (QString elementName, QPtrList<UMLAttribute> &attribs, QTextStream &XMLschema )
{

	if (attribs.count()> 0) {

		// write a little documentation
		writeComment("attributes for element "+elementName,XMLschema);

		// open attribute group
		XMLschema<<getIndent()<<"<"<<makeSchemaTag("attributeGroup")<<" name=\""<<elementName+"AttribGroupType"<<"\">"<<endl;

		indentLevel++;

		for( UMLAttribute *at=attribs.first(); at; at=attribs.next())
		{
			writeAttributeDecl(at,XMLschema);
		}

		indentLevel--;

		// close attrib group node
		XMLschema<<getIndent()<<"</"<<makeSchemaTag("attributeGroup")<<">"<<endl;
	}
}

void XMLSchemaWriter::writeComment(QString comment, QTextStream &XMLschema)
{
	// in the case we have several line comment..
	// NOTE: this part of the method has the problem of adopting UNIX newline,
	// need to resolve for using with MAC/WinDoze eventually I assume
	QString indent = getIndent();
	XMLschema<<indent<<"<!-- ";
	if (comment.contains(QRegExp("\n"))) {
		XMLschema<<endl;
		QStringList lines = QStringList::split( "\n", comment);
		for(uint i= 0; i < lines.count(); i++)
			XMLschema<<indent<<"     "<<lines[i]<<endl;

		XMLschema<<indent<<"-->"<<endl;
	} else {
		// this should be more fancy in the future, breaking it up into 80 char
		// lines so that it doesnt look too bad
		XMLschema<<comment<<" -->"<<endl;
	}
}

// all that matters here is roleA, the role served by the children of this class
// in any composition or aggregation association. In full associations, I have only
// considered the case of "self" association, so it shouldnt matter if we use role A or
// B to find the child class as long as we dont use BOTH roles. I bet this will fail
// badly for someone using a plain association between 2 different classes. THAT should
// be done, but isnt yet (this is why I have left role b code in for now). -b.t.
bool XMLSchemaWriter::writeAssociationDecls(QPtrList<UMLAssociation> associations,
		bool noRoleNameOK, bool didFirstOne, int id, QTextStream &XMLschema)
{

	if( !associations.isEmpty() )
	{
		bool printRoleA = false, printRoleB = false;

		for(UMLAssociation *a = associations.first(); a; a = associations.next())
		{
			// it may seem counter intuitive, but you want to insert the role of the
			// *other* class into *this* class.

			if (a->getRoleAId() == id && a->getVisibilityB() != Uml::Private)
				printRoleB = true;

			if (a->getRoleBId() == id && a->getVisibilityA() != Uml::Private)
				printRoleA = true;

			// First: we insert documentaion for association IF it has either role
			// AND some documentation (!)
			if ((printRoleA || printRoleB) && !(a->getDoc().isEmpty()))
				writeComment(a->getDoc(), XMLschema);

			// opening for sequence
			if(!didFirstOne && (printRoleA || printRoleB))
			{
				didFirstOne = true;
				XMLschema<<getIndent()<<"<"<<makeSchemaTag("sequence")<<">"<<endl;
				indentLevel++;
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
					// otherwise its not meant to be declared
					if (!a->getRoleNameB().isEmpty() || noRoleNameOK)
						writeAssociationRoleDecl(classifierB, a->getMultiB(), XMLschema);
				}
			}
			*/

			// print RoleA decl
			if (printRoleA)
			{
				UMLClassifier *classifierA = dynamic_cast<UMLClassifier*>(a->getObjectA());
				if (classifierA) {
					// ONLY write out IF there is a rolename given
					// otherwise its not meant to be declared
					if (!a->getRoleNameA().isEmpty() || noRoleNameOK )
						writeAssociationRoleDecl(classifierA, a->getMultiA(), XMLschema);
				}
			}
		}

	}

	return didFirstOne;
}

QPtrList<UMLObject> XMLSchemaWriter::findChildObjsInAssociations (UMLClassifier *c, QPtrList<UMLAssociation> associations)
{
	int id = c->getID();
	QPtrList<UMLObject> list;
	for(UMLAssociation *a = associations.first(); a; a = associations.next())
	{
		if (a->getRoleAId() == id
				&& a->getVisibilityB() != Uml::Private
				&& !a->getRoleNameB().isEmpty()
		   )
			list.append(a->getObjectB());

		if (a->getRoleBId() == id
				&& a->getVisibilityA() != Uml::Private
				&& !a->getRoleNameA().isEmpty()
		   )
			list.append(a->getObjectA());
	}
	return list;
}

void XMLSchemaWriter::writeAssociationRoleDecl( UMLClassifier *c, QString multi, QTextStream &XMLschema)
{

	bool isAbstract = c->getAbstract();
	bool isInterface = dynamic_cast<UMLInterface*>(c) ? true : false;

	QString elementName = getElementName(c);
	QString doc = c->getDoc();

	if (!doc.isEmpty())
		writeComment(doc, XMLschema);


	// Min/Max Occurs is based on whether it is this a single element
	// or a List (maxoccurs>1). One day this will be done correctly with special
	// multiplicity object that we dont have to figure out what it means via regex.
	QString minOccurs = "0";
	QString maxOccurs = "unbounded";
	if (multi.isEmpty())
	{
		// in this case, association will only specify ONE element can exist
		// as a child
		minOccurs = "1";
		maxOccurs = "1";
	}
	else
	{
		QStringList values = QStringList::split( QRegExp("[^\\d{1,}|\\*]"), multi);

		// could use some improvement here.. for sequences like "0..1,3..5,10" we
		// dont capture the whole "richness" of the multi. Instead we translate it
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
	// Therefore, IF a group is abstract, but has no inheriting sub-classes, there are no choices, and its nigh
	// on pointless to declare it as a group, in this special case, abstract classes get declared
	// as complexTypes.
	//
	// Of course, in OO methodology, you should be able to inherit from
	// any class, but schema just dont allow use to have full inheritence using either groups
	// or complexTypes. Thus we have taken a middle rode. If someone wants to key me into a
	// better way to represent this, I'd be happy to listen. =b.t.
	//
	// UPDATE: partial solution to the above: as of 13-Mar-2003 we now write BOTH a complexType
	//         AND a group declaration for interfaces AND classes which are inherited from.
	//
	if ((isAbstract || isInterface ) && c->findSubClassConcepts(m_doc).count() > 0)
		XMLschema<<getIndent()<<"<"<<makeSchemaTag("group")
			<<" ref=\""<<makePackageTag(getElementGroupTypeName(c))<<"\"";
	else
		XMLschema<<getIndent()<<"<"<<makeSchemaTag("element")
			<<" name=\""<<getElementName(c)<<"\""
			<<" type=\""<<makePackageTag(getElementTypeName(c))<<"\"";

	// min/max occurs
	if (minOccurs != "1")
		XMLschema<<" minOccurs=\""<<minOccurs<<"\"";

	if (maxOccurs != "1")
		XMLschema<<" maxOccurs=\""<<maxOccurs<<"\"";

	// tidy up the node
	XMLschema<<"/>"<<endl;

}

// IF the type is "string" we need to declare it as
// the XMLSchema Object "String" (there is no string primative in XMLSchema).
// Same thing again for "bool" to "boolean"
QString XMLSchemaWriter::fixTypeName(QString string)
{
//	string.replace(QRegExp("^string$"),schemaNamespaceTag+":string");
//	string.replace(QRegExp("^bool$"),schemaNamespaceTag+":boolean");
	string.prepend(schemaNamespaceTag+":");
	return string;
}

QString XMLSchemaWriter::fixInitialStringDeclValue(QString value, QString type)
{
	// check for strings only
	if (!value.isEmpty() && type == "xs:string") {
		if (!value.startsWith("\""))
			value.remove(0,1);
		if (!value.endsWith("\""))
			value.remove(value.length(),1);
	}
	return value;
}

QString XMLSchemaWriter::lowerFirstLetterCase(QString string)
{
	QChar firstChar = string.at(0);
	string.replace( 0, 1, firstChar.lower());
	return string;
}

QString XMLSchemaWriter::getIndent ()
{
	QString myIndent = "";
	for (int i = 0 ; i < indentLevel ; i++)
		myIndent.append(indent);
	return myIndent;
}

QString XMLSchemaWriter::getElementName(UMLClassifier *c)
{
	// return cleanName(lowerFirstLetterCase(c->getName()));
	return cleanName(c->getName());
}

QString XMLSchemaWriter::getElementTypeName(UMLClassifier *c)
{
	QString elementName = getElementName(c);
	return elementName + "Type";
}

QString XMLSchemaWriter::getElementGroupTypeName(UMLClassifier *c)
{
	QString elementName = getElementName(c);
	return elementName + "InterfaceType";
}

QString XMLSchemaWriter::makePackageTag (QString tagName) {
	tagName.prepend( packageNamespaceTag + ":");
	return tagName;
}

QString XMLSchemaWriter::makeSchemaTag (QString tagName) {
	tagName.prepend( schemaNamespaceTag + ":");
	return tagName;
}
