/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "enumliteral.h"
#include "inputdialog.h"

#include <kdebug.h>
#include <klocale.h>

UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent, QString name, int id)
	: UMLClassifierListItem(parent, name, id) {
	m_BaseType = ot_EnumLiteral;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLEnumLiteral::UMLEnumLiteral(UMLObject *parent) : UMLClassifierListItem(parent) {
	m_BaseType = ot_EnumLiteral;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLEnumLiteral::~UMLEnumLiteral() { }
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLEnumLiteral::operator==(UMLEnumLiteral& rhs) {
	if ( this == &rhs )  {
		return true;
	}
	if ( !UMLObject::operator==( rhs ) )  {
		return false;
	}
	return true;
}

void UMLEnumLiteral::copyInto(UMLEnumLiteral *rhs) const
{
	UMLClassifierListItem::copyInto(rhs);
}

UMLEnumLiteral* UMLEnumLiteral::clone() const
{
	UMLEnumLiteral *clone = new UMLEnumLiteral((UMLObject *) parent());
	copyInto(clone);

	return clone;
}


QString UMLEnumLiteral::toString(Signature_Type /*sig = st_NoSig*/) {
	return getName();
}

bool UMLEnumLiteral::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement attributeElement = qDoc.createElement( "UML:EnumLiteral" );
	bool status = UMLObject::saveToXMI( qDoc, attributeElement );
	qElement.appendChild( attributeElement );
	return status;
}

bool UMLEnumLiteral::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) )  {
		return false;
	}
	return true;
}

bool UMLEnumLiteral::showPropertiesDialogue(QWidget* parent) {
	bool ok;
	QString name = KInputDialog::getText(i18n("Name"), i18n("Enter name:"), getName(), &ok, parent);
	if ( ok && !name.isEmpty() )  {
		setName(name);
		return true;
	} else {
		return false;
	}
}


