/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <kdebug.h>

#include "association.h"
#include "concept.h"
#include "umldoc.h"

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
	"implementation",	// at_Implementation
	"composition",		// at_Composition
	"realization",		// at_Realization
	"uniassociation",	// at_UniAssociation
	"anchor",		// at_Anchor
	"state",		// at_State
	"activity", 		// at_Activity
};

Uml::Association_Type UMLAssociation::toAssocType (QString atype) {
	for (unsigned i = 0; i < nAssocTypes; i++)
		if (atype == assocTypeStr[i])
			return (Uml::Association_Type)(i + (unsigned)atypeFirst);
	return at_Unknown;
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
		   << m_RoleA << m_RoleB
		   << m_MultiA << m_MultiB
		   << m_NameA << m_NameB;
	} else {
		// QString atype;
		*s >> Q_INT32(m_AssocType)
		   >> m_RoleA >> m_RoleB
		   >> m_MultiA >> m_MultiB
		   >> m_NameA >> m_NameB;
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

	if ( !m_NameA.length() )
		l_size += sizeof(tmp);
	else
		l_size += (m_NameA.length()*sizeof(QChar));

	if ( !m_NameB.length() )
		l_size += sizeof(tmp);
	else
		l_size += (m_NameB.length()*sizeof(QChar));

	return l_size;
}

bool UMLAssociation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement associationElement = qDoc.createElement( "UML:Association" );
	bool status = UMLObject::saveToXMI( qDoc, associationElement );
	associationElement.setAttribute( "assoctype", toString(m_AssocType) );
	associationElement.setAttribute( "rolea", m_RoleA );
	associationElement.setAttribute( "roleb", m_RoleB );
	associationElement.setAttribute( "multia", m_MultiA );
	associationElement.setAttribute( "multib", m_MultiB );
	associationElement.setAttribute( "namea", m_NameA );
	associationElement.setAttribute( "nameb", m_NameB );
	qElement.appendChild( associationElement );
	return status;
}

bool UMLAssociation::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI( element ) )
		return false;
        QString atype = element.attribute( "assoctype", "" );
	m_AssocType = toAssocType(atype);
	QString AId_str = element.attribute( "rolea", "-1" );
	QString BId_str = element.attribute( "roleb", "-1" );
	m_RoleA = AId_str.toInt();
	m_RoleB = BId_str.toInt();
	m_MultiA = element.attribute( "multia", "" );
	m_MultiB = element.attribute( "multib", "" );
	m_NameA = element.attribute( "namea", "" );
	m_NameB = element.attribute( "nameb", "" );

	((UMLDoc*)parent())->addAssocToConcepts(this);
	return true;
}



