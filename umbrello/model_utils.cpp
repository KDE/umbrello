 /*
  *  copyright (C) 2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header
#include "model_utils.h"

// qt/kde includes
#include <qstringlist.h>
#include <kdebug.h>

// app includes
#include "umlobject.h"
#include "package.h"
#include "classifier.h"

namespace Umbrello {

bool isCloneable(Uml::Widget_Type type) {
	switch (type) {
		case Uml::wt_Actor:
		case Uml::wt_UseCase:
		case Uml::wt_Class:
		case Uml::wt_Interface:
		case Uml::wt_Enum:
		case Uml::wt_Datatype:
		case Uml::wt_Package:
		case Uml::wt_Component:
		case Uml::wt_Node:
		case Uml::wt_Artifact:
			return true;
		default:
			return false;
	}
}

UMLObject * findObjectInList(Uml::IDType id, UMLObjectList inList) {
	for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
		UMLObject *obj = oit.current();
		if (obj->getID() == id)
			return obj;
		UMLObject *o;
		Uml::Object_Type t = obj->getBaseType();
		switch (t) {
			case Uml::ot_Package:
				o = ((UMLPackage*)obj)->findObjectById(id);
				if (o)
					return o;
				break;
			case Uml::ot_Interface:
			case Uml::ot_Class:
			case Uml::ot_Enum:
				o = ((UMLClassifier*)obj)->findChildObject(id);
				if (o == NULL &&
				    (t == Uml::ot_Interface || t == Uml::ot_Class))
					o = ((UMLPackage*)obj)->findObjectById(id);
				if (o)
					return o;
				break;
			default:
				break;
		}
	}
	return NULL;
}

UMLObject* findUMLObject(UMLObjectList inList, QString name,
			 Uml::Object_Type type /* = ot_UMLObject */,
			 UMLObject *currentObj /* = NULL */) {
	QStringList components = QStringList::split("::", name);
	QString nameWithoutFirstPrefix;
	if (components.size() > 1) {
		name = components.front();
		components.pop_front();
		nameWithoutFirstPrefix = components.join("::");
	}
	if (currentObj) {
		UMLPackage *pkg = NULL;
		if (dynamic_cast<UMLClassifierListItem*>(currentObj)) {
			// FIXME: This is ugly. It is non-obvious what the
			// currentObj->parent() is. In particular, UMLObject
			// has a constructor that makes UMLDoc the parent()
			// - pray we're not dealing with an object that was
			// constructed like that :(
			pkg = dynamic_cast<UMLPackage*>(currentObj->parent());
		} else {
			pkg = dynamic_cast<UMLPackage*>(currentObj);
			if (pkg == NULL)
				pkg = currentObj->getUMLPackage();
		}
		for (; pkg; pkg = currentObj->getUMLPackage()) {
			UMLObjectList objectsInCurrentScope = pkg->containedObjects();
			for (UMLObjectListIt oit(objectsInCurrentScope); oit.current(); ++oit) {
				UMLObject *obj = oit.current();
				if (obj->getName() != name)
					continue;
				Uml::Object_Type foundType = obj->getBaseType();
				if (nameWithoutFirstPrefix.isEmpty()) {
					if (type != Uml::ot_UMLObject && type != foundType) {
						kdDebug() << "findUMLObject: type mismatch for "
							  << name << " (seeking type: "
							  << type << ", found type: "
							  << foundType << ")" << endl;
						continue;
					}
					return obj;
				}
				if (foundType != Uml::ot_Package &&
				    foundType != Uml::ot_Class &&
				    foundType != Uml::ot_Interface) {
					kdDebug() << "findUMLObject: found \"" << name
						  << "\" is not a package (?)" << endl;
					continue;
				}
				UMLPackage *pkg = static_cast<UMLPackage*>(obj);
				return findUMLObject( pkg->containedObjects(),
						      nameWithoutFirstPrefix, type );
			}
			currentObj = pkg;
		}
	}
	for (UMLObjectListIt oit(inList); oit.current(); ++oit) {
		UMLObject *obj = oit.current();
		if (obj->getName() != name)
			continue;
		Uml::Object_Type foundType = obj->getBaseType();
		if (nameWithoutFirstPrefix.isEmpty()) {
			if (type != Uml::ot_UMLObject && type != foundType) {
				kdDebug() << "findUMLObject: type mismatch for "
					  << name << " (seeking type: "
					  << type << ", found type: "
					  << foundType << ")" << endl;
				continue;
			}
			return obj;
		}
		if (foundType != Uml::ot_Package &&
		    foundType != Uml::ot_Class &&
		    foundType != Uml::ot_Interface) {
			kdDebug() << "findUMLObject: found \"" << name
				  << "\" is not a package (?)" << endl;
			continue;
		}
		UMLPackage *pkg = static_cast<UMLPackage*>(obj);
		return findUMLObject( pkg->containedObjects(),
				      nameWithoutFirstPrefix, type );
	}
	return NULL;
}

bool isCommonXMIAttribute( const QString &tag ) {
	bool retval = (Uml::tagEq(tag, "name") ||
		       Uml::tagEq(tag, "visibility") ||
		       Uml::tagEq(tag, "isRoot") ||
		       Uml::tagEq(tag, "isLeaf") ||
		       Uml::tagEq(tag, "isAbstract") ||
		       Uml::tagEq(tag, "isSpecification") ||
		       Uml::tagEq(tag, "isActive") ||
		       Uml::tagEq(tag, "namespace") ||
		       Uml::tagEq(tag, "ownerScope") ||
		       Uml::tagEq(tag, "GeneralizableElement.generalization") ||
		       Uml::tagEq(tag, "specialization") ||   //NYI
		       Uml::tagEq(tag, "clientDependency") || //NYI
		       Uml::tagEq(tag, "supplierDependency")  //NYI
		      );
	return retval;
}

}  // namespace Umbrello

