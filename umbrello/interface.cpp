 /*
  *  copyright (C) 2003-2004
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

#include "interface.h"
#include "operation.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLInterface::UMLInterface(const QString & name, Uml::IDType id)
   : UMLClassifier(name, id)
{
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLInterface::~UMLInterface() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::operator==( UMLInterface & rhs ) {
	return UMLClassifier::operator==(rhs);
}

void UMLInterface::copyInto(UMLInterface *rhs) const
{
	UMLClassifier::copyInto(rhs);
}

UMLObject* UMLInterface::clone() const
{
	UMLInterface *clone = new UMLInterface();
	copyInto(clone);

	return clone;
}

void UMLInterface::init() {
	m_BaseType = Uml::ot_Interface;
	setStereotype( i18n("interface") );
}

void UMLInterface::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement interfaceElement = UMLObject::save("UML:Interface", qDoc);
	//save operations
	UMLOperationList opsList = getOpList();
	if (opsList.count()) {
		QDomElement featureElement = qDoc.createElement( "UML:Classifier.feature" );
		for (UMLOperation *pOp = opsList.first(); pOp; pOp = opsList.next() ) {
			pOp->saveToXMI(qDoc, featureElement);
		}
		interfaceElement.appendChild( featureElement );
	}
	//save contained objects
	if (m_objects.count()) {
		QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );
		for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
			obj->saveToXMI (qDoc, ownedElement);
		interfaceElement.appendChild( ownedElement );
	}
	qElement.appendChild( interfaceElement );
}

bool UMLInterface::load(QDomElement & element) {
	return UMLClassifier::load( element );
}


#include "interface.moc"
