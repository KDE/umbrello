/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "attribute.h"
#include "dialogs/umlattributedialog.h"

#include <kdebug.h>

UMLAttribute::UMLAttribute( UMLObject *parent, QString Name, int id, QString type,
			    Scope s, QString iv )
  : UMLClassifierListItem(parent, Name, id) {
	m_TypeName = type;
	m_InitialValue = iv;
	m_BaseType = ot_Attribute;
	m_Scope = s;
	m_ParmKind = pk_In;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute::UMLAttribute(UMLObject *parent) : UMLClassifierListItem(parent) {
	m_BaseType = ot_Attribute;
	m_TypeName = m_InitialValue = "";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute::~UMLAttribute() { }
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLAttribute::getTypeName() {
	return m_TypeName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLAttribute::setTypeName(QString type) {
	if(m_TypeName != type) {
		m_TypeName = type;
		emit modified();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLAttribute::getInitialValue() {
	return m_InitialValue;
}

void UMLAttribute::setInitialValue(QString iv) {
	if(m_InitialValue != iv) {
		m_InitialValue = iv;
		emit modified();
	}
}

void UMLAttribute::setParmKind (Uml::Parameter_Kind pk) {
	m_ParmKind = pk;
}

Uml::Parameter_Kind UMLAttribute::getParmKind () const {
	return m_ParmKind;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLAttribute::toString(Signature_Type sig) {
	QString s;

	if(sig == st_ShowSig || sig == st_NoSig) {
		if(m_Scope == Public)
			s = "+ ";
		else if(m_Scope == Private)
			s = "- ";
		else if(m_Scope == Protected)
			s= "# ";
	} else
		s = "";

	if(sig == st_ShowSig || sig == st_SigNoScope) {
		QString string = s + getName() + " : " + m_TypeName;
		if(m_InitialValue.length() > 0)
			string += " = " + m_InitialValue;
		return string;
	} else
		return s + getName();
}

bool UMLAttribute::operator==( UMLAttribute &rhs) {
	if( this == &rhs )
		return true;

	if( !UMLObject::operator==( rhs ) )
		return false;

	if( m_InitialValue != rhs.m_InitialValue )
		return false;

	if( m_TypeName != rhs.m_TypeName )
		return false;

	return true;
}

bool UMLAttribute::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement attributeElement = qDoc.createElement( "UML:Attribute" );
	bool status = UMLObject::saveToXMI( qDoc, attributeElement );
	attributeElement.setAttribute( "type", m_TypeName );
	attributeElement.setAttribute( "value", m_InitialValue );
	qElement.appendChild( attributeElement );
	return status;
}

bool UMLAttribute::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI( element ) )
		return false;
	m_TypeName = element.attribute( "type", "" );
	m_InitialValue = element.attribute( "value", "" );
	return true;
}

bool UMLAttribute::showPropertiesDialogue(QWidget* parent) {
	UMLAttributeDialog dialogue(parent, this);
	return dialogue.exec();
}
