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
#include "class.h"
#include "operation.h"
#include "attribute.h"
#include "association.h"
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
	if (type != Uml::ot_Class) {
		kdDebug() << "CreateUMLObject(int) error" << endl;
		return NULL;
	}

	UMLObject * o = findUMLObject(type, className);
	if (o == NULL)
		o = UMLDoc::createUMLObject( type, className );
	return o;
}

/** No descriptions */
void ClassImport::insertAttribute(UMLObject *o, Uml::Scope scope, QString name, QString type, QString value /*= ""*/) {
	int attID = getUniqueID();

	UMLAttribute *temp = reinterpret_cast<UMLAttribute *>(((UMLClass*)o)->addAttribute(name , attID));

	temp->setTypeName(type);
	temp->setInitialValue(value);
	temp->setScope(scope);

	setModified(true);
}

/** No descriptions */
void ClassImport::insertMethod(UMLObject *o, Uml::Scope scope, QString name, QString type, UMLAttributeList *parList /*= NULL*/) {
	UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(o);
	if(!classifier)
	{
		kdWarning()<<"ClassImport::insertMethod(..) called for a non-classifier!"<<endl;
		return;
	}
	//UMLOperation *op = UMLApp::app()->getDocument()->createOperation( classifier, name );
	UMLOperation *op = UMLDoc::createOperation( classifier, name );
	//op->setName( name );
	op->setReturnType(type);

	if(parList != NULL) {
		UMLAttributeListIt it(*parList);
		for( ; it.current(); ++it ) {
			UMLAttribute *par = it.current();
			int parID = getUniqueID();
			par->setID(parID);
			op->addParm(par);
		}
	}
	op->setScope(scope);
	//setModified(true);
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
				UMLAttribute *parameter = new UMLAttribute(currentClass, parg->name, 0);
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

	// Create generalizations.
	for (QStrListIterator cit(*cList); cit.current(); ++cit) {
		CParsedClass* currentParsedClass =  classParser.store.getClassByName(cit);
		UMLObject *classObj = findUMLObject(Uml::ot_Class, currentParsedClass->name);
		if (classObj == NULL) {
			kdDebug() << "ClassImport::importCPP: Could not find UML object for "
				  << currentParsedClass->name << endl;
			continue;
		}

		QPtrListIterator<CParsedParent> pit(currentParsedClass->parents);
		for (; pit.current() ; ++pit) {
			CParsedParent *parsedParent = pit.current();
			UMLObject *parentObj = findUMLObject(Uml::ot_Class, parsedParent->name);
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
