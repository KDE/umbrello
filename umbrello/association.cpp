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
#include <qregexp.h>

#include "association.h"
#include "classifier.h"
#include "uml.h"
#include "umldoc.h"
#include "umlrole.h"

// static members
const Uml::Association_Type UMLAssociation::atypeFirst = Uml::at_Generalization;
const Uml::Association_Type UMLAssociation::atypeLast = Uml::at_Activity;
const unsigned UMLAssociation::nAssocTypes = (unsigned)atypeLast -
					     (unsigned)atypeFirst + 1;

// constructor
UMLAssociation::UMLAssociation( Association_Type type,
				UMLObject * roleA, UMLObject * roleB )
    : UMLObject("", -1)
{
	init(type, roleA, roleB);
}

// destructor
UMLAssociation::~UMLAssociation( ) {
	// delete ourselves from the parent document
	UMLApp::app()->getDocument()->removeAssociation(this);
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
	/* The elements must be listed in the same order as in the
	   Uml::Association_Type.  */
	i18n("Generalization"),			// at_Generalization
	i18n("Aggregation"),			// at_Aggregation
	i18n("Dependency"),			// at_Dependency
	i18n("Association"),			// at_Association
	i18n("Self Association"),		// at_Association_Self
	i18n("Collaboration Message"),		// at_Coll_Message
	i18n("Sequence Message"),		// at_Seq_Message
	i18n("Collaboration Self Message"),	// at_Coll_Message_Self
	i18n("Sequence Self Message"),		// at_Seq_Message_Self
	i18n("Containment"),			// at_Containment
	i18n("Composition"),			// at_Composition
	i18n("Realization"),			// at_Realization
	i18n("Uni Association"),		// at_UniAssociation
	i18n("Anchor"),				// at_Anchor
	i18n("State Transition"),		// at_State
	i18n("Activity"), 			// at_Activity
};

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
	string += ":" + typeAsString(m_AssocType) + ":";
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
		atype == Uml::at_UniAssociation ||
		atype == Uml::at_Aggregation ||
		atype == Uml::at_Composition);
}

void UMLAssociation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	if (m_AssocType == Uml::at_Generalization ||
	    m_AssocType == Uml::at_Realization) {
		QDomElement assocElement = UMLObject::save("UML:Generalization", qDoc);
		assocElement.setAttribute( "child", getRoleAId() );
		assocElement.setAttribute( "parent", getRoleBId() );
		qElement.appendChild( assocElement );
		return;
	}
	QDomElement associationElement = UMLObject::save("UML:Association", qDoc);
	QDomElement connElement = qDoc.createElement("UML:Association.connection");
	getUMLRoleA()->saveToXMI (qDoc, connElement);
	getUMLRoleB()->saveToXMI (qDoc, connElement);
	associationElement.appendChild (connElement);
	qElement.appendChild( associationElement );
}

bool UMLAssociation::load( QDomElement & element ) {
	if (getID() == -1)
		return false; // old style XMI file. No real info in this association.

	UMLDoc * doc = UMLApp::app()->getDocument();
	if (m_AssocType == Uml::at_Generalization) {

		QString roleAIdStr = element.attribute( "child", "-1" );
		if (roleAIdStr == "-1") {
			kdError() << "UMLAssociation::load: "
				  << "child not given or illegal" << endl;
			return false;
		}
		QString roleBIdStr = element.attribute( "parent", "-1" );
		if (roleBIdStr == "-1") {
			kdError() << "UMLAssociation::load: "
				  << "parent not given or illegal" << endl;
			return false;
		}

		// set umlobjects of roles
		UMLObject *objA, *objB;
		if (roleAIdStr.contains(QRegExp("\\D")))
			objA = doc->findObjectByIdStr(roleAIdStr);
		else
			objA = doc->findUMLObject(roleAIdStr.toInt());
		if (objA == NULL) {
			kdError() << "UMLAssociation::load: "
				  << "cannot find child object " << roleAIdStr << endl;
			return false;
		}
		getUMLRoleA()->setObject(objA);

		if (roleBIdStr.contains(QRegExp("\\D")))
			objB = doc->findObjectByIdStr(roleBIdStr);
		else
			objB = doc->findUMLObject(roleBIdStr.toInt());
		if (objB == NULL) {
			kdError() << "UMLAssociation::load: "
				  << "cannot find parent object " << roleBIdStr << endl;
			return false;
		}
		getUMLRoleB()->setObject(objB);

		// setting the association type:
                //
                // In the old days, we could just record this on the association,
                // and be done with it. But thats not how the UML13.dtd does things.
                // As a result, we are checking roleA for information about the
                // parent association (!) which by this point in the parse, should
                // be set. However, the information that the roles are allowed to have
                // is not complete, so we need to finish the analysis here.

/*
		if (objA->getBaseType() == Uml::ot_Class &&
		    objB->getBaseType() == Uml::ot_Interface)
*/
		// its a realization if either endpoint is an interface
		if (objA->getBaseType() == Uml::ot_Interface ||
		    objB->getBaseType() == Uml::ot_Interface)
			m_AssocType = Uml::at_Realization;

		return true;
	}

	QDomNode node = element.firstChild();
	if (!node.isNull()) {
		QDomElement tempElement = node.toElement();
		// uml13.dtd compliant format (new style)
		if (tempElement.isNull()) {
			kdWarning() << "UMLAssociation::load: "
				<< "expecting UML:Association.connection" << endl;
			return false;
		}
		QString tag = tempElement.tagName();
		if (!tagEq(tag, "Association.connection") &&
		    !tagEq(tag, "Namespace.ownedElement") &&
		    !tagEq(tag, "Namespace.contents")) {
			kdWarning() << "UMLAssociation::load: "
				    << "unknown child node " << tag << endl;
			return false;
		}
		// Load role A.
		node = tempElement.firstChild();
		tempElement = node.toElement();
		if (tempElement.isNull()) {
			kdWarning() << "UML:Association : element (A) is Null" << endl;
			return false;
		}
		tag = tempElement.tagName();
		if (!tagEq(tag, "AssociationEndRole") &&
		    !tagEq(tag, "AssociationEnd")) {
			kdWarning() << "UMLAssociation::load: "
				    << "unknown child (A) tag " << tag << endl;
			return false;
		}
		if (! getUMLRoleA()->loadFromXMI(tempElement, false)) // do not load xmi.id
			return false;
		// Load role B.
		node = node.nextSibling();
		tempElement = node.toElement();
		if (tempElement.isNull()) {
			kdWarning() << "UML:Association : element (B) is Null" << endl;
			return false;
		}
		tag = tempElement.tagName();
		if (!tagEq(tag, "AssociationEndRole") &&
		    !tagEq(tag, "AssociationEnd")) {
			kdWarning() << "UMLAssociation::load: "
				    << "unknown child (B) tag " << tag << endl;
			return false;
		}
		if (! getUMLRoleB()->loadFromXMI(tempElement, false)) // do not load xmi.id
			return false;

		// setting the association type:
		//
		// In the old days, we could just record this on the association,
		// and be done with it. But thats not how the UML13.dtd does things.
		// As a result, we are checking roleA for information about the
		// parent association (!) which by this point in the parse, should
		// be set. However, the information that the roles are allowed to have
		// is not complete, so we need to finish the analysis here.

		// find self-associations
		if(getAssocType() == Uml::at_Association && getRoleAId() == getRoleBId())
			m_AssocType = Uml::at_Association_Self;

		// fall-back default type
		if(getAssocType() == Uml::at_Unknown)
		{
			m_AssocType = Uml::at_Association;
			// Q: is this truely a warning condition? Do state diagrams store
			// stuff this way (for example)?
			kdWarning()<<" Warning: load can't determine association type, setting to 'plain' association"<<endl;
		}

		return true;
	}

	// From here on it's old-style stuff.
	QString assocTypeStr = element.attribute( "assoctype", "-1" );
	Uml::Association_Type assocType = Uml::at_Unknown;
	int assocTypeNum = assocTypeStr.toInt();
	if (assocTypeNum < (int)atypeFirst || assocTypeNum > (int)atypeLast) {
		kdWarning() << "bad assoctype of UML:Association " << getID() << endl;
		return false;
	}
	assocType = (Uml::Association_Type)assocTypeNum;
	setAssocType( assocType );

	int roleAObjID = element.attribute( "rolea", "-1" ).toInt();
	int roleBObjID = element.attribute( "roleb", "-1" ).toInt();
	if (assocType == at_Aggregation || assocType == at_Composition) {
		// Flip roles to compensate for changed diamond logic in LinePath.
		// For further explanations see AssociationWidget::loadFromXMI.
		int tmp = roleAObjID;
		roleAObjID = roleBObjID;
		roleBObjID = tmp;
	}

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

        // Visibility defaults to Public if it cant set it here..
        QString visibilityA = element.attribute( "visibilitya", "0");
        QString visibilityB = element.attribute( "visibilityb", "0");
        if (visibilityA.toInt() > 0)
                setVisibilityA( (Scope) visibilityA.toInt());
        if (visibilityB.toInt() > 0)
                setVisibilityB( (Scope) visibilityB.toInt());

        // Changeability defaults to Changeable if it cant set it here..
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
	if(m_AssocType == at_UniAssociation)
	{
		// In this case we need to auto-set the multiplicity/rolenames
		// of the roles
		kdDebug() << " A new uni-association has been created." << endl;
	}
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
	nrof_parent_widgets = -1;

	m_pRoleA = new UMLRole (this, roleAObj, 1);
	m_pRoleB = new UMLRole (this, roleBObj, 0);
}


#include "association.moc"
