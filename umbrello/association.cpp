/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "association.h"
#include "concept.h"
#include "umldoc.h"

UMLAssociation::UMLAssociation(UMLDoc* parent) : UMLObject((UMLObject *)parent) {
	init();
}

bool UMLAssociation::operator==(UMLAssociation &rhs) {
	if (this == &rhs) {
			return true;
	}
	return( UMLObject::operator==( rhs ) &&
		m_AssocType == rhs.m_AssocType &&
		m_RoleAId == rhs.m_RoleAId &&
		m_RoleBId == rhs.m_RoleBId &&
		m_MultiA == rhs.m_MultiA &&
		m_MultiB == rhs.m_MultiB &&
		m_RoleNameA == rhs.m_RoleNameA &&
		m_RoleNameB == rhs.m_RoleNameB );
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


bool UMLAssociation::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if(!status)
		return status;
	if(archive) {
		*s << Q_INT32(m_AssocType)
		   << m_RoleAId << m_RoleBId
		   << m_MultiA << m_MultiB
		   << m_RoleNameA << m_RoleNameB;
	} else {
		// QString atype;
		*s >> Q_INT32(m_AssocType)
		   >> m_RoleAId >> m_RoleBId
		   >> m_MultiA >> m_MultiB
		   >> m_RoleNameA >> m_RoleNameB;
		// m_AssocType = toAssocType(atype);
	}
	return status;
}

/** Returns the amount of bytes needed to serialize this object */
/* If the serialization method of this class is changed this function will have to be CHANGED TOO*/
/*This function is used by the Copy and Paste Functionality*/
/*The Size in bytes of a serialized QString Object is long sz:
		if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
		{
			sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
		}
	This calculation is valid only for QT 2.1.x or superior, this is totally incompatible with QT 2.0.x or QT 1.x or inferior
	That means the copy and paste functionality will work on with QT 2.1.x or superior
*/
long UMLAssociation::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();
	l_size += sizeof(Uml::Association_Type);  // m_AssocType
	l_size += sizeof(int);   // m_RoleA
	l_size += sizeof(int);   // m_RoleB

	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

// We assume we are working with QT 2.1.x or superior, that means
// if unicode returns a null pointer then the serialization process of the QString object
// will write a null marker 0xffffff, see QString::operator<< implementation

	if ( !m_MultiA.length() )
		l_size += sizeof(tmp);
	else
		l_size += (m_MultiA.length()*sizeof(QChar));

	if ( !m_MultiB.length() )
		l_size += sizeof(tmp);
	else
		l_size += (m_MultiB.length()*sizeof(QChar));

	if ( !m_RoleNameA.length() )
		l_size += sizeof(tmp);
	else
		l_size += (m_RoleNameA.length()*sizeof(QChar));

	if ( !m_RoleNameB.length() )
		l_size += sizeof(tmp);
	else
		l_size += (m_RoleNameB.length()*sizeof(QChar));

	return l_size;
}

bool UMLAssociation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
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
	associationElement.setAttribute( "visibilitya", getVisibilityA() );
	associationElement.setAttribute( "visibilityb", getVisibilityB() );
	associationElement.setAttribute( "changeabilitya", getChangeabilityA() );
	associationElement.setAttribute( "changeabilityb", getChangeabilityB() );
	qElement.appendChild( associationElement );
	return status;
}

bool UMLAssociation::loadFromXMI( QDomElement & element ) {

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
	return m_pObjectA;
}

UMLObject* UMLAssociation::getObjectB() {
	return m_pObjectB;
}

int UMLAssociation::getRoleAId() const {
	return m_RoleAId;
}

int UMLAssociation::getRoleBId() const {
	return m_RoleBId;
}

Changeability_Type UMLAssociation::getChangeabilityA() const {
	return m_ChangeabilityA;
}

Changeability_Type UMLAssociation::getChangeabilityB() const {
	return m_ChangeabilityB;
}

Scope UMLAssociation::getVisibilityA() const {
	return m_VisibilityA;
}

Scope UMLAssociation::getVisibilityB() const {
	return m_VisibilityB;
}

QString UMLAssociation::getMultiA() const {
	return m_MultiA;
}

QString UMLAssociation::getMultiB() const {
	return m_MultiB;
}

QString UMLAssociation::getRoleNameA() const {
	return m_RoleNameA;
}

QString UMLAssociation::getRoleNameB() const {
	return m_RoleNameB;
}

void UMLAssociation::setAssocType(Uml::Association_Type assocType) {
	m_AssocType = assocType;
}

void UMLAssociation::setRoleAId(int roleA) {
	m_RoleAId = roleA;
}

void UMLAssociation::setRoleBId(int roleB) {
	m_RoleBId = roleB;
}

void UMLAssociation::setObjectA(UMLObject *obj) {
	m_pObjectA = obj;
}

void UMLAssociation::setObjectB(UMLObject *obj) {
	m_pObjectB = obj;
}

void UMLAssociation::setVisibilityA(Scope value) {
	m_VisibilityA = value;
}

void UMLAssociation::setVisibilityB(Scope value) {
	m_VisibilityB = value;
}

void UMLAssociation::setChangeabilityA(Changeability_Type value) {
	m_ChangeabilityA = value;
}

void UMLAssociation::setChangeabilityB(Changeability_Type value) {
	m_ChangeabilityB = value;
}

void UMLAssociation::setMultiA(QString multiA) {
	m_MultiA = multiA;
}

void UMLAssociation::setMultiB(QString multiB) {
	m_MultiB = multiB;
}

void UMLAssociation::setRoleNameA(QString roleNameA) {
	m_RoleNameA = roleNameA;
}

void UMLAssociation::setRoleNameB(QString roleNameB) {
	m_RoleNameB = roleNameB;
}

static QString UMLAssociation::ChangeabilityToString(Uml::Changeability_Type type) {

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

static QString UMLAssociation::ScopeToString(Uml::Scope scope) {

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
	m_BaseType = ot_Association;
}
