/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "stereotype.h"
#include "inputdialog.h"

#include <klocale.h>
#include <kdebug.h>

UMLStereotype::UMLStereotype( UMLObject *parent, QString name, int id,
			      UMLObject_Type listType)
  : UMLClassifierListItem( parent, name, id ) {
	m_BaseType = ot_Stereotype;
	m_listType = listType;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLStereotype::UMLStereotype(UMLObject *parent)
  : UMLClassifierListItem( parent ) {
	m_BaseType = ot_Stereotype;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLStereotype::~UMLStereotype() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLStereotype::toString(Signature_Type /*sig*/) {
	return "«" + getName() + "»";
}

bool UMLStereotype::operator==( UMLStereotype &rhs) {
	if (this == &rhs) {
		return true;
	}

	if ( !UMLObject::operator==( rhs ) ) {
		return false;
	}

	return true;
}

void UMLStereotype::copyInto(UMLStereotype *rhs) const
{
	UMLClassifierListItem::copyInto(rhs);

	// is an enum
	rhs->m_listType = m_listType;
}

UMLStereotype* UMLStereotype::clone() const
{
	UMLStereotype *clone = new UMLStereotype( (UMLStereotype *) parent());
	copyInto(clone);

	return clone;
}


bool UMLStereotype::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement stereotypeElement = qDoc.createElement("stereotype");
	bool status = UMLObject::saveToXMI(qDoc, stereotypeElement);
	stereotypeElement.setAttribute("listtype", m_listType);
	qElement.appendChild( stereotypeElement );
	return status;
}

bool UMLStereotype::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	QString listType = element.attribute("listtype", "-1");
	m_listType = (UMLObject_Type)listType.toInt();
	if (m_listType == -1) {
		kdWarning() << "<stereotype> without a listtype" << endl;
		return false;
	}
	return true;
}

bool UMLStereotype::showPropertiesDialogue(QWidget* parent) {
	bool ok;
	QString name = KInputDialog::getText(i18n("Stereotype"), i18n("Enter name:"), getName(),&ok, parent);
	if (ok) {
		setName(name);
	}
	return ok;
}

QString UMLStereotype::getShortName() {
	return "«" + getName() + "»";
}


