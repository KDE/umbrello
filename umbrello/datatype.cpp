/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "datatype.h"

#include <kdebug.h>
#include <klocale.h>
#include "operation.h"
#include "clipboard/idchangelog.h"
#include "umldoc.h"
#include "uml.h"
#include "stereotype.h"

UMLDatatype::UMLDatatype(const QString& name, int id)
  : UMLClassifier(name, id) {
	init();
}

UMLDatatype::~UMLDatatype() {
}

bool UMLDatatype::operator==(UMLDatatype& rhs) {
	return UMLClassifier::operator==(rhs);
}

void UMLDatatype::copyInto(UMLDatatype *rhs) const
{
	UMLClassifier::copyInto(rhs);
}

UMLObject* UMLDatatype::clone() const
{
	UMLDatatype *clone = new UMLDatatype();
	copyInto(clone);

	return clone;
}

void UMLDatatype::setOriginType(UMLClassifier *origType) {
	m_pOrigType = origType;
}

UMLClassifier * UMLDatatype::originType() {
	return m_pOrigType;
}

void UMLDatatype::setIsReference(bool isRef) {
	m_isRef = isRef;
}

bool UMLDatatype::isReference() {
	return m_isRef;
}

void UMLDatatype::init() {
	m_BaseType = Uml::ot_Datatype;
	setStereotype( i18n("datatype") );
	m_pOrigType = NULL;
	m_isRef = false;
}

bool UMLDatatype::isInterface() {
	return false;
}

bool UMLDatatype::resolveTypes() {
	if (m_idStr.isEmpty())
		return true;
	int id = m_idStr.toInt();
	UMLDoc *pDoc = UMLApp::app()->getDocument();
	UMLObject *typeObj = pDoc->findUMLObject(id);
	m_pOrigType = static_cast<UMLClassifier*>(typeObj);
	return true;
}

void UMLDatatype::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement classElement = UMLObject::save("UML:DataType", qDoc);
	if (m_pOrigType)
		classElement.setAttribute( "elementReference", m_pOrigType->getID() );
	qElement.appendChild( classElement );
}

bool UMLDatatype::load(QDomElement& element) {
	//CHECK: Does our usage of the elementReference attribute
	//       violate the XMI standard? - See resolveTypes()
	m_idStr = element.attribute( "elementReference", "" );
	if (!m_idStr.isEmpty()) {
		// @todo We do not currently support composition.
		m_isRef = true;
	}
	return true;
}

