/***************************************************************************
			  cppwriter.cpp  -  description
    This is the "old" code generator that does not support code editing
    in the Modeller but uses significantly less file space because the
    source code is not replicated in the XMI file.
                             -------------------
    copyright	    : (C) 2003 Brian Thomas brian.thomas@gsfc.nasa.gov
         (C) 2004  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
***************************************************************************/

/***************************************************************************
 *									 *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.				   *
 *									 *
 ***************************************************************************/

// own header
#include "cppwriter.h"
// qt/kde includes
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <kdebug.h>
// app includes
#include "classifierinfo.h"
#include "codegen_utils.h"
#include "../umldoc.h"
#include "../classifier.h"
#include "../class.h"
#include "../interface.h"
#include "../operation.h"
#include "../umlclassifierlistitemlist.h"
#include "../classifierlistitem.h"
#include "../model_utils.h"

// 3-14-2003: this code developed from the javawriter with parts of the
// original cppwriter by Luis De la Parra Blum

CppWriter::CppWriter( UMLDoc *parent, const char *name )
  : SimpleCodeGenerator(parent, name)
{

	// set some general parameters for how to generate code in this class
	STRING_TYPENAME = "string";
	STRING_TYPENAME_INCLUDE = "<string>";

	VECTOR_TYPENAME = "vector"; // std::vector
	VECTOR_TYPENAME_INCLUDE = "<vector>"; // std::vector

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

QString CppWriter::getLanguage() {
	return "C++";
}

bool CppWriter::isType (QString & type)
{
	if (type == "CppWriter")
		return true;
	return false;
}

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

	// open stream for writing
	QTextStream h (&fileh);

	// up the indent level to one
	m_indentLevel = 1;

	// write header blurb
	QString str = getHeadingFile(".h");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),classifierInfo->fileName+".h");
		str.replace(QRegExp("%filepath%"),fileh.name());
		h<<str<< m_endl;
	}

	// Write the hash define stuff to prevent multiple parsing/inclusion of header
	QString hashDefine = classifierInfo->className.upper().simplifyWhiteSpace().replace(QRegExp(" "),  "_");
	writeBlankLine(h);
	h << "#ifndef "<< hashDefine + "_H" << m_endl;
	h << "#define "<< hashDefine + "_H" << m_endl;

	UMLClassifierList superclasses = classifierInfo->superclasses;
	for(UMLClassifier *classifier = superclasses.first(); classifier ;classifier = superclasses.next()) {
		QString headerName = findFileName(classifier, ".h");
		if (headerName.isEmpty()) {
			h<<"#include \""<<findFileName(classifier,".h")<<".h\""<<m_endl;
		}
	}

	writeBlankLine(h);
	h<<"#include "<<STRING_TYPENAME_INCLUDE<<m_endl;
	if(classifierInfo->hasVectorFields)
	{
		h<<"#include "<<VECTOR_TYPENAME_INCLUDE<<m_endl;
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
			h<<"using "<<cleanName(classifier->getPackage())<<"::"<<cleanName(classifier->getName())<<";"<<m_endl;
		}
	}

	if(!c->getPackage().isEmpty() && WRITE_PACKAGE_NAMESPACE)
		h<<m_endl<<"namespace "<<cleanName(c->getPackage())<<" {"<<m_endl<<m_endl;

	//Write class Documentation if there is somthing or if force option
	if(forceDoc() || !c->getDoc().isEmpty()) {
		h<<m_endl<<"/**"<<m_endl;
		h<<"  * class "<<classifierInfo->className<<m_endl;
		h<<formatDoc(c->getDoc(),"  * ");
		h<<"  */";
		writeBlankLine(h);
		writeBlankLine(h);
	}

	//check if class is abstract and / or has abstract methods
	if((c->getAbstract() || classifierInfo->isInterface )
			&& !hasAbstractOps(c))
		h<<"/******************************* Abstract Class ****************************"<<m_endl
		<<classifierInfo->className<<" does not have any pure virtual methods, but its author"<<m_endl
		<<"  defined it as an abstract class, so you should not use it directly."<<m_endl
		<<"  Inherit from it instead and create only objects from the derived classes"<<m_endl
		<<"*****************************************************************************/"<<m_endl<<m_endl;

	if (!classifierInfo->isInterface) {
		UMLClass* k = dynamic_cast<UMLClass*>(c);
		if (k->isEnumeration()) {
			h << "enum " << classifierInfo->className << " {" << m_endl;
			UMLAttributeList atl = k->getAttributeList();
			UMLAttribute *at = atl.first();
			while (at) {
				QString attrName = cleanName(at->getName());
				h << getIndent() << attrName;
				if ((at = atl.next()) == NULL)
					break;
				h << "," << m_endl;
			}
			h << m_endl << "};" << m_endl;	// end of class header
	                if(!c->getPackage().isEmpty() && WRITE_PACKAGE_NAMESPACE)
				h << "}  // end of package namespace" << m_endl;
			h << m_endl << "#endif // " << hashDefine + "_H" << m_endl;
			return;
		} else if (c->getBaseType() == Uml::ot_Enum) {
			UMLClassifierListItemList litList = c->getFilteredList(Uml::ot_EnumLiteral);
			uint i = 0;
			h << "enum " << classifierInfo->className << " {" << m_endl;
			for (UMLClassifierListItem *lit = litList.first(); lit; lit = litList.next()) {
				QString enumLiteral = cleanName(lit->getName());
				h << getIndent() << enumLiteral;
				if (++i < litList.count())
					h << ",";
				h << m_endl;
			}
			h << m_endl << "};" << m_endl;	// end of class header
	                if(!c->getPackage().isEmpty() && WRITE_PACKAGE_NAMESPACE)
				h << "}  // end of package namespace" << m_endl;
			h << m_endl << "#endif // " << hashDefine + "_H" << m_endl;
			return;
		}
	}

	// Generate template parameters.
	UMLClassifierListItemList template_params = c->getFilteredList(Uml::ot_Template);
	if (template_params.count()) {
		h << "template<";
		for (UMLClassifierListItem *li = template_params.first(); li; ) {
			QString formalName = li->getName();
			QString typeName = li->getTypeName();
			UMLClassifier *typeObj = li->getType();
			if (typeName == "class") {
				h << "class";
			} else if (typeObj == NULL) {
				kdError() << "CppWriter::writeClass(" << classifierInfo->className
					  << "): typeObj is NULL" << endl;
			} else {
				h << li->getTypeName();
			}
			h << " " << formalName;
			if ((li = template_params.next()) != NULL)
				h << ", ";
		}
		h << ">" << m_endl;
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

	h<<m_endl<<"{"<<m_endl; // begin the body of the class


	//declarations of operations
        //

	//
	// write out field and operations decl grouped by visibility
	//

	// PUBLIC attribs/methods
	h<<"public:"<<m_endl<<m_endl; // print visibility decl.
	// for public: constructors are first ops we print out
	if(!classifierInfo->isInterface)
		writeConstructorDecls(h);
	writeHeaderFieldDecl(c,Uml::Public, h);
        writeHeaderAccessorMethodDecl(c, Uml::Public, h);
	writeOperations(c,true,Uml::Public,h);

	// PROTECTED attribs/methods
	//
	h<<"protected"<<":"<<m_endl<<m_endl; // print visibility decl.
	writeHeaderFieldDecl(c,Uml::Protected, h);
        writeHeaderAccessorMethodDecl(c, Uml::Protected, h);
	writeOperations(c,true,Uml::Protected,h);

	// PRIVATE attribs/methods
	//
	h<<"private"<<":"<<m_endl<<m_endl; // print visibility decl.
	writeHeaderFieldDecl(c,Uml::Private, h);
        writeHeaderAccessorMethodDecl(c, Uml::Private, h);
	writeOperations(c,true,Uml::Private,h);
        writeInitAttibuteDecl(h); // this is always private, used by constructors to initialize class

	// end of class header
	h<<m_endl<<"};"<<m_endl;

	// end of class namespace, if any
	if(!c->getPackage().isEmpty() && WRITE_PACKAGE_NAMESPACE)
		h<<"}; // end of package namespace"<<m_endl;

	// last thing..close our hashdefine
	h << m_endl << "#endif // " << hashDefine + "_H" << m_endl;


}

void CppWriter::writeHeaderAccessorMethodDecl(UMLClassifier *c, Uml::Scope permitScope, QTextStream &stream)
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

void CppWriter::writeHeaderFieldDecl(UMLClassifier *c, Uml::Scope permitScope, QTextStream &stream)
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

	// open stream for writing
	QTextStream cpp (&filecpp);

	// set the starting indentation at zero
	m_indentLevel = 0;

	//try to find a heading file (license, coments, etc)
	QString str;
	str = getHeadingFile(".cpp");
	if(!str.isEmpty()) {
		str.replace(QRegExp("%filename%"),classifierInfo->fileName+".cpp");
		str.replace(QRegExp("%filepath%"),filecpp.name());
		cpp<<str<<m_endl;
	}

	// IMPORT statements
	// Q: Why all utils? Isnt just List and Vector the only classes we are using?
	// Our import *should* also look at operations, and check that objects being
	// used arent in another package (and thus need to be explicitly imported here).
	cpp<<"#include \""<<(classifierInfo->className).lower()<<".h\""<<m_endl;
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

void CppWriter::writeAttributeDecls (Uml::Scope visibility, bool writeStatic, QTextStream &stream )
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
                QString strVis = capitalizeFirstLetter(Umbrello::scopeToString(visibility));
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
	                stream<<getIndent()<<staticValue<<typeName<<" "<<varName<<";"<<m_endl;

	        }

/*
	        if(list->count() > 0)
	                writeBlankLine(stream);
*/

	}

	return;

}

void CppWriter::writeHeaderAttributeAccessorMethods (Uml::Scope visibility, bool writeStatic, QTextStream &stream )
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
					Uml::Scope visibility, bool isHeaderMethod,
 					bool isStatic,
					bool writeMethodBody, QTextStream &stream)
{

	if(!WRITE_ATTRIBUTE_ACCESSOR_METHODS)
		return;

	if(forceDoc() || attribs->count()>0)
	{
		QString strVis = capitalizeFirstLetter(Umbrello::scopeToString(visibility));
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
						    methodBaseName, at->getDoc(), Uml::chg_Changeable, isHeaderMethod,
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
			cpp<<myIndent<<"// "<<lines[i] << m_endl;
		}
	} else {
		// this should be more fancy in the future, breaking it up into 80 char
		// lines so that it doesnt look too bad
		cpp<<myIndent<<"// "<< comment << m_endl;
	}
}

void CppWriter::writeDocumentation(QString header, QString body, QString end, QTextStream &cpp)
{
	writeBlankLine(cpp);
	QString indent = getIndent();

	cpp<<indent<<"/**"<<m_endl;
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
	cpp<<indent<<" */"<<m_endl;
}

void CppWriter::writeAssociationDecls(UMLAssociationList associations, Uml::Scope permitScope, Uml::IDType id, QTextStream &h)
{

	if( forceSections() || !associations.isEmpty() )
	{
		bool printRoleA = false, printRoleB = false;
		for(UMLAssociation *a = associations.first(); a; a = associations.next())
		{

			// it may seem counter intuitive, but you want to insert the role of the
			// *other* class into *this* class.
			if (a->getRoleId(Uml::A) == id && a->getRoleName(Uml::B) != "")
				printRoleB = true;

			if (a->getRoleId(Uml::B) == id && a->getRoleName(Uml::A) != "")
				printRoleA = true;

			// First: we insert documentaion for association IF it has either role AND some documentation (!)
			if ((printRoleA || printRoleB) && !(a->getDoc().isEmpty()))
				writeComment(a->getDoc(), getIndent(), h);

			// print RoleB decl
			if (printRoleB && a->getVisibility(Uml::B) == permitScope)
			{

				QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::B)));
				writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::B), a->getMulti(Uml::B), a->getRoleDoc(Uml::B), h);
			}

			// print RoleA decl
			if (printRoleA && a->getVisibility(Uml::A) == permitScope)
			{
				QString fieldClassName = cleanName(getUMLObjectName(a->getObject(Uml::A)));
				writeAssociationRoleDecl(fieldClassName, a->getRoleName(Uml::A), a->getMulti(Uml::A), a->getRoleDoc(Uml::A), h);
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

		stream<<indent<<fieldClassName<<" * "<<fieldVarName<<";"<<m_endl;
	}
	else
	{
		QString fieldVarName = "m_" + roleName.lower() + "Vector";

        	// record unique occurances for later when we want to check
        	// for initialization of this vector
        	if(VectorFieldVariables.findIndex(fieldVarName) == -1)
                	VectorFieldVariables.append(fieldVarName);

		stream << indent << VECTOR_TYPENAME <<"<" << fieldClassName << "*";
		stream << "> " << fieldVarName << ";" << m_endl;
	}
}

// for either source or header files
void CppWriter::writeAssociationMethods (UMLAssociationList associations,
					Uml::Scope permitVisib,
					bool isHeaderMethod,
					bool writeMethodBody,
					bool writePointerVar,
					Uml::IDType myID, QTextStream &stream)
{
	if( forceSections() || !associations.isEmpty() )
	{
		for(UMLAssociation *a = associations.first(); a; a = associations.next())
		{

			// insert the methods to access the role of the other
			// class in the code of this one
			if (a->getRoleId(Uml::A) == myID && a->getVisibility(Uml::B) == permitVisib)
			{
				// only write out IF there is a rolename given
				if(!a->getRoleName(Uml::B).isEmpty()) {
					QString fieldClassName = getUMLObjectName(a->getObject(Uml::B)) + (writePointerVar ? " *":"");
					writeAssociationRoleMethod(fieldClassName,
									isHeaderMethod,
									writeMethodBody,
								   a->getRoleName(Uml::B),
								   a->getMulti(Uml::B), a->getRoleDoc(Uml::B),
								   a->getChangeability(Uml::B), stream);
				}
			}

			if (a->getRoleId(Uml::B) == myID && a->getVisibility(Uml::A) == permitVisib)
			{
				// only write out IF there is a rolename given
				if(!a->getRoleName(Uml::A).isEmpty()) {
					QString fieldClassName = getUMLObjectName(a->getObject(Uml::A)) + (writePointerVar ? " *":"");
					writeAssociationRoleMethod(fieldClassName,
									isHeaderMethod,
									writeMethodBody,
								   a->getRoleName(Uml::A),
								   a->getMulti(Uml::A),
								   a->getRoleDoc(Uml::A),
								   a->getChangeability(Uml::A),
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
					    QString description, Uml::Changeability_Type change,
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
						      Uml::Changeability_Type changeType,
						      bool isHeaderMethod,
						      bool writeMethodBody,
						      QTextStream &stream)
{

	fieldClassName = fixTypeName(fieldClassName);
	fieldName = capitalizeFirstLetter(fieldName);
	QString indent = getIndent();

	// ONLY IF changeability is NOT Frozen
	if (changeType != Uml::chg_Frozen)
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
			stream<<indent<<" {"<<m_endl;
	                m_indentLevel++;
			printTextAsSeparateLinesWithIndent(method,getIndent(),stream);
	                m_indentLevel--;
			stream<<indent<<"}"<<m_endl;
		} else
			stream<<";"<<m_endl;
	}

	// ONLY IF changeability is Changeable
	if (changeType == Uml::chg_Changeable)
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
			stream<<indent<<" {"<<m_endl;
	                m_indentLevel++;
			printTextAsSeparateLinesWithIndent(method,getIndent(),stream);
	                m_indentLevel--;
			stream<<indent<<"}"<<m_endl;
		} else
			stream<<";"<<m_endl;
	}

	// always allow getting the list of stuff
	QString returnVarName = VECTOR_TYPENAME+"<"+fieldClassName+">";
	writeDocumentation("Get the list of "+fieldName+" objects held by "+fieldVarName,description,"@return "+returnVarName+" list of "+fieldName+" objects held by "+fieldVarName,stream);
	stream<<indent<<returnVarName<<" ";
	if(!isHeaderMethod)
		stream<<classifierInfo->className<<"::";
	stream<<"get"<<fieldName<<"List ( )";
	if(writeMethodBody) {
		stream<<indent<<" {"<<m_endl;
		m_indentLevel++;
		stream<<getIndent()<<"return "<<fieldVarName<<";"<<m_endl;
		m_indentLevel--;
		stream<<indent<<"}"<<m_endl;
	} else
		stream<<";"<<m_endl;

}


void CppWriter::writeSingleAttributeAccessorMethods(QString fieldClassName, QString fieldVarName,
						     QString fieldName, QString description,
						     Uml::Changeability_Type change,
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
	if (change == Uml::chg_Changeable && !isStatic) {
		writeDocumentation("Set the value of "+fieldVarName,description,"@param new_var the new value of "+fieldVarName,stream);
		stream<<indent<<"void ";
		if(!isHeaderMethod)
			stream<<classifierInfo->className<<"::";
		stream<<"set"<<fieldName<<" ( "<<fieldClassName<<" new_var )";

		if(writeMethodBody) {
			stream<<indent<<" {"<<m_endl;
			m_indentLevel++;
			stream<<getIndent()<<indent;
			m_indentLevel--;
			if(isStatic)
				stream<<classifierInfo->className<<"::";
			stream<<fieldVarName<<" = new_var;"<<m_endl;
			stream<<indent<<"}"<<m_endl;
		} else
		       stream<<";"<<m_endl;
	}

	// get method
	writeDocumentation("Get the value of "+fieldVarName,description,"@return the value of "+fieldVarName,stream);
	stream<<indent<<fieldClassName<<" ";
	if(!isHeaderMethod)
		stream<<classifierInfo->className<<"::";
	stream<<"get"<<fieldName<<" ( )";

	if(writeMethodBody) {
		stream<<indent<<" {"<<m_endl;
		m_indentLevel++;
		stream<<getIndent()<<"return ";
		m_indentLevel--;
		if(isStatic)
			stream<<classifierInfo->className<<"::";
		stream<<fieldVarName<<";"<<m_endl;
		stream<<indent<<"}";
	} else
		       stream<<";"<<m_endl;

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
		stream<<getIndent()<<classifierInfo->className<<" ( );"<<m_endl;
	}

	if(WRITE_EMPTY_DESTRUCTOR)
	{
		writeDocumentation("", "Empty Destructor", "", stream);
		stream<<getIndent();
		if (WRITE_VIRTUAL_DESTRUCTORS)
			stream<<"virtual ";
		stream<<"~"<<classifierInfo->className<<" ( );"<<m_endl;
	}

	if(WRITE_EMPTY_DESTRUCTOR || WRITE_EMPTY_CONSTRUCTOR)
		writeBlankLine(stream);
}

void CppWriter::writeInitAttibuteDecl (QTextStream &stream)
{
	if(WRITE_EMPTY_CONSTRUCTOR && classifierInfo->hasAttributes)
		stream<<getIndent()<<"void initAttributes ( ) ;"<<m_endl;
}

void CppWriter::writeInitAttibuteMethod (QTextStream &stream)
{

	// only need to do this under certain conditions
	if(WRITE_EMPTY_CONSTRUCTOR && classifierInfo->hasAttributes)
	{
		QString className = classifierInfo->className;
		QString indent = getIndent();

		stream<<indent<<"void "<<className<<"::"<<"initAttributes ( ) {"<<m_endl;

		m_indentLevel++;
		// first, initiation of fields derived from attributes
		UMLAttributeList* atl = classifierInfo->getAttList();
		for(UMLAttribute *at = atl->first(); at ; at = atl->next()) {
			if(!at->getInitialValue().isEmpty()) {
				QString varName = getAttributeVariableName(at);
				stream<<getIndent()<<varName<<" = "<<at->getInitialValue()<<";"<<m_endl;
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
				stream<<getIndent()<<method<<m_endl;
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
				stream<<getIndent()<<method<<m_endl;
			}
		}

		// clean up
		ObjectFieldVariables.clear(); // shouldnt be needed?
		VectorFieldVariables.clear(); // shouldnt be needed?

		m_indentLevel--;

		stream<<indent<<"}"<<m_endl;
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
		stream<<indent<<className<<"::"<<className<<" ( ) {"<<m_endl;
		if(classifierInfo->hasAttributes)
			stream<<indent<<INDENT<<"initAttributes();"<<m_endl;
		stream<<indent<<"};"<<m_endl;
		writeBlankLine(stream);
	}

	// empty destructor
	if(WRITE_EMPTY_DESTRUCTOR)
	{
		stream<<getIndent()<<className<<"::~"<<className<<" ( ) { };"<<m_endl;
		writeBlankLine(stream);
	}

}

// IF the type is "string" we need to declare it as
// the Java Object "String" (there is no string primative in Java).
QString CppWriter::fixTypeName(QString string)
{
	if (string.isEmpty())
		return "void";
	string.replace(QRegExp("^string$"), STRING_TYPENAME);
	return string;
}

void CppWriter::writeOperations(UMLClassifier *c, bool isHeaderMethod,
				Uml::Scope permitScope, QTextStream &cpp) {

	UMLOperationList oplist;

	//sort operations by scope first and see if there are abstract methods
	UMLOperationList inputlist = c->getOpList();
	for (UMLOperation *op = inputlist.first(); op; op = inputlist.next()) {
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

		QString methodReturnType = fixTypeName(op->getTypeName());
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
		cpp<<getIndent()<<str<<m_endl;
		writeBlankLine(cpp);
	}
}

// To prevent circular including when both classifiers on either end
// of an association have roles we need to have forward declaration of
// the other class...but only IF its not THIS class (as could happen
// in self-association relationship).
void CppWriter::printAssociationIncludeDecl (UMLAssociationList list, Uml::IDType myId, QTextStream &stream)
{

	for (UMLAssociation *a = list.first(); a; a = list.next()) {
		UMLClassifier *current = NULL;
		bool isFirstClass = true;

		// only use OTHER classes (e.g. we dont need to write includes for ourselves!!
		// AND only IF the roleName is defined, otherwise, its not meant to be noticed.
		if (a->getRoleId(Uml::A) == myId && a->getRoleName(Uml::B) != "") {
			current = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
		} else if (a->getRoleId(Uml::B) == myId && a->getRoleName(Uml::A) != "") {
			current = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
			isFirstClass = false;
		}

		// as header doc for this method indicates, we need to be a bit sophisticated on
		// how to declare some associations.
		if( current )
			if( !isFirstClass && !a->getRoleName(Uml::A).isEmpty() && !a->getRoleName(Uml::B).isEmpty())
				stream<<"class "<<current->getName()<<";"<<m_endl; // special case: use forward declaration
			else
				stream<<"#include \""<<current->getName().lower()<<".h\""<<m_endl; // just the include statement
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
	stream<<m_endl;
}

void CppWriter::printTextAsSeparateLinesWithIndent (QString text, QString indent, QTextStream &stream)
{
	if(text.isEmpty())
		return;

	QStringList lines = QStringList::split( "\n", text);
	for(uint i= 0; i < lines.count(); i++)
		stream<<indent<<lines[i]<<m_endl;
}

QString CppWriter::getAttributeVariableName (UMLAttribute *at)
{
	QString fieldName = "m_" + cleanName(at->getName());
	return fieldName;
}

void CppWriter::createDefaultDatatypes() {
	Umbrello::createCppDatatypes();
}

const QStringList CppWriter::reservedKeywords() const {
	return Umbrello::reservedCppKeywords();
}

