/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

UMLAssociation * UMLRole::getParentAssociation () {
	UMLAssociation * assoc = dynamic_cast<UMLAssociation*>(this->parent());
	return assoc;
}

UMLObject* UMLRole::getObject() {
	return m_pObject;
}

int UMLRole::getId() const {
        if(m_pObject)
		return m_pObject->getID();
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
        if(m_pObject)
		m_pObject->setID(id);
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

#include "umlrole.moc"
