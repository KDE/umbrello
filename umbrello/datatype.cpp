/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "datatype.h"
#include "operation.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLDatatype::UMLDatatype(const QString& name, int id)
  : UMLClassifier(name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLDatatype::~UMLDatatype() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDatatype::operator==(UMLDatatype& rhs) {
	return UMLClassifier::operator==(rhs);
}

void UMLDatatype::copyInto(UMLDatatype *rhs) const
{
	UMLClassifier::copyInto(rhs);
}

UMLDatatype* UMLDatatype::clone() const
{
	UMLDatatype *clone = new UMLDatatype();
	copyInto(clone);

	return clone;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLDatatype::uniqChildName(UMLObject_Type type) {
	return UMLCanvasObject::uniqChildName(type);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDatatype::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement classElement = qDoc.createElement("UML:DataType");
	bool status = UMLObject::saveToXMI( qDoc, classElement );
	qElement.appendChild( classElement );
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDatatype::loadFromXMI(QDomElement& element) {
	return UMLObject::loadFromXMI(element);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLDatatype::init() {
	m_BaseType = ot_Datatype;
	setStereotype( i18n("datatype") );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLDatatype::isInterface() {
	return false;
}
