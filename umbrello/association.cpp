/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include <iostream.h>
#include "association.h"
#include "classifier.h"
#include "umldoc.h"
#include "umlrole.h"

// static members
const Uml::Association_Type UMLAssociation::atypeFirst = Uml::at_Generalization;
const Uml::Association_Type UMLAssociation::atypeLast = Uml::at_Activity;
const unsigned UMLAssociation::nAssocTypes = (unsigned)atypeLast - (unsigned)atypeFirst + 1;

// constructor
UMLAssociation::UMLAssociation(UMLDoc* parent) 
    : UMLObject((UMLObject *)parent) 
{
	init();
}

bool UMLAssociation::operator==(UMLAssociation &rhs) {
	if (this == &rhs) {
			return true;
	}
       return( UMLObject::operator==( rhs ) &&
                m_AssocType == rhs.m_AssocType &&
                m_Name == rhs.m_Name &&
                m_pRoleA == rhs.m_pRoleA &&
                m_pRoleB == rhs.m_pRoleB );
/*
	return( UMLObject::operator==( rhs ) &&
		m_AssocType == rhs.m_AssocType &&
		m_RoleAId == rhs.m_RoleAId &&
		m_RoleBId == rhs.m_RoleBId &&
		m_MultiA == rhs.m_MultiA &&
		m_MultiB == rhs.m_MultiB &&
		m_RoleNameA == rhs.m_RoleNameA &&
		m_RoleNameB == rhs.m_RoleNameB );
*/
}

const QString UMLAssociation::assocTypeStr[UMLAssociation::nAssocTypes] = {
	"generalization",	// at_Generalization
	"aggregation",		// at_Aggregation
	"dependency",		// at_Dependency
	"association",		// at_Association
	"associationself",	// at_Association_Self
	"collmessage",		// at_Coll_Message
	"seqmessage",		// at_Seq_Message
	"collmessageself",	// at_Coll_Message_Self
	"seqmessageself",	// at_Seq_Message_Self
	/*implementation no longer exists, but we keep it for compatibility*/
	"implementation",	// at_Implementation
	"composition",		// at_Composition
	"realization",		// at_Realization
	"uniassociation",	// at_UniAssociation
	"anchor",		// at_Anchor
	"state",		// at_State
	"activity", 		// at_Activity
};

//FIXME needed??
Uml::Association_Type UMLAssociation::toAssocType (QString atype) {
	for (unsigned i = 0; i < nAssocTypes; i++)
		if (atype == assocTypeStr[i])
			return (Uml::Association_Type)(i + (unsigned)atypeFirst);
	return at_Unknown;
}

Uml::Association_Type UMLAssociation::getAssocType() const {
	return m_AssocType;
}

QString UMLAssociation::toString (Uml::Association_Type atype) {
	if (atype < atypeFirst || atype > atypeLast)
		return "";
	return assocTypeStr[(unsigned)atype - (unsigned)atypeFirst];
}


bool UMLAssociation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement associationElement = qDoc.createElement( "UML:Association" );
	bool status = UMLObject::saveToXMI( qDoc, associationElement );

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
/*
// the future...
	getUMLRoleA()->saveToXMI (doc, associationElement);
	getUMLRoleB()->saveToXMI (doc, associationElement);
*/

	qElement.appendChild( associationElement );
	return status;
}

bool UMLAssociation::loadFromXMI( QDomElement & element ) {

	if( !UMLObject::loadFromXMI( element ) )
		return false;

	int atype = element.attribute( "assoctype", "-1" ).toInt();
	if (atype < (int)atypeFirst | atype > (int)atypeLast) {
		kdWarning() << "bad assoctype of UML:Association " << getID() << endl;
		return false;
	}
	setAssocType( (Uml::Association_Type)atype );

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

UMLObject* UMLAssociation::getObjectA() {
	return m_pRoleA->getObject();
}

UMLObject* UMLAssociation::getObjectB() {
	return m_pRoleB->getObject();
}

int UMLAssociation::getRoleAId() const {
	return m_pRoleA->getID();
}

int UMLAssociation::getRoleBId() const {
	return m_pRoleB->getID();
}

Changeability_Type UMLAssociation::getChangeabilityA() const {
	return m_pRoleA->getChangeability();
}

Changeability_Type UMLAssociation::getChangeabilityB() const {
	return m_pRoleB->getChangeability();
}

Scope UMLAssociation::getVisibilityA() const {
	return m_pRoleA->getVisibility();
}

Scope UMLAssociation::getVisibilityB() const {
	return m_pRoleB->getVisibility();
}

QString UMLAssociation::getMultiA() const {
	return m_pRoleA->getMultiplicity();
}

QString UMLAssociation::getMultiB() const {
	return m_pRoleB->getMultiplicity();
}

QString UMLAssociation::getRoleNameA() const {
	return m_pRoleA->getName();
}

QString UMLAssociation::getRoleADoc() const {
	return m_pRoleA->getDoc();
}

QString UMLAssociation::getRoleNameB() const {
	return m_pRoleB->getName();
}

QString UMLAssociation::getRoleBDoc() const {
	return m_pRoleB->getDoc();
}

UMLRole * UMLAssociation::getUMLRoleA() {
	return m_pRoleA;
}

UMLRole * UMLAssociation::getUMLRoleB() {
	return m_pRoleB;
}

void UMLAssociation::setAssocType(Uml::Association_Type assocType) {
	m_AssocType = assocType;
	emit modified();
}

void UMLAssociation::setRoleAId (int roleA) {
	m_pRoleA->setID(roleA);
}

void UMLAssociation::setRoleBId(int roleB) {
	m_pRoleB->setID(roleB);
}

void UMLAssociation::setObjectA(UMLObject *obj) {
	m_pRoleA->setObject(obj);
}

void UMLAssociation::setObjectB(UMLObject *obj) {
	m_pRoleB->setObject(obj);
}

void UMLAssociation::setVisibilityA(Scope value) {
	m_pRoleA->setVisibility(value);
}

void UMLAssociation::setVisibilityB(Scope value) {
	m_pRoleB->setVisibility(value);
}

void UMLAssociation::setChangeabilityA(Changeability_Type value) {
	m_pRoleA->setChangeability(value);
//	emit modified(); // not needed.. should originate from UMLRole
}

void UMLAssociation::setChangeabilityB(Changeability_Type value) {
	m_pRoleB->setChangeability(value);
}

void UMLAssociation::setMultiA(QString value) {
	m_pRoleA->setMultiplicity(value);
}

void UMLAssociation::setMultiB(QString value) {
	m_pRoleB->setMultiplicity(value);
}

void UMLAssociation::setRoleNameA(QString value) {
	m_pRoleA->setName(value);
}

void UMLAssociation::setRoleADoc(QString doc) {
	m_pRoleA->setDoc(doc);
}

void UMLAssociation::setRoleNameB(QString value) {
	m_pRoleB->setName(value);
}

void UMLAssociation::setRoleBDoc(QString doc) {
	m_pRoleB->setDoc(doc);
}

QString UMLAssociation::ChangeabilityToString(Uml::Changeability_Type type) {

	switch (type) {
	case Uml::chg_Frozen:
		return "frozen";
		break;
	case Uml::chg_AddOnly:
		return "addOnly";
		break;
	case Uml::chg_Changeable:
	default:
		return "changeable";
		break;
	}
}

QString UMLAssociation::ScopeToString(Uml::Scope scope) {

	switch (scope) {
	case Uml::Protected:
		return "#";
		break;
	case Uml::Private:
		return "-";
		break;
	case Uml::Public:
	default:
		return "+";
		break;
	}
}

void UMLAssociation::init() {
	m_AssocType = Uml::at_Unknown;
	m_BaseType = ot_Association;
	m_Name = "";

	m_pRoleA = new UMLRole (this);
	m_pRoleB = new UMLRole (this);

cerr<<" NEW ROLE ID A: "<<m_pRoleA->getID()<<endl;

/*
	m_pObjectA = 0;
	m_pObjectB = 0;
	m_RoleAId = -1;
	m_RoleBId = -1;
	m_MultiA = "";
	m_MultiB = "";
	m_RoleNameA = "";
	m_RoleNameB = "";
	m_VisibilityA = Public;
	m_VisibilityB = Public;
	m_ChangeabilityA = chg_Changeable;
	m_ChangeabilityB = chg_Changeable;
*/
}
#include "association.moc"
