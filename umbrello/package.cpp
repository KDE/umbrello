/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "package.h"
#include "uml.h"
#include "umldoc.h"
#include <kdebug.h>
#include <klocale.h>

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

UMLPackage* UMLPackage::clone() const
{
	UMLPackage *clone = new UMLPackage();
	copyInto(clone);

	return clone;
}

void UMLPackage::addObject(const UMLObject *pObject) {
	bool alreadyThere = false;
	int id = pObject->getID();
	for (UMLObject *o = m_objects.first(); o; o = m_objects.next()) {
		if (o->getID() == id) {
			alreadyThere = true;
			break;
		}
	}
	if (! alreadyThere)
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
	for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
	{
		if (obj->getID() == id)
			return obj;
		if (obj->getBaseType() == Uml::ot_Package) {
			UMLObject *o = ((UMLPackage*)obj)->findObject(id);
			if (o)
				return o;
		}
	}
	return NULL;
}

bool UMLPackage::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement packageElement = qDoc.createElement("UML:Package");
	bool status = UMLObject::saveToXMI(qDoc, packageElement);

#ifndef XMI_FLAT_PACKAGES
	for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
		obj->saveToXMI (qDoc, packageElement);
#endif

	qElement.appendChild(packageElement);
	return status;
}

bool UMLPackage::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	return load(element);
}

bool UMLPackage::load(QDomElement& element) {
	UMLDoc *umldoc = UMLApp::app()->getDocument();
	QDomNode node = element.firstChild();
	element = node.toElement();
	while (!element.isNull()) {
		QString type = element.tagName();
		QDomElement tempElement = element;
		if (type == "UML:Namespace.ownedElement") {
			//CHECK: Umbrello currently assumes that nested elements
			// are ownedElements anyway.
			// Therefore the <UML:Namespace.ownedElement> tag is of no
			// significance.
			if (! load(tempElement))
				return false;
			node = node.nextSibling();
			element = node.toElement();
			continue;
		}
		UMLObject *pObject = umldoc->makeNewUMLObject(type);
		if( !pObject ) {
			kdWarning() << "UMLPackage::loadFromXMI: "
				    << "Given wrong type of umlobject to create: "
				    << type << endl;
			return false;
		}
		pObject->setUMLPackage(this);
		if (pObject->loadFromXMI(tempElement)) {
			addObject(pObject);
			if (type == "UML:Generalization")
				umldoc->addAssocToConcepts((UMLAssociation *) pObject);
		} else {
			delete pObject;
		}
		node = node.nextSibling();
		element = node.toElement();
	}
	return true;
}

#include "package.moc"
