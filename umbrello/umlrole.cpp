/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream.h>
#include "umlrole.h"
#include "association.h"

// constructor
UMLRole::UMLRole(UMLAssociation * parent) 
    : UMLObject((UMLObject *)parent) 
{
	init();

	// connect this up to parent
        connect(this,SIGNAL(modified()),parent,SIGNAL(modified()));

}

bool UMLRole::operator==(UMLRole &rhs) {
	if (this == &rhs) {
			return true;
	}
	return( UMLObject::operator==( rhs ) &&
		m_Id == rhs.m_Id &&
		m_Changeability == rhs.m_Changeability &&
		m_Visibility == rhs.m_Visibility &&
		m_Multi == rhs.m_Multi &&
		m_Name == rhs.m_Name 
              );
}

/*
bool UMLRole::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement associationElement = qDoc.createElement( "UML:Association" );
	bool status = UMLObject::saveToXMI( qDoc, associationElement );

	associationElement.setAttribute( "name", getName() );
	// associationElement.setAttribute( "assoctype", toString( getAssocType() ) );
	associationElement.setAttribute( "assoctype", getAssocType() );
	associationElement.setAttribute( "rolea", getRoleAId() );
	associationElement.setAttribute( "roleb", getRoleBId() );
	associationElement.setAttribute( "multia", getMultiA() );
	associationElement.setAttribute( "multib", getMultiB() );
	associationElement.setAttribute( "namea", getRoleNameA() );
	associationElement.setAttribute( "nameb", getRoleNameB() );
	associationElement.setAttribute( "doca", getRoleADoc() );
	associationElement.setAttribute( "docb", getRoleBDoc() );
	associationElement.setAttribute( "visibilitya", getVisibilityA() );
	associationElement.setAttribute( "visibilityb", getVisibilityB() );
	associationElement.setAttribute( "changeabilitya", getChangeabilityA() );
	associationElement.setAttribute( "changeabilityb", getChangeabilityB() );
	qElement.appendChild( associationElement );
	return status;
}

bool UMLRole::loadFromXMI( QDomElement & element ) {

	if( !UMLObject::loadFromXMI( element ) )
		return false;

	setName(element.attribute( "name", "" ));

	setAssocType( toAssocType( element.attribute( "assoctype", "" )));

	setRoleAId(element.attribute( "rolea", "-1" ).toInt());
	setRoleBId(element.attribute( "roleb", "-1" ).toInt());

	setMultiA(element.attribute( "multia", "" ));
	setMultiB(element.attribute( "multib", "" ));

	setRoleNameA(element.attribute( "namea", "" ));
	setRoleNameB(element.attribute( "nameb", "" ));

	setRoleADoc(element.attribute( "doca", "" ));
	setRoleBDoc(element.attribute( "docb", "" ));

        // visibilty defaults to Public if it cant set it here..
        QString visibilityA = element.attribute( "visibilitya", "0");
        QString visibilityB = element.attribute( "visibilityb", "0");
        if (visibilityA.toInt() > 0)
                setVisibilityA( (Scope) visibilityA.toInt());
        if (visibilityB.toInt() > 0)
                setVisibilityB( (Scope) visibilityB.toInt());

        // Changeability defaults to "Changeable" if it cant set it here..
        QString changeabilityA = element.attribute( "changeabilitya", "0");
        QString changeabilityB = element.attribute( "changeabilityb", "0");
        if (changeabilityA.toInt() > 0)
                setChangeabilityA ( (Changeability_Type) changeabilityA.toInt());
        if (changeabilityB.toInt() > 0)
                setChangeabilityB ( (Changeability_Type) changeabilityB.toInt());

	((UMLDoc*)parent())->addAssocToConcepts(this);
	return true;
}
*/

UMLAssociation * UMLRole::getParentAssociation () {
	UMLAssociation * assoc = dynamic_cast<UMLAssociation*>(this->parent());
	return assoc;
};

UMLObject* UMLRole::getObject() {
	return m_pObject;
}

int UMLRole::getId() const {
	return m_Id;
}

Changeability_Type UMLRole::getChangeability() const {
	return m_Changeability;
}

Scope UMLRole::getVisibility() const {
	return m_Visibility;
}

QString UMLRole::getMultiplicity() const {
	return m_Multi;
}

QString UMLRole::getName() const {
	return m_Name;
}

QString UMLRole::getDoc() const {
	return m_Doc;
}

void UMLRole::setId(int id) {
	m_Id = id;
	emit modified();
}

void UMLRole::setObject (UMLObject *obj) {
	// I dont believe we need the 'modified' signal on role -b.t.
/*
        if(m_pObject)
		disconnect(m_pObject,SIGNAL(modified()),this,SIGNAL(modified()));
//        connect(obj,SIGNAL(modified()),this,SIGNAL(modified()));
*/
	m_pObject = obj;
	emit modified();
}

void UMLRole::setVisibility (Scope value) {
	m_Visibility = value;
	emit modified();
}

void UMLRole::setChangeability (Changeability_Type value) {
	m_Changeability = value;
	emit modified();
}

void UMLRole::setMultiplicity (QString multi) {
	m_Multi = multi;
	emit modified();
}

void UMLRole::setName(QString roleName) {
	m_Name = roleName;
	emit modified();
}

void UMLRole::setDoc(QString doc) {
	m_Doc = doc;
	emit modified();
}

void UMLRole::init() {
	m_pObject = 0;
	m_Id = -1;
	m_Multi = "";
	m_Name = "";
	m_Visibility = Public;
	m_Changeability = chg_Changeable;
}

