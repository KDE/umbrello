/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classimport.h"
#include "uml.h"
#include "umldoc.h"
#include "docwindow.h"
#include "package.h"
#include "enum.h"
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "association.h"
#include "classparser/ClassParser.h"
#include "classparser/ParsedContainer.h"
#include "classparser/ParsedArgument.h"
#include "classparser/ParsedAttribute.h"
#include "classparser/ParsedClass.h"
#include "classparser/ParsedMethod.h"
#include <kdebug.h>
#include <qregexp.h>

ClassImport::ClassImport(QWidget *parent, const char *name) : UMLDoc(parent, name) {
}

ClassImport::~ClassImport() {}

QString ClassImport::doxyComment(QString comment) {
	QStringList lines = QStringList::split("\n", comment);
	if (lines.first() != "/**")
		return "";
	lines.pop_front();  // remove comment start
	lines.pop_back();   // remove comment end
	if (! lines.count())
		return "";
	for (QStringList::Iterator lit = lines.begin(); lit != lines.end(); ++lit) {
		(*lit).remove(QRegExp("^\\s+"));
		(*lit).remove(QRegExp("^\\*+\\s?"));
	}
	return lines.join("\n");
}

UMLObject *ClassImport::createUMLObject(Uml::UMLObject_Type type,
					QString name,
					QString comment,
					UMLPackage *parentPkg) {
	UMLObject * o = findUMLObject(name);
	if (o == NULL)
		o = UMLDoc::createUMLObject(type, name, parentPkg);
	else
		o->setUMLPackage(parentPkg);
	QString strippedComment = doxyComment(comment);
	if (! strippedComment.isEmpty()) {
		o->setDoc(strippedComment);
		UMLDoc::getDocWindow()->showDocumentation(o, true);
	}
	return o;
}

void ClassImport::insertAttribute(CClassStore& store,
				  UMLObject *o, Uml::Scope scope, QString name,
				  QString type, QString comment /* ="" */,
				  bool isStatic /* =false */) {
	QString strippedComment = doxyComment(comment);
	QString typeName(type);
	int isPointer = typeName.contains('*');
	typeName.replace(QRegExp("^const\\s+"), "");
	typeName.replace(QRegExp("[^:\\w].*$"), "");
	UMLObject *newObj = NULL;
	UMLObject *other = findUMLObject(typeName);
	if (other == NULL && store.hasClass(typeName.latin1())) {
		// "Forward declare" the class.
		other = createUMLObject(Uml::ot_Class, typeName);
	}
	if (other != NULL && other->getBaseType() != Uml::ot_Datatype) {
		kdDebug() << "ClassImport::insertAttribute: creating assoc for "
			  << name << endl;
		Uml::Association_Type assocType;
		if (isPointer)
			assocType = Uml::at_Aggregation;
		else
			assocType = Uml::at_Composition;
		UMLAssociation *assoc = new UMLAssociation(this, assocType, o, other);
		assoc->setRoleNameB(name);
		assoc->setVisibilityB(scope);
		UMLDoc::addAssociation(assoc);
		newObj = assoc;
	} else {
		int attID = getUniqueID();
		UMLAttribute *attr = ((UMLClass*)o)->addAttribute(name , attID);
		attr->setTypeName(type);
		attr->setScope(scope);
		attr->setStatic(isStatic);
		newObj = attr;
	}
	if (! strippedComment.isEmpty()) {
		newObj->setDoc(strippedComment);
		UMLDoc::getDocWindow()->showDocumentation(newObj, true);
	}

	setModified(true);
}

void ClassImport::insertMethod(UMLObject *o, Uml::Scope scope, QString name,
			       QString type, bool isStatic, bool isAbstract,
			       QString comment /* = "" */,
			       UMLAttributeList *parList /*= NULL*/) {
	if (o->getBaseType() != Uml::ot_Class)
	{
		kdWarning() << "ClassImport::insertMethod called for a non-class: "
			<< o->getName() << "(type " << o->getBaseType() << ")" << endl;
		return;
	}
	UMLClass *klass = static_cast<UMLClass*>(o);
	if(!klass)
	{
		kdWarning()<<"ClassImport::insertMethod() cannot cast object to UMLClass"<<endl;
		return;
	}
	UMLOperation *op = UMLDoc::createOperation( klass, name, parList );
	if(!op)
	{
		kdError()<<"Could not create operation with name "<<name<<endl;
		return;
	}
	op->setScope(scope);
	op->setReturnType(type);
	op->setStatic(isStatic);
	op->setAbstract(isAbstract);
	QString strippedComment = doxyComment(comment);
	if (! strippedComment.isEmpty()) {
		op->setDoc(strippedComment);
		UMLDoc::getDocWindow()->showDocumentation(op, true);
	}
	//setModified(true);
}

void ClassImport::importCPP(QStringList headerFileList) {
	CClassParser classParser;

	QStringList::Iterator fileIT = headerFileList.begin();
	for( ; fileIT != headerFileList.end(); ++fileIT) {
		classParser.parse( (*fileIT).latin1() );
	} // for

	// Do the classes.
	QStrList *cList;
	cList = classParser.store.getSortedClassNameList();
	QStrListIterator it(*cList);

	for(; it.current();++it ) {
		CParsedClass* currentParsedClass =  classParser.store.getClassByName(it);
		QPtrList<CParsedAttribute> *attributes = currentParsedClass->getSortedAttributeList();
		QPtrListIterator<CParsedAttribute> aIt(*attributes);

		QString pkgName( currentParsedClass->declaredInScope );
		UMLPackage *pkg = NULL;
		if( ! pkgName.isEmpty() )
			pkg = (UMLPackage *)createUMLObject(Uml::ot_Package, pkgName);
		UMLObject *currentClass = createUMLObject(Uml::ot_Class,
							  currentParsedClass->name,
							  currentParsedClass->comment,
							  pkg);
		if (pkg)
			pkg->addObject( currentClass );

		for( ; aIt.current() ; ++aIt) {
			CParsedAttribute *attr = aIt.current();
			QString scope;
			Uml::Scope attrScope = Uml::Public;
			switch(attr->exportScope) {
				case PIE_PUBLIC:
					scope = "public";
					attrScope = Uml::Public;
					break;

				case PIE_PROTECTED:
					scope = "protected";
					attrScope = Uml::Protected;

					break;

				case PIE_PRIVATE:
					scope = "private";
					attrScope = Uml::Private;
					break;
				case PIE_GLOBAL:
					break;
			} //switch
			this->insertAttribute(classParser.store,
			                      currentClass, attrScope, attr->name,
					      attr->type, attr->comment, attr->isStatic);
		} // attribute for() loop

		//CParsedMethod *aMethod;
		QPtrList<CParsedMethod> *methods = currentParsedClass->getSortedMethodList();
		QPtrListIterator<CParsedMethod> mIt(*methods);

		for(;mIt.current(); ++mIt) {
			CParsedMethod *pMethod = mIt.current();
			QPtrListIterator<CParsedArgument> argsIt(pMethod->arguments);
			QString scope = "";
			Uml::Scope attrScope = Uml::Public;
			switch(pMethod->exportScope) {
				case PIE_PUBLIC:
					scope = "public";
					attrScope = Uml::Public;
					break;

				case PIE_PROTECTED:
					scope = "protected";
					attrScope = Uml::Protected;
					break;

				case PIE_PRIVATE:
					scope = "private";
					attrScope = Uml::Protected;
					break;
				case PIE_GLOBAL:
					break;
			} //switch

			UMLAttributeList parList;

			for( ; argsIt.current(); ++argsIt) {
				CParsedArgument *parg = argsIt.current();
				UMLAttribute *parameter = new UMLAttribute(currentClass, parg->name, 0);
				parameter->setTypeName(parg->type);
				parList.append(parameter);
			} //for( ; argsIt.current(); ++argsIt)

			QString methodType;
			if(!pMethod->isVirtual) {
				methodType = pMethod->type;
			} else {
				//CHECK: This is a C++ specific hack.
				// How do we express virtuality in UML?
				methodType = "virtual "+pMethod->type;
			}
			this->insertMethod(currentClass, attrScope, pMethod->name,
					   methodType, pMethod->isStatic, pMethod->isPure,
					   pMethod->comment, &parList);
		} // method for() loop
	} // 	class for() loop

	// Create generalizations.
	for (QStrListIterator cit(*cList); cit.current(); ++cit) {
		CParsedClass* currentParsedClass =  classParser.store.getClassByName(cit);
		UMLObject *classObj = findUMLObject(currentParsedClass->name, Uml::ot_Class);
		if (classObj == NULL) {
			kdDebug() << "ClassImport::importCPP: Could not find UML object for "
				  << currentParsedClass->name << endl;
			continue;
		}

		QPtrListIterator<CParsedParent> pit(currentParsedClass->parents);
		for (; pit.current() ; ++pit) {
			CParsedParent *parsedParent = pit.current();
			UMLObject *parentObj = findUMLObject(parsedParent->name, Uml::ot_Class);
			if (parentObj == NULL) {
				kdDebug() << "ClassImport::importCPP: Could not find UML object for parent "
					  << parsedParent->name << endl;
				continue;
			}
			UMLAssociation *assoc = new UMLAssociation( this, Uml::at_Generalization,
								    classObj, parentObj );
			addAssociation(assoc);
		}
	}

	// Do the enums.
	CParsedEnum *parsedEnum;
	QPtrList<CParsedEnum> *eList = classParser.store.getSortedEnumList();
	for (QPtrListIterator<CParsedEnum> eit(*eList);
	     (parsedEnum = eit.current()) != 0; ++eit)
	{
		QString scopeName( parsedEnum->declaredInScope );
		UMLPackage *pkg = NULL;
		if (! scopeName.isEmpty()) {
			UMLObject *scopeObj = findUMLObject(scopeName);
			if (scopeObj == NULL)
				pkg = (UMLPackage*)UMLDoc::createUMLObject(Uml::ot_Package, scopeName);
			else if (scopeObj->getBaseType() == Uml::ot_Package)
				pkg = (UMLPackage *)scopeObj;
			// We don't support class scope.
		}
		UMLObject *c = createUMLObject(Uml::ot_Enum,
					       parsedEnum->name,
					       parsedEnum->comment,
					       pkg);
		if (pkg) {
			pkg->addObject( c );
			UMLDoc::setModified(true, false);
		}
		UMLEnum *e = static_cast<UMLEnum*>( c );
		for (QStringList::Iterator lit = parsedEnum->literals.begin();
		     lit != parsedEnum->literals.end(); ++lit)
			e->addEnumLiteral(*lit, UMLDoc::getUniqueID());
	}

	// Do the typedefs.
	CParsedTypedef *parsedTypedef;
	QPtrList<CParsedTypedef> *tList = classParser.store.getSortedTypedefList();
	for (QPtrListIterator<CParsedTypedef> tit(*tList);
	     (parsedTypedef = tit.current()) != 0; ++tit)
	{
		QString scopeName( parsedTypedef->declaredInScope );
		UMLPackage *pkg = NULL;
		if (! scopeName.isEmpty()) {
			UMLObject *scopeObj = findUMLObject(scopeName);
			if (scopeObj == NULL)
				pkg = (UMLPackage*)UMLDoc::createUMLObject(Uml::ot_Package, scopeName);
			else if (scopeObj->getBaseType() == Uml::ot_Package)
				pkg = (UMLPackage *)scopeObj;
			// We don't support class scope.
		}
		UMLObject *c = createUMLObject(Uml::ot_Datatype,
					       parsedTypedef->name,
					       parsedTypedef->comment,
					       pkg);
		if (pkg) {
			pkg->addObject( c );
			UMLDoc::setModified(true, false);
		}
	}

} // method

