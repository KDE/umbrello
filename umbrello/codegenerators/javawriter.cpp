/***************************************************************************
                          javawriter.cpp  -  description
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

#include "javawriter.h"

#include <iostream.h>
#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "../umldoc.h"
#include "../concept.h"
#include "../operation.h"
#include "../umlnamespace.h"

JavaWriter::JavaWriter( QObject *parent, const char *name ) : CodeGenerator(parent, name) {
	indent = "\t";
	startline = "\n"+indent; // using UNIX newLine standard.. bad
}

JavaWriter::~JavaWriter() {}

void JavaWriter::writeClass(UMLConcept *c)
{

	if (!c) {
		kdDebug()<<"Cannot write class of NULL concept!\n";
		return;
	}

	QString fileName = cleanName(c->getName().lower());

	//find an appropiate name for our file
	fileName = findFileName(c,".java");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	// check that we may open that file for writing
	QFile file;
	if ( !openFile(file,fileName+".java") ) {
		emit codeGenerated(c, false);
		return;
	}

	// Preparations
	//

	// sort attributes by Scope
        UMLAttribute *at;
        QList <UMLAttribute> *atl;
        QList <UMLAttribute>  atpub, atprot, atpriv;
        QList <UMLAttribute>  final_atpub, final_atprot, final_atpriv;
        atpub.setAutoDelete(false);
        final_atpub.setAutoDelete(false);
        atprot.setAutoDelete(false);
        final_atprot.setAutoDelete(false);
        atpriv.setAutoDelete(false);
        final_atpriv.setAutoDelete(false);

        atl = c->getAttList();
        for(at=atl->first(); at ; at=atl->next()) {
                switch(at->getScope())
                {
		case Uml::Public:
			if(at->getStatic())
				final_atpub.append(at);
			else
				atpub.append(at);
			break;
		case Uml::Protected:
			if(at->getStatic())
				final_atprot.append(at);
			else
				atprot.append(at);
			break;
		case Uml::Private:
			if(at->getStatic())
				final_atprot.append(at);
			else
				atpriv.append(at);
			break;
                }
        }

	// another preparation, determine what we have
	QPtrList <UMLAssociation> associations = c->getSpecificAssocs(Uml::at_Association); // BAD! only way to get "general" associations.
	QPtrList <UMLAssociation> aggregations = c->getAggregations();
	QPtrList <UMLAssociation> compositions = c->getCompositions();

	bool hasAssociations = aggregations.count() > 0 || associations.count() > 0 || compositions.count() > 0;
	bool hasAttributes = atpub.count() > 0 || atprot.count() > 0 || atpriv.count() > 0
			     || final_atpub.count() > 0
			     || final_atprot.count() > 0
			     || final_atpriv.count() > 0;
	bool hasAccessorMethods = hasAttributes || hasAssociations;
	bool hasOperationMethods = c->getOpList()->last() ? true : false;
	// this is a bit too simplistic..some associations are for
	// SINGLE objects, and WONT be declared as Vectors, so this
	// is a bit overly inclusive
	bool hasVectorFields = hasAssociations ? true : false;

	// open text stream to file
	QTextStream java(&file);

	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".java");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),fileName);
		str.replace(QRegExp("%filepath%"),file.name());
		java<<str<<endl;
	}

	if(!c->getPackage().isEmpty())
		java<<"package "<<c->getPackage()<<";"<<endl;

	// IMPORT statements
	// Q: Why all utils? Isnt just List and Vector the only classes we are using?
	// Our import *should* also look at operations, and check that objects being
	// used arent in another package (and thus need to be explicitly imported here).
	if (hasVectorFields )
	{
		java<<endl<<"import java.util.List;"<<endl;
		java<<"import java.util.Vector;"<<endl;
	}

	//only import classes in a different package as this class
	QList<UMLConcept> imports;
	findObjectsRelated(c,imports);
	for(UMLConcept *con = imports.first(); con ; con = imports.next())
		if(con->getPackage() != c->getPackage())
			java<<"import "<<con->getPackage()<<"."<<cleanName(con->getName())<<";"<<endl;
	java<<endl;

	// write the opening declaration for the class incl any documentation,
	// interfaces and/or inheritence issues we have
	writeClassDecl(c, java);

	// start body of class
	java<<endl<<"{"<<endl;

	// ATTRIBUTES
	//

	// write comment for section IF needed
	if (forceDoc() || hasAccessorMethods)
	{
		writeComment(" ", indent, java);
		writeComment("Fields", indent, java);
		writeComment(" ", indent, java);
		java<<endl;
	}

	writeAttributeDecls(final_atpub, final_atprot, final_atpriv, java);
	writeAttributeDecls(atpub, atprot, atpriv, java);

	writeAssociationDecls(associations, c->getID(), java);
	writeAssociationDecls(aggregations, c->getID(), java);
	writeAssociationDecls(compositions, c->getID(), java);

	// Constructors: anything we more we need to do here ?
	//
	writeConstructor(c, java);

	// METHODS
	//

	// write comment for section IF needed
	if (forceDoc() || hasAccessorMethods || hasOperationMethods)
	{

		java<<startline;
		writeComment(" ", indent, java);
		writeComment("Methods", indent, java);
		writeComment(" ", indent, java);
		java<<endl;
	}

	// write comment for sub-section IF needed
	if (forceDoc() || hasAccessorMethods )
	{
		writeComment("Accessor methods", indent, java);
		writeComment(" ", indent, java);
		java<<endl;
	}

	// Accessors for attributes
	writeAttributeMethods(final_atpub, Public, java);
	writeAttributeMethods(final_atprot, Protected, java);
	writeAttributeMethods(final_atpriv, Private, java);
	writeAttributeMethods(atpub, Public, java);
	writeAttributeMethods(atprot, Protected, java);
	writeAttributeMethods(atpriv, Private, java);

	// accessor methods for associations

	// first: determine the name of the other class
	writeAssociationMethods(associations, c, java);
	writeAssociationMethods(aggregations, c, java);
	writeAssociationMethods(compositions, c, java);

	// Other operation methods
	// all other operations are now written

	// write comment for sub-section IF needed
	if (forceDoc() || hasOperationMethods)
	{
		writeComment("Other methods", indent, java);
		writeComment(" ", indent, java);
		java<<endl;
	}
	writeOperations(c,java);

	java<<endl<<"}"<<endl; // end class

	file.close();
	emit codeGenerated(c, true);
}

void JavaWriter::writeClassDecl(UMLConcept *c, QTextStream &java)
{

	QPtrList<UMLAssociation> generalizations = c->getGeneralizations(); // list of what we inherit from
	QString classname = cleanName(c->getName()); // our class name

	// write documentation for class, if any, first
        if(forceDoc() || !c->getDoc().isEmpty())
	{
		writeDocumentation("Class "+classname,c->getDoc(),"","",java);
		java<<endl;
	}

	// Now write the actual class declaration
	QString scope = ""; // = scopeToJavaDecl(c->getScope());
	if (c->getScope() != Uml::Public) {
		// We should emit a warning in here .. java doesnt like to allow
		// private/protected classes. The best we can do (I believe)
		// is to let these declarations default to "package visibility"
		// which is a level between traditional "private" and "protected"
		// scopes. To get this visibility level we just print nothing..
	} else
		scope = "public ";

	java<<(c->getAbstract()?QString("abstract "):QString(""))
	    <<scope<<"class "<<classname<<(generalizations.count()>0?" extends ":"");


	UMLAssociation *a;
	int i;
	for (a = generalizations.first(), i = generalizations.count();
	     a && i; a = generalizations.next(), i--)
	{
		UMLObject* obj = m_doc->findUMLObject(a->getRoleBId());
		java<<cleanName(obj->getName())<<(i>1?", ":"");
	}

}

void JavaWriter::writeAttributeDecls(QList<UMLAttribute> &atpub, QList<UMLAttribute> &atprot,
				     QList<UMLAttribute> &atpriv, QTextStream &java )
{
	UMLAttribute *at;

	for(at=atpub.first(); at; at=atpub.next())
	{
		QString documentation = at->getDoc();
		QString staticValue = at->getStatic() ? "final " : "";
		QString typeName = fixTypeName(at->getTypeName());
		QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
		if(!documentation.isEmpty())
			writeComment(documentation, indent, java);
		java<<startline<<staticValue<<"public "<<typeName<<" "<<cleanName(at->getName())
		    <<(initialValue.isEmpty()?QString(""):QString(" = ") + initialValue)<<";";
	}

	for(at=atprot.first();at;at=atprot.next())
	{
		QString documentation = at->getDoc();
		QString typeName = fixTypeName(at->getTypeName());
		QString staticValue = at->getStatic() ? "final " : "";
		QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
		if(!documentation.isEmpty())
			writeComment(documentation, indent, java);
		java<<startline<<staticValue<<"protected "<<typeName<<" "<<cleanName(at->getName())
		    <<(initialValue.isEmpty()?QString(""):QString(" = ") + initialValue)<<";";
	}

	for(at=atpriv.first();at;at=atpriv.next())
	{
		QString documentation = at->getDoc();
		QString typeName = fixTypeName(at->getTypeName());
		QString staticValue = at->getStatic() ? "final " : "";
		QString initialValue = fixInitialStringDeclValue(at->getInitialValue(), typeName);
		if(!documentation.isEmpty())
			writeComment(documentation, indent, java);
		java<<startline<<staticValue<<"private "<<typeName<<" "<<cleanName(at->getName())
		    <<(initialValue.isEmpty()?QString(""):QString(" = ") + initialValue)<<";";
	}

}

void JavaWriter::writeAttributeMethods(QList <UMLAttribute> &atpub, Scope visibility, QTextStream &java)
{

	UMLAttribute *at;
	for(at=atpub.first(); at; at=atpub.next())
	{
		QString fieldName = cleanName(at->getName());
		// force capitalizing the field name, this is silly,
		// from what I can tell, this IS the default behavior for
		// cleanName. I dunno why its not working -b.t.
		fieldName.stripWhiteSpace();
		fieldName.replace(0,1,fieldName.at(0).upper());

		writeSingleAttributeAccessorMethods(at->getTypeName(),
						    cleanName(at->getName()),
						    fieldName,
						    at->getDoc(),
						    visibility, chg_Changeable, at->getStatic(), java);
	}

}

void JavaWriter::writeComment(QString comment, QString myIndent, QTextStream &java)
{
	// in the case we have several line comment..
	// NOTE: this part of the method has the problem of adopting UNIX newline,
	// need to resolve for using with MAC/WinDoze eventually I assume
	if (comment.contains(QRegExp("\n"))) {

		QStringList lines = QStringList::split( "\n", comment);
		for(uint i= 0; i < lines.count(); i++)
		{
			java<<endl<<myIndent<<"// "<<lines[i];
		}
	} else
		// this should be more fancy in the future, breaking it up into 80 char
		// lines so that it doesnt look too bad
		java<<endl<<myIndent<<"// "<<comment;
}

void JavaWriter::writeDocumentation(QString header, QString body, QString end, QString indent, QTextStream &java)
{
	java<<endl<<indent<<"/**"<<endl;
	if (!header.isEmpty())
		java<<formatDoc(header, indent+" * ");
	if (!body.isEmpty())
		java<<formatDoc(body, indent+" * ");
	if (!end.isEmpty())
	{
		QStringList lines = QStringList::split( "\n", end);
		for(uint i= 0; i < lines.count(); i++)
			java<<formatDoc(lines[i], indent+" * ");
	}
	java<<indent<<" */";
}

void JavaWriter::writeAssociationDecls(QPtrList<UMLAssociation> associations, int id, QTextStream &java)
{

	if( forceSections() || !associations.isEmpty() )
	{
		for(UMLAssociation *a = associations.first(); a; a = associations.next())
		{

			// insert the role of the other class in the code of this one
			if (a->getRoleAId() == id)
			{
				QString fieldClassName = cleanName(getUMLObjectName(a->getObjectB()));
				writeAssociationRoleDecl(fieldClassName, a->getRoleNameB(), a->getMultiB(), a->getVisibilityB(), java);
			}

			if (a->getRoleBId() == id)
			{
				QString fieldClassName = cleanName(getUMLObjectName(a->getObjectA()));
				writeAssociationRoleDecl(fieldClassName, a->getRoleNameA(), a->getMultiA(), a->getVisibilityA(), java);
			}
		}
	}
}

void JavaWriter::writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi, Scope visib, QTextStream &java)
{
	// ONLY write out IF there is a rolename given
	// otherwise its not meant to be declared in the code
	if (roleName.isEmpty())
		return;

	QString scope = scopeToJavaDecl(visib);

	// declare the association based on whether it is this a single variable
	// or a List (Vector). One day this will be done correctly with special
	// multiplicity object that we dont have to figure out what it means via regex.
	if(multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
	{
		QString fieldVarName = "m_" + roleName.lower();
		java<<startline<<scope<<" "<<fieldClassName<<" "<<fieldVarName<<";";
	}
	else
	{
		QString fieldVarName = roleName.lower() + "Vector";
		java<<startline<<scope<<" Vector "<<fieldVarName<<" = new Vector();";
		// from here we could initialize default values, or put in an init() section
		// of the constructors
	}
}

void JavaWriter::writeAssociationMethods (QPtrList<UMLAssociation> associations, UMLConcept *thisClass, QTextStream &java)
{
	if( forceSections() || !associations.isEmpty() )
	{
		for(UMLAssociation *a = associations.first(); a; a = associations.next())
		{

			// insert the methods to access the role of the other
			// class in the code of this one
			if (a->getRoleAId() == thisClass->getID())
			{
				// only write out IF there is a rolename given
				if(!a->getRoleNameB().isEmpty()) {
					QString fieldClassName = getUMLObjectName(a->getObjectB());
					writeAssociationRoleMethod(fieldClassName,
								   a->getRoleNameB(),
								   a->getMultiB(), a->getDoc(),
								   a->getVisibilityB(),
								   a->getChangeabilityB(), java);
				}
			}

			if (a->getRoleBId() == thisClass->getID())
			{
				// only write out IF there is a rolename given
				if(!a->getRoleNameA().isEmpty()) {
					QString fieldClassName = getUMLObjectName(a->getObjectA());
					writeAssociationRoleMethod(fieldClassName, a->getRoleNameA(),
								   a->getMultiA(),
								   a->getDoc(),
								   a->getVisibilityA(),
								   a->getChangeabilityA(),
								   java);
				}
			}

		}
	}
}

void JavaWriter::writeAssociationRoleMethod (QString fieldClassName, QString roleName, QString multi,
					     QString description, Scope visib, Changeability_Type change,
					     QTextStream &java)
{
	if(multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
	{
		QString fieldVarName = "m_" + roleName.lower();
		writeSingleAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
						    description, visib, change, false, java);
	}
	else
	{
		QString fieldVarName = roleName.lower() + "Vector";
		writeVectorAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
						    description, visib, change, java);
	}
}

void JavaWriter::writeVectorAttributeAccessorMethods (QString fieldClassName, QString fieldVarName,
						      QString fieldName, QString description,
						      Scope visibility, Changeability_Type changeType,
						      QTextStream &java)
{

	fieldClassName = fixTypeName(fieldClassName);
	fieldName = capitalizeFirstLetter(fieldName);
	QString strVis = scopeToJavaDecl(visibility);

	// ONLY IF changeability is NOT Frozen
	if (changeType != chg_Frozen)
	{
		writeDocumentation("Add a "+fieldName+" object to the "+fieldVarName+" List",description,"",indent,java);
		java<<startline<<strVis<<" void add"<<fieldName<<" ( "<<fieldClassName<<" new_object )";
		java<<startline<<"{";
		java<<startline<<indent<<fieldVarName<<".add(new_object);";
		java<<startline<<"}"<<endl;
	}

	// ONLY IF changeability is Changeable
	if (changeType == chg_Changeable)
	{
		writeDocumentation("Remove a "+fieldName+" object from "+fieldVarName+" List",description,"",indent,java);
		java<<startline<<strVis<<" void remove"<<fieldName<<" ( "<<fieldClassName<<" new_object )";
		java<<startline<<"{";
		java<<startline<<indent<<fieldVarName<<".remove(new_object);";
		java<<startline<<"}"<<endl;
	}

	// always allow getting the list of stuff
	writeDocumentation("Get the List of "+fieldName+" objects held by "+fieldVarName,description,"@return List of "+fieldName+" objects held by "+fieldVarName,indent,java);
	java<<startline<<strVis<<" List get"<<fieldName<<"List ( )";
	java<<startline<<"{";
	java<<startline<<indent<<"return (List) "<<fieldVarName<<";";
	java<<startline<<"}"<<endl;

	java<<endl;
}


void JavaWriter::writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
						     QString fieldName, QString description,
						     Scope visibility, Changeability_Type change,
						     bool isFinal, QTextStream &java)
{

	QString strVis = scopeToJavaDecl(visibility);
	fieldClassName = fixTypeName(fieldClassName);
	fieldName = capitalizeFirstLetter(fieldName);

	// set method
	if (change == chg_Changeable && !isFinal) {
		writeDocumentation("Set the value of "+fieldVarName,description,"@param new_var the new value of "+fieldVarName,indent,java);
		java<<startline<<strVis<<" void set"<<fieldName<<" ( "<<fieldClassName<<" new_var )";
		java<<startline<<"{";
		java<<startline<<indent<<fieldVarName<<" = new_var;";
		java<<startline<<"}"<<endl;
	}

	// get method
	writeDocumentation("Get the value of "+fieldVarName,description,"@return the value of "+fieldVarName,indent,java);
	java<<startline<<strVis<<" "<<fieldClassName<<" get"<<fieldName<<" ( )";
	java<<startline<<"{";
	java<<startline<<indent<<"return "<<fieldVarName<<";";
	java<<startline<<"}";

	java<<endl;
}

void JavaWriter::writeConstructor(UMLConcept *c, QTextStream &java)
{

	if (forceDoc())
	{
		java<<startline;
		writeComment(" ", indent, java);
		writeComment("Constructors", indent, java);
		writeComment(" ", indent, java);
		java<<endl;
	}

	// write the first constructor
	QString className = cleanName(c->getName());
	java<<indent<<className<<" () { };";

}

// IF the type is "string" we need to declare it as
// the Java Object "String" (there is no string primative in Java).
// Same thing again for "bool" to "boolean"
QString JavaWriter::fixTypeName(QString string)
{
	string.replace(QRegExp("^string$"),"String");
	string.replace(QRegExp("^bool$"),"boolean");
	return string;
}

void JavaWriter::writeOperations(UMLConcept *c, QTextStream &java) {
	QList<UMLOperation> *opl;
	QList <UMLOperation> oppub,opprot,oppriv;
	oppub.setAutoDelete(false);
	opprot.setAutoDelete(false);
	oppriv.setAutoDelete(false);

	UMLOperation *op;
	//sort operations by scope first and see if there are abstrat methods
	opl = c->getOpList();
	for(op = opl->first(); op ; op=opl->next()) {
		switch(op->getScope()) {
		case Uml::Public:
			oppub.append(op);
			break;
		case Uml::Protected:
			opprot.append(op);
			break;
		case Uml::Private:
			oppriv.append(op);
			break;
		}
	}

	// do people REALLY want these comments? Hmm.
	/*
	  if(forceSections() || oppub.count())
	  {
	  writeComment("public operations",indent,java);
	  java<<endl;
	  }
	*/
	writeOperations(oppub,java);

	/*
	  if(forceSections() || opprot.count())
	  {
	  writeComment("protected operations",indent,java);
	  java<<endl;
	  }
	*/
	writeOperations(opprot,java);

	/*
	  if(forceSections() || oppriv.count())
	  {
	  writeComment("private operations",indent,java);
	  java<<endl;
	  }
	*/
	writeOperations(oppriv,java);

}

void JavaWriter::writeOperations(QList<UMLOperation> &oplist, QTextStream &java) {
	UMLOperation *op;
	QList<UMLAttribute> *atl;
	UMLAttribute *at;
	int i,j;
	QString str;

	// generate method decl for each operation given
	for( op=oplist.first(); op ;op=oplist.next())
	{

		QString returnStr = "";
		// write documentation

		QString methodReturnType = fixTypeName(op->getReturnType());
		if(methodReturnType != "void")
			returnStr += "@return	"+methodReturnType+"\n";

		str = ""; // reset for next method
		str += (op->getAbstract() ? "abstract ":"");
		str += (op->getStatic() ? "final":"");
		str += scopeToJavaDecl(op->getScope()) + " ";
		str += methodReturnType + " " +cleanName(op->getName()) + "( ";

		atl = op->getParmList();
		i= atl->count();
		j=0;
		for( at = atl->first(); at ;at = atl->next(),j++) {
			QString typeName = fixTypeName(at->getTypeName());
			QString atName = cleanName(at->getName());
			str += typeName + " " + atName +
			       (!(at->getInitialValue().isEmpty()) ?
			        (QString(" = ")+at->getInitialValue()) :
			        QString(""))
			       + ((j < i-1)?", ":"");
			returnStr += "@param	"+atName+" "+at->getDoc()+"\n";
		}
		str+= " )";

		// method only gets a body IF its not abstract
		if (op->getAbstract())
			str+=";\n\n"; // terminate now
		else
			str+=startline+"{\n\n"+indent+"}\n\n"; // empty method body

		// write it out
		writeDocumentation("", op->getDoc(), returnStr, indent, java);
		java<<startline<<str;
	}
}

QString JavaWriter::fixInitialStringDeclValue(QString value, QString type)
{
	// check for strings only
	if (!value.isEmpty() && type == "String") {
		if (!value.startsWith("\""))
			value.prepend("\"");
		if (!value.endsWith("\""))
			value.append("\"");
	}
	return value;
}

QString JavaWriter::scopeToJavaDecl(Uml::Scope scope)
{
	QString scopeString;
	switch(scope)
	{
		case Uml::Public:
			scopeString = "public";
			break;
		case Uml::Protected:
			scopeString = "protected";
			break;
		case Uml::Private:
		default:
			scopeString = "private";
			break;
	}
	return scopeString;
}

// methods like this _shouldnt_ be needed IF we properly did things thruought the code.
QString JavaWriter::getUMLObjectName(UMLObject *obj)
{
	return(obj!=0)?obj->getName():"NULL";
}

QString JavaWriter::capitaliseFirstLetter(QString string)
{
	QChar firstChar = string.at(0);
	string.replace( 0, 1, firstChar.upper());
	return string;
}

