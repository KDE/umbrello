/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "package.h"
#include "umldoc.h"
#include <kdebug.h>
#include <klocale.h>

UMLPackage::UMLPackage(UMLDoc * parent, const QString & name, int id)
  : UMLCanvasObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLPackage::~UMLPackage() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLPackage::init() {
	m_BaseType = ot_Package;
}

void UMLPackage::addObject(const UMLObject *pObject) {
	if (! m_objects.contains( pObject ))
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

bool UMLPackage::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement packageElement = qDoc.createElement("UML:Package");
	bool status = UMLObject::saveToXMI(qDoc, packageElement);

#ifdef XMI_NEST_PACKAGES
	// Under construction
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
	UMLDoc *parentDoc = (UMLDoc*)parent();

	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();
	while (!tempElement.isNull()) {
		QString type = tempElement.tagName();
		UMLObject *pObject = parentDoc->makeNewUMLObject(type);
		if( !pObject ) {
			kdWarning() << "UMLPackage::loadFromXMI: "
				    << "Given wrong type of umlobject to create: "
				    << type << endl;
			return false;
		}
		if (! pObject->loadFromXMI(element)) {
			delete pObject;
			return false;
		}
		pObject->setUMLPackage(this);
		if (type == "UML:Generalization")
			parentDoc->addAssocToConcepts((UMLAssociation *) pObject);
		m_objects.append(pObject);

		node = node.nextSibling();
		tempElement = node.toElement();
	}
	return true;
}

#include "package.moc"
