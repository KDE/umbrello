/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qglobal.h>
#include <qdatastream.h>

#include <kdebug.h>
#include <klocale.h>

#include "floatingtextdata.h"
#include "linepath.h"
#include "associationwidgetdata.h"

AssociationWidgetData::AssociationWidgetData() {
	m_nIndexA = m_nIndexB = 1;
	m_nTotalCountA = m_nTotalCountB = 2;
	m_pMultiDataB = 0;
	m_pMultiDataA = 0;
	m_pRoleData = 0;
	m_nWidgetAID = -1;
	m_nWidgetBID = -1;
	m_AssocType = Uml::at_Association;
}

AssociationWidgetData::~AssociationWidgetData() {}

AssociationWidgetData::AssociationWidgetData(AssociationWidgetData & Other) {
	*this = Other;
}

AssociationWidgetData & AssociationWidgetData::operator=(AssociationWidgetData & Other) {
	m_AssocType = Other.m_AssocType;
	m_LinePath = Other.m_LinePath ;
	m_nWidgetAID = Other.m_nWidgetAID;
	m_nWidgetBID = Other.m_nWidgetBID;
	m_nIndexA = Other.m_nIndexA;
	m_nIndexB = Other.m_nIndexB;
	m_nTotalCountA = Other.m_nTotalCountA;
	m_nTotalCountB = Other.m_nTotalCountB;

	m_pMultiDataA = new FloatingTextData();
	if( Other.m_pMultiDataA )
		*(m_pMultiDataA) = *(Other.m_pMultiDataA);

	m_pMultiDataB = new FloatingTextData();
	if( Other.m_pMultiDataB )
		*(m_pMultiDataB) = *(Other.m_pMultiDataB);

	m_pRoleData = new FloatingTextData();
	if( Other.m_pRoleData )
		*(m_pRoleData) = *(Other.m_pRoleData);
	return *this;
}

bool  AssociationWidgetData::operator==(AssociationWidgetData & Other) {
	if( this == &Other )

		return true;
	if( !m_pMultiDataA || !Other.m_pMultiDataA ) {
		if( !Other.m_pMultiDataA && m_pMultiDataA )
			return false;
		if( !m_pMultiDataA && Other.m_pMultiDataA )
			return false;
	} else if ( !(*m_pMultiDataA == *(Other.m_pMultiDataA)) )
		return false;

	if( !m_pMultiDataB || !Other.m_pMultiDataB ) {
		if( !Other.m_pMultiDataB && m_pMultiDataB )
			return false;
		if( !m_pMultiDataB && Other.m_pMultiDataB )
			return false;
	} else if ( !(*m_pMultiDataB == *(Other.m_pMultiDataB)) )
		return false;

	if( !m_pRoleData || !Other.m_pRoleData ) {
		if( !Other.m_pRoleData && m_pRoleData )
			return false;
		if( !m_pRoleData && Other.m_pRoleData )
			return false;
	} else if ( !(*m_pRoleData == *(Other.m_pRoleData)) )
		return false;

	if ( m_AssocType != Other.m_AssocType )
		return false;

	if(m_nWidgetAID != Other.m_nWidgetAID)
		return false;

	if(m_nWidgetBID != Other.m_nWidgetBID)
		return false;
	return true;
	/*
		//Don't test these values - only graphic representation of association
			and if all the other values are the same then it's the same association.

		if( !(m_LinePath  == Other.m_LinePath)  )
	    return false;


	  if(m_nIndexA != Other.m_nIndexA)
	    return false;
	  if(m_nIndexB != Other.m_nIndexB)
	    return false;
	  if(m_nTotalCountA != Other.m_nTotalCountA)
	    return false;
	  if(m_nTotalCountB != Other.m_nTotalCountB)
	    return false;
	*/
}

FloatingTextData* AssociationWidgetData::getMultiDataA() {
	return m_pMultiDataA;
}

void AssociationWidgetData::setMultiDataA( FloatingTextData* pData) {
	m_pMultiDataA = pData;
}

FloatingTextData* AssociationWidgetData::getMultiDataB() {
	return m_pMultiDataB;
}

void AssociationWidgetData::setMultiDataB( FloatingTextData* pData) {
	m_pMultiDataB = pData;
}

FloatingTextData* AssociationWidgetData::getRoleData() {
	return m_pRoleData;
}

void AssociationWidgetData::setRoleData( FloatingTextData* pData) {
	m_pRoleData = pData;
}

Uml::Association_Type AssociationWidgetData::getAssocType() {
	return m_AssocType;
}

void AssociationWidgetData::setAssocType( Uml::Association_Type Type) {
	m_AssocType = Type;
	m_LinePath.setAssocType( Type );
}

bool AssociationWidgetData::serialize(QDataStream *s, bool archive, int fileversion) {
	if(archive)
	{
		QString str;
		if(m_pMultiDataA) {
			str =  "MULTI_A";
			*s << str;
			if(!m_pMultiDataA->serialize(s, archive, fileversion)) {
				return false;
			}
		} else {
			str =  "NO_MULTI_A";
			*s << str;
		}
		if(m_pMultiDataB) {
			str =  "MULTI_B";
			*s << str;
			if(!m_pMultiDataB->serialize(s, archive, fileversion)) {
				return false;

			}

		} else {
			str =  "NO_MULTI_B";
			*s << str;
		}
		if(m_pRoleData) {
			str =  "ROLE";
			*s << str;
			if(!m_pRoleData->serialize(s, archive, fileversion)) {
				return false;
			}
		} else {
			str =  "NO_ROLE";
			*s << str;
		}
		int i = static_cast<int>(m_AssocType);
		*s << (Q_INT32) i;
		*s << m_nWidgetAID;
		*s << m_nWidgetBID;
		*s << m_nIndexA << m_nIndexB;
		*s << m_nTotalCountA << m_nTotalCountB << m_Doc;
		m_LinePath.serialize( s, archive, fileversion);
	} else //if (archive)
	{
		if (fileversion > 4)
		{
			QString str;

			*s >> str;
			if(str == "MULTI_A") {
				m_pMultiDataA = new FloatingTextData();

				if(!m_pMultiDataA->serialize(s, archive, fileversion)) {
					return false;
				}
			}
			*s >> str;
			if(str == "MULTI_B") {
				m_pMultiDataB = new FloatingTextData();
				if(!m_pMultiDataB->serialize(s, archive, fileversion)) {
					return false;
				}

			}
			*s >> str;
			if(str == "ROLE") {
				m_pRoleData = new FloatingTextData();
				if(!m_pRoleData->serialize(s, archive, fileversion)) {
					return false;
				}
			}

			int i;
			*s >> i;

			m_AssocType = static_cast<Uml::Association_Type>(i);
			*s >> m_nWidgetAID;
			*s >> m_nWidgetBID;
			*s >> m_nIndexA >> m_nIndexB;
			*s >> m_nTotalCountA >> m_nTotalCountB >> m_Doc;
			m_LinePath.serialize( s, archive, fileversion);
		} else {
			int assoc, count, totalCount;
			int fID , fx, fy, fox, foy, ax, ay, aox, aoy, bx, by, box, boy;
			QString ft, at, bt;
			*s >> assoc >> count >> totalCount;
			*s >> m_nWidgetAID >> m_nWidgetBID;

			switch(assoc) {
				case /* ARROW */ 402 :
					m_AssocType = Uml::at_Unknown;
					break;
				case /* GENERAL */ 403 :
					m_AssocType = Uml::at_Generalization;
					break;
				case /* AGGREG */ 404 :
					m_AssocType = Uml::at_Aggregation;
					break;
				case /* DEPENDENCY */ 405 :
					m_AssocType = Uml::at_Dependency;
					break;
				case /* ASSOC */ 406 :
					m_AssocType = Uml::at_Association;
					break;
				case /* MESSAGE */ 407 :
					m_AssocType = Uml::at_Seq_Message;
					break;
				case /* IMPLEMENTS */ 408 :
					m_AssocType = Uml::at_Implementation;
					break;
				case /* COMPOSITE */ 409 :
					m_AssocType = Uml::at_Composition;
					break;
				case /* UNIASSOC */ 410 :
					m_AssocType = Uml::at_UniAssociation;
					break;
				case /* REALIZE */ 411 :
					m_AssocType = Uml::at_Realization;
					break;
				case /* ANCHOR */ 412 :
					m_AssocType = Uml::at_Anchor;
					break;
				default:
					m_AssocType = Uml::at_Unknown;
					kdDebug() << "Unknown association type (" << assoc << ")" << endl;
					return false;
			}
//
// warning : missing init of offsets (Jens)
// the values of fox, foy, aox, aoy, box, boy are ignored
//
			*s >> ft >> fID >> fx >> fy >> fox >> foy;
			m_pRoleData = new FloatingTextData();
			m_pRoleData->setText(ft);
			m_pRoleData->setRole(Uml::tr_RoleName);
			m_pRoleData->setX(fx);
			m_pRoleData->setY(fy);

			*s >> at >> ax >> ay >> aox >> aoy;
			m_pMultiDataA = new FloatingTextData();
			m_pMultiDataA->setText(at);
			m_pMultiDataA->setRole(Uml::tr_MultiA);
			m_pMultiDataA->setX(ax);
			m_pMultiDataA->setY(ay);

			*s >> bt >> bx >> by >> box >> boy;
			m_pMultiDataB = new FloatingTextData();
			m_pMultiDataB->setText(bt);
			m_pMultiDataB->setRole(Uml::tr_MultiB);
			m_pMultiDataB->setX(bx);
			m_pMultiDataB->setY(by);
//
// warning : the values of m_nIndexA, m_nIndexB, m_nTotalCountA, m_nTotalCountB, m_Doc
// are not initialized
//
		}
	}

	return true;
}

long AssociationWidgetData::getClipSizeOf() {
	long l_size = 0;
	QString str;
	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	if(m_pMultiDataA) {
		str = "MULTI_A";
		l_size += m_pMultiDataA->getClipSizeOf();
	} else {
		str = "NO_MULTI_A";
	}
	if ( !str.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (str.length()*sizeof(QChar));
	}
	if(m_pMultiDataB) {
		str = "MULTI_B";
		l_size += m_pMultiDataB->getClipSizeOf();
	} else {
		str = "NO_MULTI_B";
	}
	if ( !str.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (str.length()*sizeof(QChar));
	}
	if(m_pRoleData) {
		str = "ROLE";

		l_size += m_pRoleData->getClipSizeOf();
	} else {
		str = "NO_ROLE";
	}
	if ( !str.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (str.length()*sizeof(QChar));

	}
	if ( !m_Doc.length() ) {
		l_size += sizeof(tmp);

	} else {
		l_size += (m_Doc.length()*sizeof(QChar));

	}

	l_size += sizeof(Q_INT32); //assoc type

	l_size += m_LinePath.getClipSizeOf();

	uint count = 0;
	Q_INT32 j =  0;
	l_size += ((count * 2 )* sizeof(j)); //Q_INT32 is the size of the X and Y members of each QPoint
	//This assuming we are working with a QDataStream version
	// greater than 1
	l_size += sizeof(m_nWidgetAID);
	l_size += sizeof(m_nWidgetBID);
	l_size += sizeof(m_nIndexA);
	l_size += sizeof(m_nIndexB);
	l_size += sizeof(m_nTotalCountA);
	l_size += sizeof(m_nTotalCountB);
	return l_size;
}

int AssociationWidgetData::getWidgetAID() {
	return m_nWidgetAID;
}

void AssociationWidgetData::setWidgetAID( int AID) {
	m_nWidgetAID = AID;
}

int AssociationWidgetData::getWidgetBID() {
	return m_nWidgetBID;
}

/** Write property of int m_nWidgetBID. */
void AssociationWidgetData::setWidgetBID( int BID) {
	m_nWidgetBID = BID;
}

void AssociationWidgetData::print2cerr() {
	QString str;
	switch(m_AssocType) {
		case Uml::at_Generalization:
			str = "GENERAL";
			break;
		case Uml::at_Aggregation:
			str = "AGGREG";
			break;
		case Uml::at_Dependency:
			str = "DEPENDENCY";

			break;
		case Uml::at_Association:
			str = "ASSOC";
			break;
			//case Uml::at_Message:
			//	str = "MESSAGE";
			break;
		case Uml::at_Implementation:
			str = "IMPLEMENTS";
			break;
		case Uml::at_Composition:

			str = "COMPOSITE";
			break;
		case Uml::at_UniAssociation:
			str = "UNIASSOC";
			break;
		case Uml::at_Realization:
			str = "REALIZE";
			break;
		case Uml::at_Anchor:
			str = "ANCHOR";
			break;
		default:
			str = "UNKNOWN TYPE";
			break;
	}
	kdDebug() << "m_AssocType = " << str <<endl;
	kdDebug() << "m_nWidgetAID = " << m_nWidgetAID << endl;
	kdDebug() << "m_nWidgetBID = " << m_nWidgetBID << endl;

	if(m_pMultiDataA) {
		kdDebug() << "m_pMultiDataA = {" << endl << endl;
		m_pMultiDataA->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pMultiDataA = NULL" << endl;
	}
	if(m_pRoleData) {
		kdDebug() << "m_pRoleData = {" << endl << endl;
		m_pRoleData->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pRoleData = NULL" << endl;
	}
	if(m_pMultiDataB) {
		kdDebug() << "m_pMultiDataB = {" << endl << endl;
		m_pMultiDataB->print2cerr();
		kdDebug() << "}" << endl;
	} else {

		kdDebug() << "m_pMultiDataB = NULL" << endl;
	}
}

bool AssociationWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement assocElement = qDoc.createElement( "UML:AssocWidget" );
	bool status = true;

	if( m_pMultiDataA )
		status =  m_pMultiDataA -> saveToXMI( qDoc, assocElement );

	if( m_pMultiDataB )
		status = m_pMultiDataB -> saveToXMI( qDoc, assocElement );

	if( m_pRoleData )
		status = m_pRoleData -> saveToXMI( qDoc, assocElement );

	assocElement.setAttribute( "type", m_AssocType );
	assocElement.setAttribute( "indexa", m_nIndexA );
	assocElement.setAttribute( "indexb", m_nIndexB );
	assocElement.setAttribute( "totalcounta", m_nTotalCountA );
	assocElement.setAttribute( "totalcountb", m_nTotalCountB );
	assocElement.setAttribute( "widgetaid", m_nWidgetAID );
	assocElement.setAttribute( "widgetbid", m_nWidgetBID );
	assocElement.setAttribute( "documentation", m_Doc );
	m_LinePath.saveToXMI( qDoc, assocElement );
	qElement.appendChild( assocElement );
	return status;
}

bool AssociationWidgetData::loadFromXMI( QDomElement & qElement ) {
	m_Doc = qElement.attribute( "documentation", "" );
	QString indexa = qElement.attribute( "indexa", "0" );
	QString indexb = qElement.attribute( "indexb", "0" );
	QString totalcounta = qElement.attribute( "totalcounta", "0" );
	QString totalcountb = qElement.attribute( "totalcountb", "0" );
	QString widgetaid = qElement.attribute( "widgetaid", "-1" );
	QString widgetbid = qElement.attribute( "widgetbid", "-1" );
	QString type = qElement.attribute( "type", "" );

	m_nIndexA = indexa.toInt();
	m_nIndexB = indexb.toInt();
	m_nTotalCountA = totalcounta.toInt();
	m_nTotalCountB = totalcountb.toInt();
	m_nWidgetAID = widgetaid.toInt();
	m_nWidgetBID = widgetbid.toInt();
	if( type != "" )
		m_AssocType = (Uml::Association_Type)type.toInt();

	//now load child elements
	QDomNode node = qElement.firstChild();
	QDomElement element = node.toElement();
	while( !element.isNull() ) {
		QString tag = element.tagName();
		if( tag == "linepath" ) {
			if( !m_LinePath.loadFromXMI( element ) )
				return false;
		} else if( tag == "UML:FloatingTextWidget" ) {
			QString r = element.attribute( "role", "-1");
			if( r == "-1" )
				return false;
			Uml::Text_Role role = (Uml::Text_Role)r.toInt();
			switch( role ) {
				case Uml::tr_MultiA:
					m_pMultiDataA = new FloatingTextData();
					if( !m_pMultiDataA -> loadFromXMI( element ) )
						return false;
					break;

				case Uml::tr_MultiB:
					m_pMultiDataB = new FloatingTextData();
					if( !m_pMultiDataB -> loadFromXMI( element ) )
						return false;
					break;

				case Uml::tr_RoleName:
				case Uml::tr_Coll_Message:
					m_pRoleData = new FloatingTextData();
					if( !m_pRoleData -> loadFromXMI( element ) )
						return false;
					break;
				default:
					break;
			}
		}
		node = element.nextSibling();
		element = node.toElement();
	}
	return true;
}

void AssociationWidgetData::cleanup() {
	m_LinePath.cleanup();
}






