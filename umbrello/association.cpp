 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

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
#include "model_utils.h"

using namespace Uml;

// static members
const Uml::Association_Type UMLAssociation::atypeFirst = Uml::at_Generalization;
const Uml::Association_Type UMLAssociation::atypeLast = Uml::at_Activity;
const unsigned UMLAssociation::nAssocTypes = (unsigned)atypeLast -
					     (unsigned)atypeFirst + 1;

// constructor
UMLAssociation::UMLAssociation( Association_Type type,
				UMLObject * roleA, UMLObject * roleB )
    : UMLObject("")
{
	init(type, roleA, roleB);
}

UMLAssociation::UMLAssociation( Association_Type type /* = Uml::at_Unknown */)
    : UMLObject("", "0")
{
	init(type, NULL, NULL);
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
	return ( UMLObject::operator== ( rhs ) &&
		m_AssocType == rhs.m_AssocType &&
		m_Name == rhs.m_Name &&
		m_pRole[A] == rhs.m_pRole[A] &&
		m_pRole[B] == rhs.m_pRole[B] );
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
	if(m_pRole[A])
	{
		string += m_pRole[A]->getObject()->getName();
		string += ":";
		string += m_pRole[A]->getName();
	}
	string += ":" + typeAsString(m_AssocType) + ":";
	if(m_pRole[B])
	{
		string += m_pRole[B]->getObject( )->getName();
		string += ":";
		string += m_pRole[B]->getName();
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

bool UMLAssociation::resolveRef() {
	bool successA = getUMLRole(A)->resolveRef();
	bool successB = getUMLRole(B)->resolveRef();
	if (successA && successB) {
		UMLObject *objA = getUMLRole(A)->getObject();
		UMLObject *objB = getUMLRole(B)->getObject();
		if (objA && objA->getBaseType() == Uml::ot_Interface ||
		    objB && objB->getBaseType() == Uml::ot_Interface)
			m_AssocType = Uml::at_Realization;
		return true;
	}
	return false;
}

void UMLAssociation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	if (m_AssocType == Uml::at_Generalization ||
	    m_AssocType == Uml::at_Realization) {
		QDomElement assocElement = UMLObject::save("UML:Generalization", qDoc);
		assocElement.setAttribute( "child", ID2STR(getRoleId(A)) );
		assocElement.setAttribute( "parent", ID2STR(getRoleId(B)) );
		qElement.appendChild( assocElement );
		return;
	}
	QDomElement associationElement = UMLObject::save("UML:Association", qDoc);
	QDomElement connElement = qDoc.createElement("UML:Association.connection");
	getUMLRole(A)->saveToXMI (qDoc, connElement);
	getUMLRole(B)->saveToXMI (qDoc, connElement);
	associationElement.appendChild (connElement);
	qElement.appendChild( associationElement );
}

bool UMLAssociation::load( QDomElement & element ) {
	if (getID() == Uml::id_None)
		return false; // old style XMI file. No real info in this association.

	UMLDoc * doc = UMLApp::app()->getDocument();
	UMLObject * obj[2] = { NULL, NULL };
	if (m_AssocType == Uml::at_Generalization ||
	    m_AssocType == Uml::at_Dependency) {
		for (unsigned r = Uml::A; r <= Uml::B; r++) {
			const QString fetch = (m_AssocType == Uml::at_Generalization ?
						 r == Uml::A ? "child" : "parent"
					       : r == Uml::A ? "client" : "supplier");
			QString roleIdStr = element.attribute(fetch, "");
			if (roleIdStr.isEmpty()) {
				// Might be given as a child node instead - see below.
				continue;
			}

			// set umlobject of role if possible (else defer resolution)
			obj[r] = doc->findObjectById(STR2ID(roleIdStr));
			Uml::Role_Type role = (Uml::Role_Type)r;
			if (obj[r] == NULL) {
				getUMLRole(role)->setIdStr(roleIdStr);  // defer to resolveRef()
			} else {
				getUMLRole(role)->setObject(obj[r]);
			}
		}
		if (obj[A] == NULL || obj[B] == NULL) {
			for (QDomNode node = element.firstChild(); !node.isNull();
			     node = node.nextSibling()) {
				if (node.isComment())
					continue;
				QDomElement tempElement = node.toElement();
				QString tag = tempElement.tagName();
				if (Umbrello::isCommonXMIAttribute(tag))
					continue;
				bool isGeneralization = (m_AssocType == Uml::at_Generalization &&
							 (tagEq(tag, "child") || tagEq(tag, "parent")));
				bool isDependency = (m_AssocType == Uml::at_Dependency &&
						     (tagEq(tag, "client") || tagEq(tag, "supplier")));
				if (!isGeneralization && !isDependency)
					continue;
				QString idStr = tempElement.attribute( "xmi.id", "" );
				if (idStr.isEmpty())
					idStr = tempElement.attribute( "xmi.idref", "" );
				if (idStr.isEmpty()) {
					QDomNode inner = node.firstChild();
					QDomElement tmpElem = inner.toElement();
					idStr = tmpElem.attribute( "xmi.id", "" );
					if (idStr.isEmpty())
						idStr = tmpElem.attribute( "xmi.idref", "" );
				}
				if (idStr.isEmpty()) {
					kdError() << "UMLAssociation::load (type " << m_AssocType
					  << ", id " << ID2STR(getID()) << "): "
					  << "xmi id not given for " << tag << endl;
					continue;
				}
				// Since we know for sure that we're dealing with a non
				// umbrello file, use deferred resolution unconditionally.
				if (tagEq(tag, "child") || tagEq(tag, "client")) {
					getUMLRole(A)->setIdStr(idStr);
				} else {
					getUMLRole(B)->setIdStr(idStr);
				}
			}
		}

		// setting the association type:
                //
                // In the old days, we could just record this on the association,
                // and be done with it. But thats not how the UML13.dtd does things.
                // As a result, we are checking roleA for information about the
                // parent association (!) which by this point in the parse, should
                // be set. However, the information that the roles are allowed to have
                // is not complete, so we need to finish the analysis here.

		// its a realization if either endpoint is an interface
		if (obj[A] && obj[A]->getBaseType() == Uml::ot_Interface ||
		    obj[B] && obj[B]->getBaseType() == Uml::ot_Interface)
			m_AssocType = Uml::at_Realization;

		return true;
	}

	for (QDomNode node = element.firstChild(); !node.isNull();
	     node = node.nextSibling()) {
		// uml13.dtd compliant format (new style)
		if (node.isComment())
			continue;
		QDomElement tempElement = node.toElement();
		QString tag = tempElement.tagName();
		if (Umbrello::isCommonXMIAttribute(tag))
			continue;
		if (!tagEq(tag, "Association.connection") &&
		    !tagEq(tag, "Namespace.ownedElement") &&
		    !tagEq(tag, "Namespace.contents")) {
			kdWarning() << "UMLAssociation::load: "
				    << "unknown child node " << tag << endl;
			return false;
		}
		// Load role A.
		node = tempElement.firstChild();
		while (node.isComment())
			node = node.nextSibling();
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
		if (! getUMLRole(A)->loadFromXMI(tempElement, false)) // do not load xmi.id
			return false;
		// Load role B.
		node = node.nextSibling();
		while (node.isComment())
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
		if (! getUMLRole(B)->loadFromXMI(tempElement, false)) // do not load xmi.id
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
		if(getAssocType() == Uml::at_Association && getRoleId(A) == getRoleId(B))
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
	if (assocTypeStr[0] >= 'a' && assocTypeStr[0] <= 'z') {
		// In an earlier version, the natural assoctype names were saved.
		const unsigned nAssocTypes = 16;
		const QString assocTypeString[nAssocTypes] = {
			"generalization",	// at_Generalization
			"aggregation",		// at_Aggregation
			"dependency",		// at_Dependency
			"association",		// at_Association
			"associationself",	// at_Association_Self
			"collmessage",		// at_Coll_Message
			"seqmessage",		// at_Seq_Message
			"collmessageself",	// at_Coll_Message_Self
			"seqmessageself",	// at_Seq_Message_Self
			"implementation",	// at_Implementation
			"composition",		// at_Composition
			"realization",		// at_Realization
			"uniassociation",	// at_UniAssociation
			"anchor",		// at_Anchor
			"state",		// at_State
			"activity" 		// at_Activity
		};

		unsigned index;
		for (index = 0; index < nAssocTypes; index++)
			if (assocTypeStr == assocTypeString[index])
				break;
		if (index < nAssocTypes)
			assocType = (Uml::Association_Type)index;
	} else {
		int assocTypeNum = assocTypeStr.toInt();
		if (assocTypeNum < (int)atypeFirst || assocTypeNum > (int)atypeLast) {
			kdWarning() << "bad assoctype of UML:Association "
			            << ID2STR(getID()) << endl;
			return false;
		}
		assocType = (Uml::Association_Type)assocTypeNum;
	}
	setAssocType( assocType );

	Uml::IDType roleAObjID = STR2ID(element.attribute( "rolea", "-1" ));
	Uml::IDType roleBObjID = STR2ID(element.attribute( "roleb", "-1" ));
	if (assocType == at_Aggregation || assocType == at_Composition) {
		// Flip roles to compensate for changed diamond logic in LinePath.
		// For further explanations see AssociationWidget::loadFromXMI.
		Uml::IDType tmp = roleAObjID;
		roleAObjID = roleBObjID;
		roleBObjID = tmp;
	}

	UMLObject * objA = doc->findObjectById(roleAObjID);
	UMLObject * objB = doc->findObjectById(roleBObjID);

	if(objA)
		getUMLRole(A)->setObject(objA);
	else
		return false;

	if(objB)
		getUMLRole(B)->setObject(objB);
	else
		return false;

	setMulti(element.attribute( "multia", "" ), A);
	setMulti(element.attribute( "multib", "" ), B);

	setRoleName(element.attribute( "namea", "" ), A);
	setRoleName(element.attribute( "nameb", "" ), B);

	setRoleDoc(element.attribute( "doca", "" ), A);
	setRoleDoc(element.attribute( "docb", "" ), B);

        // Visibility defaults to Public if it cant set it here..
        QString visibilityA = element.attribute( "visibilitya", "0");
        QString visibilityB = element.attribute( "visibilityb", "0");
        if (visibilityA.toInt() > 0)
                setVisibility( (Scope) visibilityA.toInt(), A);
        if (visibilityB.toInt() > 0)
                setVisibility( (Scope) visibilityB.toInt(), B);

        // Changeability defaults to Changeable if it cant set it here..
        QString changeabilityA = element.attribute( "changeabilitya", "0");
        QString changeabilityB = element.attribute( "changeabilityb", "0");
        if (changeabilityA.toInt() > 0)
                setChangeability ( (Changeability_Type) changeabilityA.toInt(), A);
        if (changeabilityB.toInt() > 0)
                setChangeability ( (Changeability_Type) changeabilityB.toInt(), B);

	return true;
}

UMLObject* UMLAssociation::getObject(Role_Type role) {
	return m_pRole[role]->getObject();
}

Uml::IDType UMLAssociation::getRoleId(Role_Type role) const {
	return m_pRole[role]->getID();
}

Changeability_Type UMLAssociation::getChangeability(Role_Type role) const {
	return m_pRole[role]->getChangeability();
}

Scope UMLAssociation::getVisibility(Role_Type role) const {
	return m_pRole[role]->getVisibility();
}

QString UMLAssociation::getMulti(Role_Type role) const {
	return m_pRole[role]->getMultiplicity();
}

QString UMLAssociation::getRoleName(Role_Type role) const {
	return m_pRole[role]->getName();
}

QString UMLAssociation::getRoleDoc(Role_Type role) const {
	return m_pRole[role]->getDoc();
}

UMLRole * UMLAssociation::getUMLRole(Role_Type role) {
	return m_pRole[role];
}

void UMLAssociation::setAssocType(Uml::Association_Type assocType) {
	m_AssocType = assocType;
	if(m_AssocType == at_UniAssociation)
	{
		// In this case we need to auto-set the multiplicity/rolenames
		// of the roles
#ifdef VERBOSE_DEBUGGING
		kdDebug() << " A new uni-association has been created." << endl;
#endif
	}
	emit modified();
}

void UMLAssociation::setObject(UMLObject *obj, Role_Type role) {
	m_pRole[role]->setObject(obj);
}

void UMLAssociation::setVisibility(Scope value, Role_Type role) {
	m_pRole[role]->setVisibility(value);
}

void UMLAssociation::setChangeability(Changeability_Type value, Role_Type role) {
	m_pRole[role]->setChangeability(value);
}

void UMLAssociation::setMulti(const QString &value, Role_Type role) {
	m_pRole[role]->setMultiplicity(value);
}

void UMLAssociation::setRoleName(const QString &value, Role_Type role) {
	m_pRole[role]->setName(value);
}

void UMLAssociation::setRoleDoc(const QString &doc, Role_Type role) {
	m_pRole[role]->setDoc(doc);
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

	m_pRole[Uml::A] = new UMLRole (this, roleAObj, Uml::A);
	m_pRole[Uml::B] = new UMLRole (this, roleBObj, Uml::B);
}


#include "association.moc"
