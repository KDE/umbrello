/***************************************************************************
			  cppwriter.cpp  -  description
			     -------------------
    copyright	    : (C) 2003 Brian Thomas brian.thomas@gsfc.nasa.gov
***************************************************************************/

/***************************************************************************
 *									 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
 *									 *
 ***************************************************************************/

#include "cppwriter.h"

#include <kdebug.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>

#include "classifierinfo.h"
#include "../umldoc.h"
#include "../class.h"
#include "../interface.h"
#include "../operation.h"
#include "../classifierlistitem.h"
#include "../umlnamespace.h"

// 3-14-2003: this code developed from the javawriter with parts of the
// original cppwriter by Luis De la Parra Blum

CppWriter::CppWriter( QObject *parent, const char *name ) : CodeGenerator(parent, name)
{

	// set some general parameters for how to generate code in this class
	INDENT = "\t";
	IndentLevel = 0;

	STRING_TYPENAME = "QString";
	STRING_TYPENAME_INCLUDE = "<qstring.h>";

//	VECTOR_TYPENAME = "vector"; // std::vector
//	VECTOR_TYPENAME_INCLUDE = "<vector.h>"; // std::vector
	VECTOR_TYPENAME = "QPtrList"; // Qt library
	VECTOR_TYPENAME_INCLUDE = "<qptrlist.h>"; // Qt library

	// Probably we could resolve this better through the use of templates,
	// but its a quick n dirty fix for the timebeing.. until codegeneration
	// template system is in place.
	// You can insert code here. 3 general variables exist: "%VARNAME%"
	// allows you to specify where the vector variable should be in your code,
	// ,"%ITEMCLASS%", if needed, where the class of the item is declared, and
	// %VECTOR_TYPENAME%, which is as definition above.
	VECTOR_METHOD_APPEND = "%VARNAME%.push_back(add_object);"; // for std::vector
	VECTOR_METHOD_REMOVE = "int i, size = %VARNAME%.size();\nfor ( i = 0; i < size; i++) {\n\t%ITEMCLASS% item = %VARNAME%.at(i);\n\tif(item == remove_object) {\n\t\tvector<%ITEMCLASS%>::iterator it = %VARNAME%.begin() + i;\n\t\t%VARNAME%.erase(it);\n\t\treturn;\n\t}\n }"; // for std::vector
	VECTOR_METHOD_INIT = ""; // nothing to be done
/*
	VECTOR_METHOD_APPEND = "%VARNAME%.append(&add_object);"; // Qt lib implementation
	VECTOR_METHOD_REMOVE = "%VARNAME%.removeRef(&remove_object);"; // Qt lib implementation
	VECTOR_METHOD_INIT = "%VARNAME%.setAutoDelete(false);"; // Qt library
*/

	OBJECT_METHOD_INIT = "%VARNAME% = new %ITEMCLASS%( );"; // Qt library

	// boolean config params
	INLINE_ATTRIBUTE_METHODS = false;
	INLINE_ASSOCIATION_METHODS = false;
	INLINE_OPERATION_METHODS = false;
	WRITE_EMPTY_CONSTRUCTOR = true;
	WRITE_EMPTY_DESTRUCTOR = true;
	WRITE_VIRTUAL_DESTRUCTORS = true;
	WRITE_ATTRIBUTE_ACCESSOR_METHODS = true;
	WRITE_PACKAGE_NAMESPACE = false;

	// should this go away?? Not currently used.. but might be by new code generator
 	/// to specifically 'tag' virtual classes and fields and methods.
	//	FieldIsVirtual = false;

}

CppWriter::~CppWriter() { }

void CppWriter::writeClass(UMLClassifier *c)
{

	if (!c) {
		kdDebug()<<"Cannot write class of NULL concept!\n";
		return;
	}

	QFile fileh, filecpp;

	// find an appropriate name for our file
	QString fileName = findFileName(c,".cpp");
	if (!fileName) {
		emit codeGenerated(c, false);
		return;
	}

	if( !openFile(fileh,fileName+".h")) {
		emit codeGenerated(c, false);
		return;
	}

	// preparations
	classifierInfo = new ClassifierInfo(c, m_doc);
	classifierInfo->fileName = fileName;
	classifierInfo->className = cleanName(c->getName());

	// write Header file
	writeHeaderFile(c, fileh);
	fileh.close();

	// Determine whether the implementation file is required.
	// (It is not required if the class is an enumeration.)
	bool need_impl = true;
	if (! classifierInfo->isInterface) {
		UMLClass* k = dynamic_cast<UMLClass*>(c);
		if (k->isEnumeration())
			need_impl = false;
	}
	if (need_impl) {
		if( !openFile(filecpp,fileName+".cpp")) {
			emit codeGenerated(c, false);
			return;
		}
		// write Source file
		writeSourceFile(c, filecpp);
		filecpp.close();
	}

	// Wrap up: free classifierInfo, emit done code
	classifierInfo = 0;

	emit codeGenerated(c, true);

}

void CppWriter::writeHeaderFile (UMLClassifier *c, QFile &fileh) {

	// open stream for writting
	QTextStream h (&fileh);

	// up the indent level to one
	IndentLevel = 1;

	// write header blurb
	QString str = getHeadingFile(".h");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),classifierInfo->fileName+".h");
		str.replace(QRegExp("%filepath%"),fileh.name());
		h<<str<<endl;
	}

	// Write the hash define stuff to prevent multiple parsing/inclusion of header
	QString hashDefine = classifierInfo->className.upper().simplifyWhiteSpace().replace(QRegExp(" "),  "_");
	writeBlankLine(h);
	h << "#ifndef "<< hashDefine + "_H" << endl;
	h << "#define "<< hashDefine + "_H" << endl;

	UMLClassifierList superclasses = classifierInfo->superclasses;
	for(UMLClassifier *classifier = superclasses.first(); classifier ;classifier = superclasses.next()) {
		QString headerName = findFileName(classifier, ".h");
		if (headerName.isEmpty()) {
			h<<"#include \""<<findFileName(classifier,".h")<<".h\""<<endl;
		}
	}

	writeBlankLine(h);
	h<<"#include "<<STRING_TYPENAME_INCLUDE<<endl;
	if(classifierInfo->hasVectorFields)
	{
		h<<"#include "<<VECTOR_TYPENAME_INCLUDE<<endl;
		writeBlankLine(h);
	}

	if(classifierInfo->hasAssociations)
	{
		// write all includes we need to include other classes, that arent us.
		printAssociationIncludeDecl (classifierInfo->plainAssociations, c->getID(), h);
		printAssociationIncludeDecl (classifierInfo->aggregations, c->getID(), h);
		printAssociationIncludeDecl (classifierInfo->compositions, c->getID(), h);

		writeBlankLine(h);
	}


	for(UMLClassifier *classifier = superclasses.first(); classifier ; classifier = superclasses.next()) {
		if(classifier->getPackage()!=c->getPackage() && !classifier->getPackage().isEmpty()) {
			h<<"using "<<cleanName(classifier->getPackage())<<"::"<<cleanName(classifier->getName())<<";"<<endl;
		}
	}

	if(!c->getPackage().isEmpty() && WRITE_PACKAGE_NAMESPACE)
		h<<endl<<"namespace "<<cleanName(c->getPackage())<<" {"<<endl<<endl;

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty()) {
		h<<endl<<"/**"<<endl;
		h<<"  * class "<<classifierInfo->className<<endl;
		h<<formatDoc(c->getDoc(),"  * ");
		h<<"  */";
		writeBlankLine(h);
		writeBlankLine(h);
	}

	//check if class is abstract and / or has abstract methods
	if((c->getAbstract() || classifierInfo->isInterface )
			&& !hasAbstractOps(c))
		h<<"/******************************* Abstract Class ****************************"<<endl
		<<classifierInfo->className<<" does not have any pure virtual methods, but its author"<<endl
		<<"  defined it as an abstract class, so you should not use it directly."<<endl
		<<"  Inherit from it instead and create only objects from the derived classes"<<endl
		<<"*****************************************************************************/"<<endl<<endl;

	if (!classifierInfo->isInterface) {
		UMLClass* k = dynamic_cast<UMLClass*>(c);
		if (k->isEnumeration()) {
			h << "enum " << classifierInfo->className << " {" << endl;
			UMLAttributeList* atl = k->getFilteredAttributeList();
			for (UMLAttribute *at=atl->first(); at ; ) {
				QString attrName = cleanName(at->getName());
				h << getIndent() << attrName;
				if ((at=atl->next()) != 0)
					h << ",";
				else
					break;
				h << endl;
			}
			h << endl << "};" << endl;	// end of class header
	                if(!c->getPackage().isEmpty() && WRITE_PACKAGE_NAMESPACE)
				h << "}  // end of package namespace" << endl;
			h << endl << "#endif // " << hashDefine + "_H" << endl;
			return;
		}
	}
	h << "class " << classifierInfo->className;
	if (classifierInfo->superclasses.count() > 0)
		h << " : ";
	uint numOfSuperClasses = classifierInfo->superclasses.count();
	uint i = 0;
	for (UMLClassifier *superClass = classifierInfo->superclasses.first();
			superClass ; superClass = classifierInfo->superclasses.next())
	{
		i++;
		if (superClass->getAbstract() || (dynamic_cast<UMLInterface*>(superClass)))
			h << "virtual ";
		h << "public " << cleanName(superClass->getName());
                if (i < numOfSuperClasses)
			h << ", ";
	}

	h<<endl<<"{"<<endl; // begin the body of the class


	//declarations of operations
        //

	//
	// write out field and operations decl grouped by visibility
	//

	// PUBLIC attribs/methods
	h<<scopeToCPPDecl(Uml::Public)<<":"<<endl<<endl; // print visibility decl.
	// for public: constructors are first ops we print out
	if(!classifierInfo->isInterface)
		writeConstructorDecls(h);
	writeHeaderFieldDecl(c,Uml::Public, h);
        writeHeaderAccessorMethodDecl(c, Uml::Public, h);
	writeOperations(c,true,Uml::Public,h);

	// PROTECTED attribs/methods
	//
	h<<scopeToCPPDecl(Uml::Protected)<<":"<<endl<<endl; // print visibility decl.
	writeHeaderFieldDecl(c,Uml::Protected, h);
        writeHeaderAccessorMethodDecl(c, Uml::Protected, h);
	writeOperations(c,true,Uml::Protected,h);

	// PRIVATE attribs/methods
	//
	h<<scopeToCPPDecl(Uml::Private)<<":"<<endl<<endl; // print visibility decl.
	writeHeaderFieldDecl(c,Uml::Private, h);
        writeHeaderAccessorMethodDecl(c, Uml::Private, h);
	writeOperations(c,true,Uml::Private,h);
        writeInitAttibuteDecl(h); // this is always private, used by constructors to initialize class

	// end of class header
	h<<endl<<"};"<<endl;

	// end of class namespace, if any
	if(!c->getPackage().isEmpty() && WRITE_PACKAGE_NAMESPACE)
		h<<"}; // end of package namespace"<<endl;

	// last thing..close our hashdefine
	h << endl << "#endif // " << hashDefine + "_H" << endl;


}

void CppWriter::writeHeaderAccessorMethodDecl(UMLClassifier *c, Scope permitScope, QTextStream &stream)
{

	// attributes
        writeHeaderAttributeAccessorMethods(permitScope, true, stream); // write static attributes first
        writeHeaderAttributeAccessorMethods(permitScope, false, stream);

	// associations
	writeAssociationMethods(classifierInfo->plainAssociations, permitScope,
                       true, INLINE_ASSOCIATION_METHODS, true, c->getID(), stream);
	writeAssociationMethods(classifierInfo->aggregations, permitScope,
                       true,  INLINE_ASSOCIATION_METHODS, true, c->getID(), stream);
	writeAssociationMethods(classifierInfo->compositions, permitScope,
                       true, INLINE_ASSOCIATION_METHODS, false, c->getID(), stream);

	writeBlankLine(stream);

}

void CppWriter::writeHeaderFieldDecl(UMLClassifier *c, Scope permitScope, QTextStream &stream)
{
	// attributes
        writeAttributeDecls(permitScope, true, stream); // write static attributes first
	writeAttributeDecls(permitScope, false, stream);

	// associations
	writeAssociationDecls(classifierInfo->plainAssociations, permitScope, c->getID(), stream);
	writeAssociationDecls(classifierInfo->aggregations, permitScope, c->getID(), stream);
	writeAssociationDecls(classifierInfo->compositions, permitScope, c->getID(), stream);

}

void CppWriter::writeSourceFile (UMLClassifier *c, QFile &filecpp ) {

	// open writing stream
	QTextStream cpp(&filecpp);

	// set the starting indentation at zero
	IndentLevel = 0;

	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".cpp");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),classifierInfo->fileName+".cpp");
		str.replace(QRegExp("%filepath%"),filecpp.name());
		cpp<<str<<endl;
	}

	// IMPORT statements
	// Q: Why all utils? Isnt just List and Vector the only classes we are using?
	// Our import *should* also look at operations, and check that objects being
	// used arent in another package (and thus need to be explicitly imported here).
	cpp<<"#include \""<<(classifierInfo->className).lower()<<".h\""<<endl;
	writeBlankLine(cpp);

	// Start body of class

	// Constructors: anything we more we need to do here ?
	//
	if(!classifierInfo->isInterface)
		writeConstructorMethods(cpp);

	// METHODS
	//

	// write comment for section IF needed
	QString indent = getIndent();
	if (forceDoc() || classifierInfo->hasAccessorMethods || classifierInfo->hasOperationMethods)
	{

		writeComment(" ", indent, cpp);
		writeComment("Methods", indent, cpp);
		writeComment(" ", indent, cpp);
		writeBlankLine(cpp);
		writeBlankLine(cpp);
	}

	// write comment for sub-section IF needed
	if (forceDoc() || classifierInfo->hasAccessorMethods )
	{
		writeComment("Accessor methods", indent, cpp);
		writeComment(" ", indent, cpp);
		writeBlankLine(cpp);
	}

	// Accessor methods for attributes
        if (!INLINE_ATTRIBUTE_METHODS && classifierInfo->hasAttributes)
	{
		writeAttributeMethods(&(classifierInfo->static_atpub), Uml::Public, false, true, !INLINE_ATTRIBUTE_METHODS, cpp);
		writeAttributeMethods(&(classifierInfo->atpub), Uml::Public, false, false, !INLINE_ATTRIBUTE_METHODS, cpp);
		writeAttributeMethods(&(classifierInfo->static_atprot), Uml::Protected, false, true, !INLINE_ATTRIBUTE_METHODS, cpp);
		writeAttributeMethods(&(classifierInfo->atprot), Uml::Protected, false, false, !INLINE_ATTRIBUTE_METHODS, cpp);
		writeAttributeMethods(&(classifierInfo->static_atpriv), Uml::Private, false, true, !INLINE_ATTRIBUTE_METHODS, cpp);
		writeAttributeMethods(&(classifierInfo->atpriv), Uml::Private, false, false, !INLINE_ATTRIBUTE_METHODS, cpp);
	}

	// accessor methods for associations

	// public
	writeAssociationMethods(classifierInfo->plainAssociations, Uml::Public, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
	writeAssociationMethods(classifierInfo->aggregations, Uml::Public, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
	writeAssociationMethods(classifierInfo->compositions, Uml::Public, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);

	// protected
	writeAssociationMethods(classifierInfo->plainAssociations, Uml::Protected, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
	writeAssociationMethods(classifierInfo->aggregations, Uml::Protected, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
	writeAssociationMethods(classifierInfo->compositions, Uml::Protected, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);


	// private
	writeAssociationMethods(classifierInfo->plainAssociations, Uml::Private, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
	writeAssociationMethods(classifierInfo->aggregations, Uml::Private, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
	writeAssociationMethods(classifierInfo->compositions, Uml::Private, false,
			!INLINE_ASSOCIATION_METHODS, true, c->getID(), cpp);
	writeBlankLine(cpp);

	// Other operation methods -- all other operations are now written
	//

	// write comment for sub-section IF needed
	if (forceDoc() || classifierInfo->hasOperationMethods)
	{
		writeComment("Other methods", indent, cpp);
		writeComment(" ", indent, cpp);
		writeBlankLine(cpp);
	}

	if(!INLINE_OPERATION_METHODS)
	{
		writeOperations(c,false,Uml::Public,cpp);
		writeOperations(c,false,Uml::Protected,cpp);
		writeOperations(c,false,Uml::Private,cpp);
	}

	// Yep, bringing up the back of the bus, our initialization method for attributes
	writeInitAttibuteMethod (cpp);

	writeBlankLine(cpp);

}

void CppWriter::writeClassDecl(UMLClassifier *c, QTextStream &cpp)
{

	QString classname = cleanName(c->getName()); // our class name

	// write documentation for class, if any, first
	if(forceDoc() || !c->getDoc().isEmpty())
	{
		if(classifierInfo->isInterface)
			writeDocumentation("Interface "+classname,c->getDoc(),"",cpp);
		else
			writeDocumentation("Class "+classname,c->getDoc(),"",cpp);

		writeBlankLine(cpp);
	}

	cpp << "class " << classname;

	// write inheritances out
	UMLClassifier *concept;
	UMLClassifierList superclasses = c->findSuperClassConcepts();

	int i = 0;
	for (concept= superclasses.first(); concept; concept = superclasses.next())
	{
		cpp<<cleanName(concept->getName())<<(i>0?", ":"");
		i++;
	}

}

void CppWriter::writeAttributeDecls (Scope visibility, bool writeStatic, QTextStream &stream )
{

	if(classifierInfo->isInterface)
		return;

	UMLAttributeList * list;
        switch (visibility)
        {
                case Uml::Private:
                        if(writeStatic)
                                list = &(classifierInfo->static_atpriv);
                        else
                                list = &(classifierInfo->atpriv);
                        break;

                case Uml::Protected:
                        if(writeStatic)
                                list = &(classifierInfo->static_atprot);
                        else
                                list = &(classifierInfo->atprot);
                        break;

                case Uml::Public:
                default:
                        if(writeStatic)
                                list = &(classifierInfo->static_atpub);
                        else
                                list = &(classifierInfo->atpub);
                        break;
        }

        //write documentation
        if(forceDoc() || list->count() > 0)
        {
                QString strVis = capitalizeFirstLetter(scopeToCPPDecl(visibility));
                QString strStatic = writeStatic ? "Static ":"";
                writeComment(strStatic+strVis+" attributes",getIndent(), stream);
                writeComment(" ",getIndent(), stream);
                writeBlankLine(stream);
        }

        if (list->count() > 0) {

	        // write attrib declarations now
	        bool isFirstAttrib = true;
	        QString documentation;
	        for(UMLAttribute *at=list->first(); at; at=list->next())
	        {

//	                bool noPriorDocExists = documentation.isEmpty();
	                documentation = at->getDoc();

	                // add a line for code clarity IF PRIOR attrib has comment on it
	                // OR this line has documentation
//	                if(!isFirstAttrib && (!documentation.isEmpty()||!noPriorDocExists))
//	                        writeBlankLine(stream);

	                isFirstAttrib = false;

	                QString varName = getAttributeVariableName(at);

	                QString staticValue = at->getStatic() ? "static " : "";
	                QString typeName = fixTypeName(at->getTypeName());
	                if(!documentation.isEmpty())
	                        writeComment(documentation, getIndent(), stream);
	                stream<<getIndent()<<staticValue<<typeName<<" "<<varName<<";"<<endl;

	        }

/*
	        if(list->count() > 0)
	                writeBlankLine(stream);
*/

	}

	return;

}

void CppWriter::writeHeaderAttributeAccessorMethods (Scope visibility, bool writeStatic, QTextStream &stream )
{

        UMLAttributeList * list;
        switch (visibility)
        {
                case Uml::Private:
                        if(writeStatic)
                                list = &(classifierInfo->static_atpriv);
                        else
                                list = &(classifierInfo->atpriv);
                        break;

                case Uml::Protected:
                        if(writeStatic)
                                list = &(classifierInfo->static_atprot);
                        else
                                list = &(classifierInfo->atprot);
                        break;

                case Uml::Public:
                default:
                        if(writeStatic)
                                list = &(classifierInfo->static_atpub);
                        else
                                list = &(classifierInfo->atpub);
                        break;
        }

	// write accessor methods for attribs we found
	writeAttributeMethods(list, visibility, true, false, INLINE_ATTRIBUTE_METHODS, stream);

}

// this is for writing *source* or *header* file attribute methods
//
void CppWriter::writeAttributeMethods(UMLAttributeList *attribs,
					Scope visibility, bool isHeaderMethod,
 					bool isStatic,
					bool writeMethodBody, QTextStream &stream)
{

	if(!WRITE_ATTRIBUTE_ACCESSOR_METHODS)
		return;

	if(forceDoc() || attribs->count()>0)
	{
		QString strVis = capitalizeFirstLetter(scopeToCPPDecl(visibility));
		QString strStatic = (isStatic ? " static" : "");
        	writeBlankLine(stream);
		writeComment(strVis+strStatic+" attribute accessor methods",getIndent(),stream);
		writeComment(" ",getIndent(), stream);
		writeBlankLine(stream);
	}

	// return now if NO attributes to work on
	if (attribs->count() == 0)
		return;

	UMLAttribute *at;
	for(at=attribs->first(); at; at=attribs->next())
	{

		QString varName = getAttributeVariableName(at);
		QString methodBaseName = cleanName(at->getName());

		// force capitalizing the field name, this is silly,
		// from what I can tell, this IS the default behavior for
		// cleanName. I dunno why its not working -b.t.
		methodBaseName.stripWhiteSpace();
		methodBaseName.replace(0,1,methodBaseName.at(0).upper());

		writeSingleAttributeAccessorMethods(at->getTypeName(), varName,
						    methodBaseName, at->getDoc(), chg_Changeable, isHeaderMethod,
						    at->getStatic(), writeMethodBody, stream);
	}

}

void CppWriter::writeComment(QString comment, QString myIndent, QTextStream &cpp)
{
	// in the case we have several line comment..
	// NOTE: this part of the method has the problem of adopting UNIX newline,
	// need to resolve for using with MAC/WinDoze eventually I assume
	if (comment.contains(QRegExp("\n"))) {

		QStringList lines = QStringList::split( "\n", comment);
		for(uint i= 0; i < lines.count(); i++)
		{
			cpp<<myIndent<<"// "<<lines[i] << endl;
		}
	} else {
		// this should be more fancy in the future, breaking it up into 80 char
		// lines so that it doesnt look too bad
		cpp<<myIndent<<"// "<< comment << endl;
	}
}

void CppWriter::writeDocumentation(QString header, QString body, QString end, QTextStream &cpp)
{
	writeBlankLine(cpp);
	QString indent = getIndent();

	cpp<<indent<<"/**"<<endl;
	if (!header.isEmpty())
		cpp<<formatDoc(header, indent+" * ");
	if (!body.isEmpty())
		cpp<<formatDoc(body, indent+" * ");
	if (!end.isEmpty())
	{
		QStringList lines = QStringList::split( "\n", end);
		for(uint i= 0; i < lines.count(); i++)
			cpp<<formatDoc(lines[i], indent+" * ");
	}
	cpp<<indent<<" */"<<endl;
}

void CppWriter::writeAssociationDecls(UMLAssociationList associations, Scope permitScope, int id, QTextStream &h)
{

	if( forceSections() || !associations.isEmpty() )
	{
		bool printRoleA = false, printRoleB = false;
		for(UMLAssociation *a = associations.first(); a; a = associations.next())
		{

			// it may seem counter intuitive, but you want to insert the role of the
			// *other* class into *this* class.
			if (a->getRoleAId() == id && a->getRoleNameB() != "")
				printRoleB = true;

			if (a->getRoleBId() == id && a->getRoleNameA() != "")
				printRoleA = true;

			// First: we insert documentaion for association IF it has either role AND some documentation (!)
			if ((printRoleA || printRoleB) && !(a->getDoc().isEmpty()))
				writeComment(a->getDoc(), getIndent(), h);

			// print RoleB decl
			if (printRoleB && a->getVisibilityB() == permitScope)
			{

				QString fieldClassName = cleanName(getUMLObjectName(a->getObjectB()));
				writeAssociationRoleDecl(fieldClassName, a->getRoleNameB(), a->getMultiB(), a->getRoleBDoc(), h);
			}

			// print RoleA decl
			if (printRoleA && a->getVisibilityA() == permitScope)
			{
				QString fieldClassName = cleanName(getUMLObjectName(a->getObjectA()));
				writeAssociationRoleDecl(fieldClassName, a->getRoleNameA(), a->getMultiA(), a->getRoleADoc(), h);
			}

			// reset for next association in our loop
			printRoleA = false;
			printRoleB = false;
		}
	}
}

void CppWriter::writeAssociationRoleDecl(QString fieldClassName, QString roleName, QString multi,
					QString doc, QTextStream &stream)
{
	// ONLY write out IF there is a rolename given
	// otherwise its not meant to be declared in the code
	if (roleName.isEmpty())
		return;

	QString indent = getIndent();

	// always put space between this and prior decl, if any
	writeBlankLine(stream);

	if (!doc.isEmpty())
		writeComment(doc, indent, stream);

	// declare the association based on whether it is this a single variable
	// or a List (Vector). One day this will be done correctly with special
	// multiplicity object that we dont have to figure out what it means via regex.
	if(multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
	{
		QString fieldVarName = "m_" + roleName.lower();

		// record this for later consideration of initialization IF the
		// multi value requires 1 of these objects
        	if(ObjectFieldVariables.findIndex(fieldVarName) == -1 &&
                     multi.contains(QRegExp("^1$"))
                  )
		{
			// ugh. UGLY. Storing variable name and its class in pairs.
                	ObjectFieldVariables.append(fieldVarName);
                	ObjectFieldVariables.append(fieldClassName);
		}

		stream<<indent<<fieldClassName<<" * "<<fieldVarName<<";"<<endl;
	}
	else
	{
		QString fieldVarName = "m_" + roleName.lower() + "Vector";

        	// record unique occurances for later when we want to check
        	// for initialization of this vector
        	if(VectorFieldVariables.findIndex(fieldVarName) == -1)
                	VectorFieldVariables.append(fieldVarName);

		stream << indent << VECTOR_TYPENAME <<"<" << fieldClassName << "*";
	       	stream << "> " << fieldVarName << ";" << endl;
	}
}

// for either source or header files
void CppWriter::writeAssociationMethods (UMLAssociationList associations,
					Scope permitVisib,
					bool isHeaderMethod,
					bool writeMethodBody,
					bool writePointerVar,
					int myID, QTextStream &stream)
{
	if( forceSections() || !associations.isEmpty() )
	{
		for(UMLAssociation *a = associations.first(); a; a = associations.next())
		{

			// insert the methods to access the role of the other
			// class in the code of this one
			if (a->getRoleAId() == myID && a->getVisibilityB() == permitVisib)
			{
				// only write out IF there is a rolename given
				if(!a->getRoleNameB().isEmpty()) {
					QString fieldClassName = getUMLObjectName(a->getObjectB()) + (writePointerVar ? " *":"");
					writeAssociationRoleMethod(fieldClassName,
									isHeaderMethod,
									writeMethodBody,
								   a->getRoleNameB(),
								   a->getMultiB(), a->getRoleBDoc(),
								   a->getChangeabilityB(), stream);
				}
			}

			if (a->getRoleBId() == myID && a->getVisibilityA() == permitVisib)
			{
				// only write out IF there is a rolename given
				if(!a->getRoleNameA().isEmpty()) {
					QString fieldClassName = getUMLObjectName(a->getObjectA()) + (writePointerVar ? " *":"");
					writeAssociationRoleMethod(fieldClassName,
									isHeaderMethod,
									writeMethodBody,
								   a->getRoleNameA(),
								   a->getMultiA(),
								   a->getRoleADoc(),
								   a->getChangeabilityA(),
								   stream);
				}
			}

		}
	}
}

void CppWriter::writeAssociationRoleMethod (QString fieldClassName,
						bool isHeaderMethod,
						bool writeMethodBody,
						QString roleName, QString multi,
					     QString description, Changeability_Type change,
					     QTextStream &stream)
{
	if(multi.isEmpty() || multi.contains(QRegExp("^[01]$")))
	{
		QString fieldVarName = "m_" + roleName.lower();
		writeSingleAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
						    description, change, isHeaderMethod, false, writeMethodBody, stream);
	}
	else
	{
		QString fieldVarName = "m_" + roleName.lower() + "Vector";
		writeVectorAttributeAccessorMethods(fieldClassName, fieldVarName, roleName,
						    description, change, isHeaderMethod, writeMethodBody, stream);
	}
}

void CppWriter::writeVectorAttributeAccessorMethods (QString fieldClassName, QString fieldVarName,
						      QString fieldName, QString description,
						      Changeability_Type changeType,
						      bool isHeaderMethod,
						      bool writeMethodBody,
						      QTextStream &stream)
{

	fieldClassName = fixTypeName(fieldClassName);
	fieldName = capitalizeFirstLetter(fieldName);
	QString indent = getIndent();

	// ONLY IF changeability is NOT Frozen
	if (changeType != chg_Frozen)
	{
		writeDocumentation("Add a "+fieldName+" object to the "+fieldVarName+" List",description,"",stream);
		stream<<indent<<"void ";
		if(!isHeaderMethod)
			stream<<classifierInfo->className<<"::";
		stream<<"add"<<fieldName<<" ( "<<fieldClassName<<" add_object )";
		if (writeMethodBody) {
			QString method = VECTOR_METHOD_APPEND;
			method.replace(QRegExp("%VARNAME%"),fieldVarName);
			method.replace(QRegExp("%VECTORTYPENAME%"), VECTOR_TYPENAME);
			method.replace(QRegExp("%ITEMCLASS%"),fieldClassName);
			stream<<indent<<" {"<<endl;
	                IndentLevel++;
			printTextAsSeparateLinesWithIndent(method,getIndent(),stream);
	                IndentLevel--;
			stream<<indent<<"}"<<endl;
		} else
			stream<<";"<<endl;
	}

	// ONLY IF changeability is Changeable
	if (changeType == chg_Changeable)
	{
		writeDocumentation("Remove a "+fieldName+" object from "+fieldVarName+" List",description,"",stream);
		stream<<indent<<"void ";
		if(!isHeaderMethod)
			stream<<classifierInfo->className<<"::";
		stream<<"remove"<<fieldName<<" ( "<<fieldClassName<<" remove_object )";
		if (writeMethodBody) {
			QString method = VECTOR_METHOD_REMOVE;
			method.replace(QRegExp("%VARNAME%"),fieldVarName);
			method.replace(QRegExp("%VECTORTYPENAME%"), VECTOR_TYPENAME);
			method.replace(QRegExp("%ITEMCLASS%"),fieldClassName);
			stream<<indent<<" {"<<endl;
	                IndentLevel++;
			printTextAsSeparateLinesWithIndent(method,getIndent(),stream);
	                IndentLevel--;
			stream<<indent<<"}"<<endl;
		} else
			stream<<";"<<endl;
	}

	// always allow getting the list of stuff
	QString returnVarName = VECTOR_TYPENAME+"<"+fieldClassName+">";
	writeDocumentation("Get the list of "+fieldName+" objects held by "+fieldVarName,description,"@return "+returnVarName+" list of "+fieldName+" objects held by "+fieldVarName,stream);
	stream<<indent<<returnVarName<<" ";
	if(!isHeaderMethod)
		stream<<classifierInfo->className<<"::";
	stream<<"get"<<fieldName<<"List ( )";
	if(writeMethodBody) {
		stream<<indent<<" {"<<endl;
		IndentLevel++;
		stream<<getIndent()<<"return "<<fieldVarName<<";"<<endl;
		IndentLevel--;
		stream<<indent<<"}"<<endl;
	} else
		stream<<";"<<endl;

}


void CppWriter::writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
						     QString fieldName, QString description,
						     Changeability_Type change,
						     bool isHeaderMethod,
						     bool isStatic,
						     bool writeMethodBody,
						     QTextStream &stream)
{

	// DONT write this IF its a source method AND writeMethodBody is "false"
	if(!isHeaderMethod && !writeMethodBody)
		return;

	fieldClassName = fixTypeName(fieldClassName);
	fieldName = capitalizeFirstLetter(fieldName);
	QString indent = getIndent();

	// set method
	if (change == chg_Changeable && !isStatic) {
		writeDocumentation("Set the value of "+fieldVarName,description,"@param new_var the new value of "+fieldVarName,stream);
		stream<<indent<<"void ";
		if(!isHeaderMethod)
			stream<<classifierInfo->className<<"::";
		stream<<"set"<<fieldName<<" ( "<<fieldClassName<<" new_var )";

		if(writeMethodBody) {
			stream<<indent<<" {"<<endl;
			IndentLevel++;
			stream<<getIndent()<<indent;
			IndentLevel--;
			if(isStatic)
				stream<<classifierInfo->className<<"::";
			stream<<fieldVarName<<" = new_var;"<<endl;
			stream<<indent<<"}"<<endl;
		} else
		       stream<<";"<<endl;
	}

	// get method
	writeDocumentation("Get the value of "+fieldVarName,description,"@return the value of "+fieldVarName,stream);
	stream<<indent<<fieldClassName<<" ";
	if(!isHeaderMethod)
		stream<<classifierInfo->className<<"::";
	stream<<"get"<<fieldName<<" ( )";

	if(writeMethodBody) {
		stream<<indent<<" {"<<endl;
		IndentLevel++;
		stream<<getIndent()<<"return ";
		IndentLevel--;
		if(isStatic)
			stream<<classifierInfo->className<<"::";
		stream<<fieldVarName<<";"<<endl;
		stream<<indent<<"}";
	} else
		       stream<<";"<<endl;

	writeBlankLine(stream);
}

// one day, this should print out non-empty constructor operations too.
void CppWriter::writeConstructorDecls(QTextStream &stream)
{

	if (forceDoc() || WRITE_EMPTY_CONSTRUCTOR || WRITE_EMPTY_DESTRUCTOR)
	{
		writeComment("Constructors/Destructors", getIndent(), stream);
		writeComment(" ", getIndent(), stream);
		writeBlankLine(stream);
	}

	if(WRITE_EMPTY_CONSTRUCTOR)
	{
		writeDocumentation("", "Empty Constructor", "", stream);
		stream<<getIndent()<<classifierInfo->className<<" ( );"<<endl;
	}

	if(WRITE_EMPTY_DESTRUCTOR)
	{
		writeDocumentation("", "Empty Destructor", "", stream);
		stream<<getIndent();
		if (WRITE_VIRTUAL_DESTRUCTORS)
			stream<<"virtual ";
		stream<<"~"<<classifierInfo->className<<" ( );"<<endl;
	}

	if(WRITE_EMPTY_DESTRUCTOR || WRITE_EMPTY_CONSTRUCTOR)
		writeBlankLine(stream);
}

void CppWriter::writeInitAttibuteDecl (QTextStream &stream)
{
	if(WRITE_EMPTY_CONSTRUCTOR && classifierInfo->hasAttributes)
		stream<<getIndent()<<"void initAttributes ( ) ;"<<endl;
}

void CppWriter::writeInitAttibuteMethod (QTextStream &stream)
{

	// only need to do this under certain conditions
	if(WRITE_EMPTY_CONSTRUCTOR && classifierInfo->hasAttributes)
	{
		QString className = classifierInfo->className;
		QString indent = getIndent();

		stream<<indent<<"void "<<className<<"::"<<"initAttributes ( ) {"<<endl;

		IndentLevel++;
		// first, initiation of fields derived from attributes
		UMLAttributeList* atl = classifierInfo->getAttList();
		for(UMLAttribute *at = atl->first(); at ; at = atl->next()) {
			if(!at->getInitialValue().isEmpty()) {
				QString varName = getAttributeVariableName(at);
				stream<<getIndent()<<varName<<" = "<<at->getInitialValue()<<";"<<endl;
			}
		}
		// Now initialize the association related fields (e.g. vectors)
		if (VECTOR_METHOD_INIT != "") {
  			QStringList::Iterator it;
	        	for( it = VectorFieldVariables.begin(); it != VectorFieldVariables.end(); ++it ) {
	                	QString fieldVarName = *it;
	                        QString method = VECTOR_METHOD_INIT;
	                        method.replace(QRegExp("%VARNAME%"),fieldVarName);
	                        method.replace(QRegExp("%VECTORTYPENAME%"), VECTOR_TYPENAME);
				stream<<getIndent()<<method<<endl;
	        	}
		}

		if (OBJECT_METHOD_INIT != "") {
  			QStringList::Iterator it;
	        	for( it = ObjectFieldVariables.begin(); it != ObjectFieldVariables.end(); ++it ) {
	                	QString fieldVarName = *it;
				it++;
	                	QString fieldClassName = *it;
	                        QString method = OBJECT_METHOD_INIT;
	                        method.replace(QRegExp("%VARNAME%"),fieldVarName);
				method.replace(QRegExp("%ITEMCLASS%"),fieldClassName);
				stream<<getIndent()<<method<<endl;
	        	}
		}

		// clean up
		ObjectFieldVariables.clear(); // shouldnt be needed?
		VectorFieldVariables.clear(); // shouldnt be needed?

		IndentLevel--;

		stream<<indent<<"}"<<endl;
	}
}

// one day, this should print out non-empty constructor operations too.
void CppWriter::writeConstructorMethods(QTextStream &stream)
{

	if (forceDoc() || WRITE_EMPTY_CONSTRUCTOR || WRITE_EMPTY_DESTRUCTOR)
	{
		writeComment("Constructors/Destructors", getIndent(), stream);
		writeComment(" ", getIndent(), stream);
		writeBlankLine(stream);
	}

	QString className = classifierInfo->className;

	// empty constructor
	if(WRITE_EMPTY_CONSTRUCTOR)
	{
		QString indent = getIndent();
		stream<<indent<<className<<"::"<<className<<" ( ) {"<<endl;
		if(classifierInfo->hasAttributes)
			stream<<indent<<INDENT<<"initAttributes();"<<endl;
		stream<<indent<<"};"<<endl;
		writeBlankLine(stream);
	}

	// empty destructor
	if(WRITE_EMPTY_DESTRUCTOR)
	{
		stream<<getIndent()<<className<<"::~"<<className<<" ( ) { };"<<endl;
		writeBlankLine(stream);
	}

}

// IF the type is "string" we need to declare it as
// the Java Object "String" (there is no string primative in Java).
QString CppWriter::fixTypeName(QString string)
{
	string.replace(QRegExp("^string$"), STRING_TYPENAME);
	return string;
}

void CppWriter::writeOperations(UMLClassifier *c, bool isHeaderMethod, Scope permitScope, QTextStream &cpp) {

	UMLOperationList *opl;
	UMLOperationList oplist;
	oplist.setAutoDelete(false);

	UMLOperation *op;
	//sort operations by scope first and see if there are abstrat methods
	opl = c->getFilteredOperationsList();
	for(op = opl->first(); op ; op=opl->next()) {
		switch(op->getScope()) {
		case Uml::Public:
			if(permitScope == Uml::Public)
				oplist.append(op);
			break;
		case Uml::Protected:
			if(permitScope == Uml::Protected)
				oplist.append(op);
			break;
		case Uml::Private:
			if(permitScope == Uml::Private)
				oplist.append(op);
			break;
		}
	}

	// do people REALLY want these comments? Hmm.
	/*
	  if(forceSections() || oppub.count())
	  {
	  writeComment("public operations",getIndent(),cpp);
		writeBlankLine(cpp);
	  }
	*/
	writeOperations(oplist,isHeaderMethod, cpp);


}

// write operation in either header or
// a source file
void CppWriter::writeOperations(UMLOperationList &oplist, bool isHeaderMethod, QTextStream &cpp) {
	UMLOperation *op;
	UMLAttribute *at;
	int i,j;
	QString str;

	QString className = classifierInfo->className;

	// generate method decl for each operation given
	for( op=oplist.first(); op ;op=oplist.next())
	{

		QString returnStr = "";
		// write documentation

		QString methodReturnType = fixTypeName(op->getReturnType());
		if(methodReturnType != "void")
			returnStr += "@return	"+methodReturnType+"\n";

		str = ""; // reset for next method
		if (op->getAbstract() || classifierInfo->isInterface) {
                	if (isHeaderMethod) {
 				// declare abstract method as 'virtual'
				str += "virtual ";
			}
                }

		// static declaration for header file
                if (isHeaderMethod)
			str += ((op->getStatic() && isHeaderMethod) ? "static ":"");

		// returntype of method
		str += methodReturnType + " ";

                if (!isHeaderMethod)
			str += className + "::";

		str += cleanName(op->getName()) + " (";

		// method parameters
		UMLAttributeList *atl = op->getParmList();
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

		// method body : only gets IF its not in a header
		if (isHeaderMethod && !INLINE_OPERATION_METHODS)
			str+=";"; // terminate now
		else
			str+=getIndent()+" {\n\n"+getIndent()+"}"; // empty method body

		// write it out
		writeDocumentation("", op->getDoc(), returnStr, cpp);
		cpp<<getIndent()<<str<<endl;
		writeBlankLine(cpp);
	}
}

// To prevent circular including when both classifiers on either end
// of an association have roles we need to have forward declaration of
// the other class...but only IF its not THIS class (as could happen
// in self-association relationship).
void CppWriter::printAssociationIncludeDecl (UMLAssociationList list, int myId, QTextStream &stream)
{

	for (UMLAssociation *a = list.first(); a; a = list.next()) {
		UMLClassifier *current = NULL;
		bool isFirstClass = true;

		// only use OTHER classes (e.g. we dont need to write includes for ourselves!!
		// AND only IF the roleName is defined, otherwise, its not meant to be noticed.
		if (a->getRoleAId() == myId && a->getRoleNameB() != "") {
			current = dynamic_cast<UMLClassifier*>(a->getObjectB());
		} else if (a->getRoleBId() == myId && a->getRoleNameA() != "") {
			current = dynamic_cast<UMLClassifier*>(a->getObjectA());
			isFirstClass = false;
		}

		// as header doc for this method indicates, we need to be a bit sophisticated on
		// how to declare some associations.
		if( current )
			if( !isFirstClass && !a->getRoleNameA().isEmpty() && !a->getRoleNameB().isEmpty())
				stream<<"class "<<current->getName()<<";"<<endl; // special case: use forward declaration
			else
				stream<<"#include \""<<current->getName().lower()<<".h\""<<endl; // just the include statement
	}
}

QString CppWriter::fixInitialStringDeclValue(QString value, QString type)
{
	// check for strings only
	if (!value.isEmpty() && type == STRING_TYPENAME) {
		if (!value.startsWith("\""))
			value.prepend("\"");
		if (!value.endsWith("\""))
			value.append("\"");
	}
	return value;
}

QString CppWriter::scopeToCPPDecl(Uml::Scope scope)
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
QString CppWriter::getUMLObjectName(UMLObject *obj)
{
	return(obj!=0)?obj->getName():QString("NULL");
}

QString CppWriter::capitalizeFirstLetter(QString string)
{
	// we could lowercase everything tostart and then capitalize? Nah, it would
	// screw up formatting like getMyRadicalVariable() to getMyradicalvariable(). Bah.
	QChar firstChar = string.at(0);
	string.replace( 0, 1, firstChar.upper());
	return string;
}

void CppWriter::writeBlankLine(QTextStream &stream)
{
	stream<<endl;
}

QString CppWriter::getIndent ()
{
	QString myIndent = "";
	for (int i = 0 ; i < IndentLevel ; i++)
		myIndent.append(INDENT);
	return myIndent;
}

void CppWriter::printTextAsSeparateLinesWithIndent (QString text, QString indent, QTextStream &stream)
{
	if(text.isEmpty())
		return;

	QStringList lines = QStringList::split( "\n", text);
	for(uint i= 0; i < lines.count(); i++)
		stream<<indent<<lines[i]<<endl;
}

QString CppWriter::getAttributeVariableName (UMLAttribute *at)
{
	QString fieldName = "m_" + cleanName(at->getName());
	return fieldName;
}
