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
	m_pMultiDataA = 0;
	m_pMultiDataB = 0;
	m_pChangeDataA = 0;
	m_pChangeDataB = 0;
	m_pNameData = 0;
	m_VisibilityA = Public;
	m_VisibilityB = Public;
	m_ChangeabilityA = chg_Changeable;
	m_ChangeabilityB = chg_Changeable;
	m_pRoleAData = 0;
	m_pRoleBData = 0;
	m_nWidgetAID = -1;
	m_nWidgetBID = -1;
	m_AssocType = Uml::at_Association;
	m_LinePath = new LinePath;
}

AssociationWidgetData::~AssociationWidgetData() {
	/*
	if (m_LinePath) {
		delete m_LinePath;
		m_LinePath = NULL;
	}
	 */
}

AssociationWidgetData::AssociationWidgetData(AssociationWidgetData & Other) {
	m_LinePath = new LinePath;
	*this = Other;
}

AssociationWidgetData & AssociationWidgetData::operator=(AssociationWidgetData & Other) {
	m_AssocType = Other.m_AssocType;
	*m_LinePath = *(Other.m_LinePath);
	m_nWidgetAID = Other.m_nWidgetAID;
	m_nWidgetBID = Other.m_nWidgetBID;
	m_nIndexA = Other.m_nIndexA;
	m_nIndexB = Other.m_nIndexB;
	m_VisibilityA = Other.m_VisibilityA;
	m_VisibilityB = Other.m_VisibilityB;
	m_ChangeabilityA = Other.m_ChangeabilityA;
	m_ChangeabilityB = Other.m_ChangeabilityB;
	m_nTotalCountA = Other.m_nTotalCountA;
	m_nTotalCountB = Other.m_nTotalCountB;

	m_pNameData = new FloatingTextData();
	if( Other.m_pNameData )
		*(m_pNameData) = *(Other.m_pNameData);

	m_pMultiDataA = new FloatingTextData();
	if( Other.m_pMultiDataA )
		*(m_pMultiDataA) = *(Other.m_pMultiDataA);

	m_pMultiDataB = new FloatingTextData();
	if( Other.m_pMultiDataB )
		*(m_pMultiDataB) = *(Other.m_pMultiDataB);

	m_pRoleAData = new FloatingTextData();
	if( Other.m_pRoleAData )
		*(m_pRoleAData) = *(Other.m_pRoleAData);

	m_pRoleBData = new FloatingTextData();
	if( Other.m_pRoleBData )
		*(m_pRoleBData) = *(Other.m_pRoleBData);

	m_pChangeDataA = new FloatingTextData();
	if( Other.m_pChangeDataA )
		*(m_pChangeDataA) = *(Other.m_pChangeDataA);

	m_pChangeDataB = new FloatingTextData();
	if( Other.m_pChangeDataB )
		*(m_pChangeDataB) = *(Other.m_pChangeDataB);

	return *this;
}

bool  AssociationWidgetData::operator==(AssociationWidgetData & Other) {
	if( this == &Other )
		return true;

	if( !m_pNameData || !Other.m_pNameData ) {
		if( !Other.m_pNameData && m_pNameData )
			return false;
		if( !m_pNameData && Other.m_pNameData )
			return false;
	} else if ( !(*m_pNameData == *(Other.m_pNameData)) )
		return false;

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

	if( !m_pRoleAData || !Other.m_pRoleAData ) {
		if( !Other.m_pRoleAData && m_pRoleAData )
			return false;
		if( !m_pRoleAData && Other.m_pRoleAData )
			return false;
	} else if ( !(*m_pRoleAData == *(Other.m_pRoleAData)) )
		return false;

        if( !m_pRoleBData || !Other.m_pRoleBData ) {
		if( !Other.m_pRoleBData && m_pRoleBData )
			return false;
		if( !m_pRoleBData && Other.m_pRoleBData )
			return false;
	} else if ( !(*m_pRoleBData == *(Other.m_pRoleBData)) )
		return false;

        if( !m_pChangeDataA || !Other.m_pChangeDataA ) {
		if( !Other.m_pChangeDataA && m_pChangeDataA )
			return false;
		if( !m_pChangeDataA && Other.m_pChangeDataA )
			return false;
	} else if ( !(*m_pChangeDataA == *(Other.m_pChangeDataA)) )
		return false;

	if( !m_pChangeDataB || !Other.m_pChangeDataB ) {
		if( !Other.m_pChangeDataB && m_pChangeDataB )
			return false;
		if( !m_pChangeDataB && Other.m_pChangeDataB )
			return false;
	} else if ( !(*m_pChangeDataB == *(Other.m_pChangeDataB)) )
		return false;

	if ( m_AssocType != Other.m_AssocType )
		return false;

	if(m_VisibilityA != Other.m_VisibilityA)
		return false;

	if(m_VisibilityB != Other.m_VisibilityB)
		return false;

	if(m_ChangeabilityA != Other.m_ChangeabilityA)
		return false;

	if(m_ChangeabilityB != Other.m_ChangeabilityB)
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

FloatingTextData* AssociationWidgetData::getChangeDataA() {
	return m_pChangeDataA;
}

FloatingTextData* AssociationWidgetData::getChangeDataB() {
	return m_pChangeDataB;
}

void AssociationWidgetData::setMultiDataB( FloatingTextData* pData) {
	m_pMultiDataB = pData;
}

FloatingTextData* AssociationWidgetData::getNameData()
{
	return m_pNameData;
}

FloatingTextData* AssociationWidgetData::getRoleAData() {
	return m_pRoleAData;
}

FloatingTextData* AssociationWidgetData::getRoleBData() {
	return m_pRoleBData;
}

Scope AssociationWidgetData::getVisibilityA() {
	return m_VisibilityA;
}

Scope AssociationWidgetData::getVisibilityB() {
	return m_VisibilityB;
}

Changeability_Type AssociationWidgetData::getChangeabilityA()
{
	return m_ChangeabilityA;
}

Changeability_Type AssociationWidgetData::getChangeabilityB()
{
	return m_ChangeabilityB;
}

// should be private?
void AssociationWidgetData::setChangeDataA( FloatingTextData* pData) {
	m_pChangeDataA = pData;
}

// should be private?
void AssociationWidgetData::setChangeDataB( FloatingTextData* pData) {
	m_pChangeDataB = pData;
}

void AssociationWidgetData::setNameData( FloatingTextData* pData) {
	m_pNameData = pData;
}

void AssociationWidgetData::setRoleAData( FloatingTextData* pData) {
	m_pRoleAData = pData;
}

void AssociationWidgetData::setRoleBData( FloatingTextData* pData) {
	m_pRoleBData = pData;
}

void AssociationWidgetData::setVisibilityA ( Scope scope) {
	m_VisibilityA = scope;
}

void AssociationWidgetData::setVisibilityB ( Scope scope) {
	m_VisibilityB = scope;
}

void AssociationWidgetData::setChangeabilityA ( Changeability_Type value ) {
	m_ChangeabilityA = value;
}

void AssociationWidgetData::setChangeabilityB ( Changeability_Type value ) {
	m_ChangeabilityB = value;
}

Uml::Association_Type AssociationWidgetData::getAssocType() {
	return m_AssocType;
}

void AssociationWidgetData::setAssocType( Uml::Association_Type Type) {
	m_AssocType = Type;
	m_LinePath->setAssocType( Type );
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

	if(m_pNameData) {
		kdDebug() << "m_pNameData = {" << endl << endl;
		m_pNameData->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pNameData = NULL" << endl;
	}
	if(m_pMultiDataA) {
		kdDebug() << "m_pMultiDataA = {" << endl << endl;
		m_pMultiDataA->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pMultiDataA = NULL" << endl;
	}
	if(m_pRoleAData) {
		kdDebug() << "m_pRoleAData = {" << endl << endl;
		m_pRoleAData->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pRoleAData = NULL" << endl;
	}
	if(m_pRoleBData) {
		kdDebug() << "m_pRoleBData = {" << endl << endl;
		m_pRoleBData->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pRoleBData = NULL" << endl;
	}
	if(m_pMultiDataB) {
		kdDebug() << "m_pMultiDataB = {" << endl << endl;
		m_pMultiDataB->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pMultiDataB = NULL" << endl;
	}
	if(m_pChangeDataA) {
		kdDebug() << "m_pChangeDataA = {" << endl << endl;
		m_pChangeDataA->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pChangeDataA = NULL" << endl;
	}
	if(m_pChangeDataB) {
		kdDebug() << "m_pChangeDataB = {" << endl << endl;
		m_pChangeDataB->print2cerr();
		kdDebug() << "}" << endl;
	} else {
		kdDebug() << "m_pChangeDataB = NULL" << endl;
	}
}

bool AssociationWidgetData::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement assocElement = qDoc.createElement( "UML:AssocWidget" );
	bool status = true;

	if( m_pNameData )
		status =  m_pNameData -> saveToXMI( qDoc, assocElement );

	if( m_pMultiDataA )
		status =  m_pMultiDataA -> saveToXMI( qDoc, assocElement );

	if( m_pMultiDataB )
		status = m_pMultiDataB -> saveToXMI( qDoc, assocElement );

	if( m_pRoleAData )
		status = m_pRoleAData -> saveToXMI( qDoc, assocElement );

	if( m_pRoleBData )
		status = m_pRoleBData -> saveToXMI( qDoc, assocElement );

	if( m_pChangeDataA )
		status =  m_pChangeDataA -> saveToXMI( qDoc, assocElement );

	if( m_pChangeDataB )
		status =  m_pChangeDataB -> saveToXMI( qDoc, assocElement );

	assocElement.setAttribute( "visibilityA", m_VisibilityA);
	assocElement.setAttribute( "visibilityB", m_VisibilityB);
	assocElement.setAttribute( "changeabilityA", m_ChangeabilityA);
	assocElement.setAttribute( "changeabilityB", m_ChangeabilityB);
	assocElement.setAttribute( "type", m_AssocType );
	assocElement.setAttribute( "indexa", m_nIndexA );
	assocElement.setAttribute( "indexb", m_nIndexB );
	assocElement.setAttribute( "totalcounta", m_nTotalCountA );
	assocElement.setAttribute( "totalcountb", m_nTotalCountB );
	assocElement.setAttribute( "widgetaid", m_nWidgetAID );
	assocElement.setAttribute( "widgetbid", m_nWidgetBID );
	assocElement.setAttribute( "documentation", m_Doc );
	assocElement.setAttribute( "roleAdoc", m_RoleADoc);
	assocElement.setAttribute( "roleBdoc", m_RoleBDoc);
	m_LinePath->saveToXMI( qDoc, assocElement );
	qElement.appendChild( assocElement );
	return status;
}

bool AssociationWidgetData::loadFromXMI( QDomElement & qElement ) {
	m_Doc = qElement.attribute( "documentation", "" );
	m_RoleADoc = qElement.attribute( "roleAdoc", "" );
	m_RoleBDoc = qElement.attribute( "roleBdoc", "" );
	QString indexa = qElement.attribute( "indexa", "0" );
	QString indexb = qElement.attribute( "indexb", "0" );
	QString totalcounta = qElement.attribute( "totalcounta", "0" );
	QString totalcountb = qElement.attribute( "totalcountb", "0" );
	QString widgetaid = qElement.attribute( "widgetaid", "-1" );
	QString widgetbid = qElement.attribute( "widgetbid", "-1" );
	QString type = qElement.attribute( "type", "" );

	QString visibilityA = qElement.attribute( "visibilityA", "0");
	QString visibilityB = qElement.attribute( "visibilityB", "0");
	QString changeabilityA = qElement.attribute( "changeabilityA", "0");
	QString changeabilityB = qElement.attribute( "changeabilityB", "0");

	// visibilty defaults to Public if it cant set it here..
	if (visibilityA.toInt() > 0)
		setVisibilityA( (Scope) visibilityA.toInt());

	if (visibilityB.toInt() > 0)
		setVisibilityB( (Scope) visibilityB.toInt());

	// Changeability defaults to "Changeable" if it cant set it here..
	if (changeabilityA.toInt() > 0)
		setChangeabilityA ( (Changeability_Type) changeabilityA.toInt());
	if (changeabilityB.toInt() > 0)
		setChangeabilityB ( (Changeability_Type) changeabilityB.toInt());

	m_nIndexA = indexa.toInt();
	m_nIndexB = indexb.toInt();
	m_nTotalCountA = totalcounta.toInt();
	m_nTotalCountB = totalcountb.toInt();
	m_nWidgetAID = widgetaid.toInt();
	m_nWidgetBID = widgetbid.toInt();
	if( !type.isEmpty() )
		m_AssocType = (Uml::Association_Type)type.toInt();

	//now load child elements
	QDomNode node = qElement.firstChild();
	QDomElement element = node.toElement();
	while( !element.isNull() ) {
		QString tag = element.tagName();
		if( tag == "linepath" ) {
			if( !m_LinePath->loadFromXMI( element ) )
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

				case Uml::tr_ChangeA:
					m_pChangeDataA = new FloatingTextData();
					if( !m_pChangeDataA -> loadFromXMI( element ) )
						return false;
					break;

				case Uml::tr_ChangeB:
					m_pChangeDataB = new FloatingTextData();
					if( !m_pChangeDataB -> loadFromXMI( element ) )
						return false;
					break;

				case Uml::tr_Name:
				case Uml::tr_Coll_Message:
					m_pNameData = new FloatingTextData();
					if( !m_pNameData -> loadFromXMI( element ) )
						return false;
					break;
				case Uml::tr_RoleAName:
					m_pRoleAData = new FloatingTextData();
					if( !m_pRoleAData -> loadFromXMI( element ) )
						return false;
					break;
				case Uml::tr_RoleBName:
					m_pRoleBData = new FloatingTextData();
					if( !m_pRoleBData -> loadFromXMI( element ) )
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
	m_LinePath->cleanup();
}






