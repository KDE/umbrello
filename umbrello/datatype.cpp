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

void UMLDatatype::setOriginType(UMLObject *origType) {
	m_pOrigType = origType;
}

UMLObject * UMLDatatype::originType() {
	return m_pOrigType;
}

void UMLDatatype::setIsReference(bool isRef) {
	m_isRef = isRef;
}

bool UMLDatatype::isReference() {
	return m_isRef;
}

void UMLDatatype::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement classElement = UMLObject::save("UML:DataType", qDoc);
	qElement.appendChild( classElement );
}

bool UMLDatatype::load(QDomElement&) {
	return true;
}

void UMLDatatype::init() {
	m_BaseType = ot_Datatype;
	setStereotype( i18n("datatype") );
	m_pOrigType = NULL;
	m_isRef = false;
}

bool UMLDatatype::isInterface() {
	return false;
}

