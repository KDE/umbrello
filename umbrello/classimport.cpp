/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classimport.h"
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "classparser/ClassParser.h"
#include "classparser/ParsedArgument.h"
#include "classparser/ParsedAttribute.h"
#include "classparser/ParsedClass.h"
#include "classparser/ParsedMethod.h"
#include <kdebug.h>

ClassImport::ClassImport(QWidget *parent, const char *name) : UMLDoc(parent, name) {
}

ClassImport::~ClassImport() {}
/** No descriptions */

UMLObject *ClassImport::createUMLObject(QString className, Uml::UMLObject_Type type) {

	UMLObject * o = findUMLObject(type, className);

	if(!o) {
		if(type == Uml::ot_Class) {
			UMLClass *c = new UMLClass(this, className, ++uniqueID);
			o = (UMLObject*)c;
		} else {
			kdDebug() << "CreateUMLObject(int) error" << endl;
			return o;
		}

		objectList.append(o);
		emit sigObjectCreated(o);
		setModified(true);
		return o;
	}
	return o;//gustavo - I put it here after compiler warning
}

/** No descriptions */
void ClassImport::insertAttribute(UMLObject *o, Uml::Scope scope, QString name, QString type, QString value /*= ""*/) {
	int attID = ++uniqueID;

	UMLAttribute *temp = reinterpret_cast<UMLAttribute *>(((UMLClass*)o)->addAttribute(name , attID));

	temp->setTypeName(type);
	temp->setInitialValue(value);
	temp->setScope(scope);

	setModified(true);
}

/** No descriptions */
void ClassImport::insertMethod(UMLObject *o, Uml::Scope scope, QString name, QString type, UMLAttributeList *parList /*= NULL*/) {
	int attID = ++uniqueID;

	UMLOperation *temp = reinterpret_cast<UMLOperation *>(((UMLClass*)o)->addOperation(name , attID));
	temp->setReturnType(type);

	if(parList != NULL) {
		UMLAttributeListIt it(*parList);
		for( ; it.current(); ++it ) {
			UMLAttribute *par = it.current();
			int parID = ++uniqueID;
			par->setID(parID);
			temp->addParm(par);
		}
	}

	temp->setScope(scope);
	setModified(true);
}

void ClassImport::importCPP(QStringList headerFileList) {
	CClassParser classParser;

	QStringList::Iterator fileIT = headerFileList.begin();
	for( ; fileIT != headerFileList.end(); ++fileIT) {
		classParser.parse( (*fileIT).latin1() );
	} // for

	QStrList *cList;
	cList = classParser.store.getSortedClassNameList();
	QStrListIterator it(*cList);

	for(; it.current();++it ) {
		UMLObject *currentClass;
		CParsedClass* currentParsedClass =  classParser.store.getClassByName(it);
		QPtrList<CParsedAttribute> *attributes = currentParsedClass->getSortedAttributeList();
		QPtrListIterator<CParsedAttribute> aIt(*attributes);

		currentClass = createUMLObject(currentParsedClass->name, Uml::ot_Class);

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
			this->insertAttribute(currentClass, attrScope, attr->name, attr->type, "");
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
				UMLAttribute *parameter = new UMLAttribute(this, parg->name, 0);
				parameter->setTypeName(parg->type);
				parList.append(parameter);
			} //for( ; argsIt.current(); ++argsIt)

			QString methodType;
			if(!pMethod->isVirtual) {
				methodType = pMethod->type;
			} else {
				methodType = "virtual "+pMethod->type;
			}
			this->insertMethod(currentClass, attrScope, pMethod->name, methodType, &parList);
		} // method for() loop
	} // 	class for() loop
} // method

/** No descriptions */
bool ClassImport::createClass(QString className, UMLObject *uObject) {
	uObject = this->createUMLObject(className, Uml::ot_Class); // create class
	if(uObject != NULL)
		return true;
	else {
		return false;
	}
}
