/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <kdebug.h>
#include <klocale.h>

#include "association.h"
#include "classifier.h"
#include "umldoc.h"
#include "umlrole.h"

// static members
const Uml::Association_Type UMLAssociation::atypeFirst = Uml::at_Generalization;
const Uml::Association_Type UMLAssociation::atypeLast = Uml::at_Activity;
const unsigned UMLAssociation::nAssocTypes = (unsigned)atypeLast -
					     (unsigned)atypeFirst + 1;

// constructor
UMLAssociation::UMLAssociation( UMLDoc* parent,
				Association_Type type,
				UMLObject * roleA, UMLObject * roleB )
    : UMLObject(parent, "", -1) 
{
	init(type, roleA, roleB);
}

// destructor
UMLAssociation::~UMLAssociation( ) { 
	// delete ourselves from the parent document
	((UMLDoc *)parent())->removeAssociation(this);
}

bool UMLAssociation::operator==(UMLAssociation &rhs) {
	if (this == &rhs) {
			return true;
	}
       return( UMLObject::operator== ( rhs ) &&
                m_AssocType == rhs.m_AssocType &&
                m_Name == rhs.m_Name &&
                m_pRoleA == rhs.m_pRoleA &&
                m_pRoleB == rhs.m_pRoleB );
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

// Required by loadFromXMI(): In an early version of saveToXMI(),
// the assoctype natural names were saved instead of the numbers.
Uml::Association_Type UMLAssociation::toAssocType (QString atype) {
	for (unsigned i = 0; i < nAssocTypes; i++)
		if (atype == assocTypeStr[i])
			return (Uml::Association_Type)(i + (unsigned)atypeFirst);
	return at_Unknown;
}

Uml::Association_Type UMLAssociation::getAssocType() const {
	return m_AssocType;
}

QString UMLAssociation::toString ( ) const
{
	QString string;
	if(m_pRoleA) 
	{
		string += m_pRoleA->getObject( )->getName();
		string += ":";
		string += m_pRoleA->getName();
	}
	string += ":";
	switch(m_AssocType)
	{
	case at_Generalization:
		string += i18n("Generalization");
		break;
	case at_Aggregation:
		string += i18n("Aggregation");
		break;
	case at_Dependency:
		string += i18n("Dependency");
		break;
	case at_Association:
		string += i18n("Association");
		break;
	case at_Association_Self:
		string += i18n("Self Association");
		break;
	case at_Anchor:
		string += i18n("Anchor");
		break;
	case at_Realization:
		string += i18n("Realization");
		break;
	case at_Composition:
		string += i18n("Composition");
		break;
	case at_UniAssociation:
		string += i18n("Uni Association");
		break;
	case at_Implementation:
		string += i18n("Implementation");
		break;
	case at_State:
		string += i18n("State Transition");
		break;
	default:
		string += i18n("Other Type");
		break;
	} //end switch
	string += ":";
	if(m_pRoleB) 
	{
		string += m_pRoleB->getObject( )->getName();
		string += ":";
		string += m_pRoleB->getName();
	}
	return string;
}

QString UMLAssociation::typeAsString (Uml::Association_Type atype)
{
	if (atype < atypeFirst || atype > atypeLast)
		return "";
	return assocTypeStr[(unsigned)atype - (unsigned)atypeFirst];
}

bool UMLAssociation::assocTypeHasUMLRepresentation(Uml::Association_Type atype)
{
	return (atype == Uml::at_Generalization ||
		atype == Uml::at_Realization ||
		atype == Uml::at_Association_Self ||
		atype == Uml::at_Aggregation ||
		atype == Uml::at_Composition);
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

	if (getID() == -1)
		return false; // old style XMI file. No real info in this association.

	QString assocTypeStr = element.attribute( "assoctype", "-1" );
	Uml::Association_Type assocType = Uml::at_Unknown;
	if (assocTypeStr[0] >= 'a' && assocTypeStr[0] <= 'z') {
		// In an earlier version, the natural assoctype names were saved.
		assocType = toAssocType( assocTypeStr );
	} else {
		int assocTypeNum = assocTypeStr.toInt();
		if (assocTypeNum < (int)atypeFirst | assocTypeNum > (int)atypeLast) {
			kdWarning() << "bad assoctype of UML:Association " << getID() << endl;
			return false;
		}
		assocType = (Uml::Association_Type)assocTypeNum;
	}
	setAssocType( assocType );

	int roleAObjID = element.attribute( "rolea", "-1" ).toInt();
	int roleBObjID = element.attribute( "roleb", "-1" ).toInt();

	UMLDoc * doc = ((UMLDoc*)parent());
	UMLObject * objA = doc->findUMLObject(roleAObjID);
	UMLObject * objB = doc->findUMLObject(roleBObjID);

	if(objA)
		getUMLRoleA()->setObject(objA);
	else
		return false;

	if(objB)
		getUMLRoleB()->setObject(objB);
	else
		return false;

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

void UMLAssociation::init(Association_Type type, UMLObject *roleAObj, UMLObject *roleBObj) {

	m_AssocType = type;
	m_BaseType = ot_Association;
	m_Name = "";
	nrof_parent_widgets = 0;

	m_pRoleA = new UMLRole (this, roleAObj, 1);
	m_pRoleB = new UMLRole (this, roleBObj, 0);

}

#include "association.moc"
