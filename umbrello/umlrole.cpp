/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include "umlrole.h"
#include "association.h"

// constructor
UMLRole::UMLRole(UMLAssociation * parent, UMLObject * parentObj)
    : UMLObject( parent->getParentUMLDoc() )
{
	init(parent, parentObj);
}

bool UMLRole::operator==(UMLRole &rhs) {
	if (this == &rhs) {
			return true;
	}
	return( UMLObject::operator==( rhs ) &&
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

int UMLRole::getID() const {
        if(m_pObject)
		return m_pObject->getID();
	return -1;
}

void UMLRole::setID( int id) {
	kdError()<<"ERROR: not allowed to setID("<<id<<") for UMLRole (id is derived from parent UMLObject), ignoring set request"<<endl; 
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

/*
void UMLRole::setId(int id) {
	m_Id = id;
        if(m_pObject)
		m_pObject->setID(id);
	emit modified();
}
*/

void UMLRole::setObject (UMLObject *obj) {
	// because we will get the id of this role from the parent
	// object, we CANT allow UMLRoles to take other UMLRoles as
	// parent objects. In fact, there is probably good reason
	// to only take UMLClassifiers here, but I'll leave it more open
	// for the time being. -b.t. 
        if(obj && dynamic_cast<UMLRole*>(obj))
	{
		kdError()<<"ERROR: UMLRole cant setObject() to another UMLRole!, ignoring"<<endl;
		return;
	}

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

void UMLRole::init(UMLAssociation * parent, UMLObject * parentObj) {

	m_pObject = parentObj;
	m_Multi = "";
	m_Name = "";
	m_Visibility = Public;
	m_Changeability = chg_Changeable;

	// connect this up to parent 
	connect(this,SIGNAL(modified()),parent,SIGNAL(modified()));
}

#include "umlrole.moc"
