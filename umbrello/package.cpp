/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// own header file
#include "package.h"

// system includes
#include <kdebug.h>
#include <klocale.h>

// local includes
#include "uml.h"
#include "umldoc.h"
#include "classifier.h"
#include "class.h"
#include "interface.h"
#include "model_utils.h"

using namespace Uml;

UMLPackage::UMLPackage(const QString & name, int id)
  : UMLCanvasObject(name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLPackage::~UMLPackage() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLPackage::init() {
	m_BaseType = ot_Package;
}

void UMLPackage::copyInto(UMLPackage *rhs) const
{
	UMLCanvasObject::copyInto(rhs);

	m_objects.copyInto(&(rhs->m_objects));
}

UMLObject* UMLPackage::clone() const
{
	UMLPackage *clone = new UMLPackage();
	copyInto(clone);

	return clone;
}

void UMLPackage::addObject(const UMLObject *pObject) {
	int id = pObject->getID();
	for (UMLObject *o = m_objects.first(); o; o = m_objects.next()) {
		if (o->getID() == id) {
#ifdef VERBOSE_DEBUGGING
			kdDebug() << "UMLPackage::addObject: "
				  << pObject->getName()
				  << " is already there" << endl;
#endif
			return;
		}
	}
	m_objects.append( pObject );
}

void UMLPackage::removeObject(const UMLObject *pObject) {
	m_objects.remove( pObject );
}

UMLObjectList& UMLPackage::containedObjects() {
	return m_objects;
}

UMLObject * UMLPackage::findObject(QString name) {
	for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
		if (obj->getName() == name)
			return obj;
	return NULL;
}

UMLObject * UMLPackage::findObject(int id) {
	return Umbrello::findObjectInList(id, m_objects);
}

UMLObject* UMLPackage::findObjectByIdStr(QString idStr) {
	for (UMLObject * o = m_objects.first(); o; o = m_objects.next()) {
		if (o->getAuxId() == idStr)
			return o;
		if (o->getBaseType() == Uml::ot_Package) {
			UMLObject *inner = ((UMLPackage*)o)->findObjectByIdStr(idStr);
			if (inner)
				return inner;
		}
	}
	return NULL;
}

void UMLPackage::appendClassifiers(UMLClassifierList& classifiers,
				   bool includeNested /* = true */) {
	for (UMLObject *o = m_objects.first(); o; o = m_objects.next()) {
		Object_Type ot = o->getBaseType();
		if (ot == ot_Class || ot == ot_Interface ||
		    ot == ot_Datatype || ot == ot_Enum) {
			classifiers.append((UMLClassifier *)o);
		} else if (includeNested && ot == ot_Package) {
			UMLPackage *inner = static_cast<UMLPackage *>(o);
			inner->appendClassifiers(classifiers);
		}
	}
}

void UMLPackage::appendClasses(UMLClassList& classes,
			      bool includeNested /* = true */) {
	for (UMLObject *o = m_objects.first(); o; o = m_objects.next()) {
		Object_Type ot = o->getBaseType();
		if (ot == ot_Class) {
			classes.append((UMLClass *)o);
		} else if (includeNested && ot == ot_Package) {
			UMLPackage *inner = static_cast<UMLPackage *>(o);
			inner->appendClasses(classes);
		}
	}
}

void UMLPackage::appendClassesAndInterfaces(UMLClassifierList& classifiers,
				 	    bool includeNested /* = true */) {
	for (UMLObject *o = m_objects.first(); o; o = m_objects.next()) {
		Object_Type ot = o->getBaseType();
		if (ot == ot_Class || ot == ot_Interface) {
			classifiers.append((UMLClassifier *)o);
		} else if (includeNested && ot == ot_Package) {
			UMLPackage *inner = static_cast<UMLPackage *>(o);
			inner->appendClassesAndInterfaces(classifiers);
		}
	}
}

void UMLPackage::appendInterfaces( UMLInterfaceList& interfaces,
				   bool includeNested /* = true */) {
	for (UMLObject *o = m_objects.first(); o; o = m_objects.next()) {
		Object_Type ot = o->getBaseType();
		if (ot == ot_Interface) {
			interfaces.append((UMLInterface *)o);
		} else if (includeNested && ot == ot_Package) {
			UMLPackage *inner = static_cast<UMLPackage *>(o);
			inner->appendInterfaces(interfaces);
		}
	}
}

bool UMLPackage::resolveRef() {
	// UMLObject::resolveRef() is not required by ot_Package itself
	// but might be required by some inheriting class.
	bool overallSuccess = UMLObject::resolveRef();
	for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
		UMLObject *obj = oit.current();
		if (! obj->resolveRef())
			overallSuccess = false;
	}
	return overallSuccess;
}

void UMLPackage::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement packageElement = UMLObject::save("UML:Package", qDoc);

#ifndef XMI_FLAT_PACKAGES
	// Save datatypes first.
	// This will cease to be necessary once deferred type resolution is up.
	for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
		if (obj->getBaseType() == Uml::ot_Datatype)
			obj->saveToXMI (qDoc, packageElement);
	for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
		if (obj->getBaseType() != Uml::ot_Datatype)
			obj->saveToXMI (qDoc, packageElement);
#endif
	qElement.appendChild(packageElement);
}

bool UMLPackage::load(QDomElement& element) {
	UMLDoc *umldoc = UMLApp::app()->getDocument();
	QDomNode node = element.firstChild();
	if (node.isComment())
		node = node.nextSibling();
	QDomElement tempElement = node.toElement();
	while (!tempElement.isNull()) {
		QString type = tempElement.tagName();
		if (tagEq(type, "Namespace.ownedElement") ||
		    tagEq(type, "Namespace.contents")) {
			//CHECK: Umbrello currently assumes that nested elements
			// are ownedElements anyway.
			// Therefore these tags are not further interpreted.
			if (! load(tempElement))
				return false;
			node = node.nextSibling();
			if (node.isComment())
				node = node.nextSibling();
			tempElement = node.toElement();
			continue;
		}
		UMLObject *pObject = umldoc->makeNewUMLObject(type);
		if( !pObject ) {
			kdWarning() << "UMLPackage::load: "
				    << "Unknown type of umlobject to create: "
				    << type << endl;
			node = node.nextSibling();
			if (node.isComment())
				node = node.nextSibling();
			tempElement = node.toElement();
			continue;
		}
		pObject->setUMLPackage(this);
		if (pObject->loadFromXMI(tempElement)) {
			addObject(pObject);
			if (tagEq(type, "Generalization"))
				umldoc->addAssocToConcepts((UMLAssociation *) pObject);
		} else {
			delete pObject;
		}
		node = node.nextSibling();
		if (node.isComment())
			node = node.nextSibling();
		tempElement = node.toElement();
	}
	return true;
}

#include "package.moc"
