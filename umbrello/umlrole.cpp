/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <qregexp.h>
#include "umlrole.h"
#include "association.h"
#include "umldoc.h"

// constructor
UMLRole::UMLRole(UMLAssociation * parent, UMLObject * parentObj, int roleID)
    : UMLObject((UMLObject *)parent)
{
	init(parent, parentObj, roleID);
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
	return m_pAssoc;
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

int UMLRole::getRoleID() {
	return m_roleID;
}

void UMLRole::init(UMLAssociation * parent, UMLObject * parentObj, int id) {

	m_roleID = id;
	m_pAssoc = parent;
	m_pObject = parentObj;
	m_Multi = "";
	m_Name = "";
	m_Visibility = Public;
	m_Changeability = chg_Changeable;

	// connect this up to parent 
	connect(this,SIGNAL(modified()),parent,SIGNAL(modified()));
}

bool UMLRole::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement roleElement = qDoc.createElement( "UML:AssociationEnd" );
	roleElement.setAttribute( "type", getID() );
	if (m_Multi != "")
		roleElement.setAttribute("multiplicity", m_Multi);
	if (m_Name != "")
		roleElement.setAttribute("name", m_Name);
	if (m_Doc != "")
		roleElement.setAttribute("comment", m_Doc);
	if (m_roleID) {  // role A
		switch (m_pAssoc->getAssocType()) {
			case Uml::at_Composition:
				roleElement.setAttribute("aggregation", "composite");
				break;
			case Uml::at_Aggregation:
				roleElement.setAttribute("aggregation", "shared");
				break;
			default:
				roleElement.setAttribute("aggregation", "none");
				break;
		}
	} else {
		if (m_pAssoc->getAssocType() == Uml::at_UniAssociation)
			roleElement.setAttribute("isNavigable", "true");
	}
	switch (m_Visibility) {
		case Uml::Private:
			roleElement.setAttribute("visibility", "private");
			break;
		case Uml::Protected:
			roleElement.setAttribute("visibility", "protected");
			break;
		case Uml::Public:
			roleElement.setAttribute("visibility", "public");
			break;
	}
	switch (m_Changeability) {
		case Uml::chg_Frozen:
			roleElement.setAttribute("changeable", "frozen");
			break;
		case Uml::chg_AddOnly:
			roleElement.setAttribute("changeable", "addOnly");
			break;
		case Uml::chg_Changeable:
			// This is the default.
			// roleElement.setAttribute("changeable", "none");
			break;
	}
	qElement.appendChild( roleElement );

	return true;
}

bool UMLRole::loadFromXMI( QDomElement & element ) {
	UMLDoc * doc = (UMLDoc*)(m_pAssoc->parent());
	if (doc == NULL) {
		kdError() << "UMLRole::loadFromXMI failed to retrieve the UMLDoc"
			  << endl;
		return false;
	}
	QString idStr = element.attribute("type", "-1");
	if (idStr == "-1") {
		kdError() << "UMLRole::loadFromXMI: type not given or illegal" << endl;
		return false;
	}
	UMLObject * obj;
	if (idStr.contains(QRegExp("\\D")))
		obj = doc->findObjectByIdStr(idStr);
	else
		obj = doc->findUMLObject(idStr.toInt());
	if (obj == NULL) {
		kdError() << "UMLRole::loadFromXMI: cannot find object of ID "
			  << idStr << endl;
		return false;
	}
	m_pObject = obj;

	// Here comes the handling of the association type.
	// This is open for discussion - I'm pretty sure there are better ways..
	if (m_roleID) {  // role A
		QString aggregation = element.attribute("aggregation", "none");
		if (aggregation == "composite")
			m_pAssoc->setAssocType(Uml::at_Composition);
		else if (aggregation == "shared")
			m_pAssoc->setAssocType(Uml::at_Aggregation);
		else
			m_pAssoc->setAssocType(Uml::at_Association);
	} else if (element.hasAttribute("isNavigable")) {
		/* Role B:
		   If isNavigable is not given, we make no change to the
		   association type.
		   If isNavigable is given, and is "true", then we assume that
		   the association's other end (role A) is not navigable, and
		   therefore we change the association type to UniAssociation.
		   The case that isNavigable is given as "false" is ignored.
		   Combined with the association type logic for role A, this
		   allows us to support at_Association and at_UniAssociation.
		 */
		if (element.attribute("isNavigable") == "true")
			m_pAssoc->setAssocType(Uml::at_UniAssociation);
	}

	m_Multi = element.attribute("multiplicity", "");
	m_Name = element.attribute("name", "");
	if (element.hasAttribute("documentation"))  // for bkwd compat.
		m_Doc = element.attribute("documentation", "");
	else
		m_Doc = element.attribute("comment", "");

	// visibilty defaults to Public if it cant set it here..
        m_Visibility = Uml::Public;
        QString vis = element.attribute("visibility", "public");
	if (vis == "private")
		m_Visibility = Uml::Private;
	else if (vis == "protected")
		m_Visibility = Uml::Protected;

	// Changeability defaults to Changeable if it cant set it here..
	m_Changeability = Uml::chg_Changeable;
        QString changeable = element.attribute("changeable", "none");
        if (changeable == "frozen")
		m_Changeability = Uml::chg_Frozen;
        else if (changeable == "addOnly")
		m_Changeability = Uml::chg_AddOnly;

	return true;
}

#include "umlrole.moc"
