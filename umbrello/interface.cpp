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

UMLInterface::UMLInterface(const QString & name, int id)
   : UMLClassifier(name, id)
{
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLInterface::~UMLInterface() {
	m_OpsList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::operator==( UMLInterface & rhs ) {
	if ( m_OpsList.count() != rhs.m_OpsList.count() ) {
		return false;
	}
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


////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLInterface::uniqChildName(const UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		return UMLCanvasObject::uniqChildName(type);
	} else if (type == ot_Operation) {
		currentName = i18n("new_operation");
	} else if (type == ot_Stereotype) {
		currentName = i18n("new_stereotype");
	} else {
		kdWarning() << "uniqChildName() called for unknown child type" << endl;
	}

	QString name = currentName;
	for (int number = 1; findChildObject(type, name).count(); ++number) {
		name = currentName + "_" + QString::number(number);
	}
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLInterface::init() {

	m_BaseType = ot_Interface;
	setStereotype( i18n("interface") );
}

void UMLInterface::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement interfaceElement = UMLObject::save("UML:Interface", qDoc);
	//save operations
	UMLClassifierListItem* pOp = 0;
	for ( pOp = m_OpsList.first(); pOp != 0; pOp = m_OpsList.next() ) {
		pOp->saveToXMI(qDoc, interfaceElement);
	}
	//save contained objects
	for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
		obj->saveToXMI (qDoc, interfaceElement);
	qElement.appendChild( interfaceElement );
}

bool UMLInterface::load(QDomElement & element) {
	return UMLClassifier::load( element );
}


#include "interface.moc"
