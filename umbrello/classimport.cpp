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
	if (o == NULL) {
		o = UMLDoc::createUMLObject( type, name, parentPkg );
		QString strippedComment = doxyComment(comment);
		if (! strippedComment.isEmpty()) {
			o->setDoc(strippedComment);
			UMLDoc::getDocWindow()->showDocumentation(o, true);
		}
	}
	return o;
}

void ClassImport::insertAttribute(UMLObject *o, Uml::Scope scope, QString name,
				  QString type, QString comment /* = "" */) {
	int attID = getUniqueID();

	UMLAttribute *temp = ((UMLClass*)o)->addAttribute(name , attID);

	temp->setTypeName(type);
	temp->setScope(scope);
	QString strippedComment = doxyComment(comment);
	if (! strippedComment.isEmpty()) {
		temp->setDoc(strippedComment);
		UMLDoc::getDocWindow()->showDocumentation(temp, true);
	}

	setModified(true);
}

void ClassImport::insertMethod(UMLObject *o, Uml::Scope scope, QString name,
			       QString type, QString comment /* = "" */,
			       UMLAttributeList *parList /*= NULL*/) {
	UMLClassifier *classifier = dynamic_cast<UMLClassifier*>(o);
	if(!classifier)
	{
		kdWarning()<<"ClassImport::insertMethod(..) called for a non-classifier!"<<endl;
		return;
	}
	UMLOperation *op = UMLDoc::createOperation( classifier, name, parList );
	if(!op)
	{
		kdError()<<"Could not create operation with name "<<name<<endl;
		return;
	}
	op->setReturnType(type);
	op->setScope(scope);
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

	QStrList *cList;
	cList = classParser.store.getSortedClassNameList();
	QStrListIterator it(*cList);

	for(; it.current();++it ) {
		UMLObject *currentClass;
		CParsedClass* currentParsedClass =  classParser.store.getClassByName(it);
		QPtrList<CParsedAttribute> *attributes = currentParsedClass->getSortedAttributeList();
		QPtrListIterator<CParsedAttribute> aIt(*attributes);

		QString pkgName( currentParsedClass->declaredInScope );
		UMLPackage *pkg = NULL;
		if( ! pkgName.isEmpty() )
			pkg = (UMLPackage *)createUMLObject(Uml::ot_Package, pkgName);
		currentClass = createUMLObject(Uml::ot_Class,
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
			this->insertAttribute(currentClass, attrScope, attr->name, attr->type, attr->comment);
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
			this->insertMethod(currentClass, attrScope, pMethod->name,
					   methodType, pMethod->comment, &parList);
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
} // method

