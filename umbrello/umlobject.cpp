/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>

#include "umlobject.h"

UMLObject::UMLObject(QObject * parent,  QString Name, int id) : QObject(parent, "AnUMLObject") {
	m_nId = id;
	m_BaseType = ot_UMLObject;
	m_Name = Name;
	m_Scope = Public;
	m_Stereotype = m_Package = "";
	m_bAbstract = false;
	m_bStatic = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject::UMLObject(QObject * parent) : QObject(parent) {
	m_BaseType = ot_UMLObject;
	m_nId = -1;
	m_Scope = Public;
	m_Name = "";
	m_Stereotype = m_Package = "";
	m_bAbstract = false;
	m_bStatic = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject::~UMLObject() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLObject::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = true;
	if(archive) {
		//save to current file
		*s << m_BaseType
		<< getName()
		<< m_nId
		<< m_Doc
		<< m_Scope
		<< (QString)m_Package
		<< (QString)m_Stereotype
		<< (int)m_bAbstract
		<< (int)m_bStatic;
	} else {
		int scope;
		m_bAbstract = false;
		m_bStatic = false;
		QString _name;
		*s >> _name
		>> m_nId
		>> m_Doc
		>> scope
		>> m_Package
		>> m_Stereotype;
		setName(_name);
		if (fileversion > 4)
		{
			int bAbstract, bStatic;
			*s >> bAbstract
			>> bStatic;
			m_Scope = (Uml::Scope)scope;
			m_bAbstract = bAbstract;
			m_bStatic = bStatic;
		}
		else
			m_Scope = (Uml::Scope)(scope - 1);
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLObject::setID(int NewID) {
	m_nId = NewID;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLObject::setName(QString strName) {
	m_Name = strName;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLObject::getName() {
	return m_Name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

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
long UMLObject::getClipSizeOf() {
	long l_size = sizeof(m_BaseType) + sizeof(m_nId) + sizeof(m_Scope);

	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	QString _name = getName();

	if ( !_name.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (_name.length()*sizeof(QChar));
	}

	if ( !m_Doc.length() ) {
		l_size += sizeof(tmp);
	} else {
		l_size += (m_Doc.length()*sizeof(QChar));
	}

	if ( !m_Package.length() ) {
		l_size += sizeof(tmp);
	} else {
		l_size += (m_Package.length()*sizeof(QChar));
	}

	if ( !m_Stereotype.length() ) {
		l_size += sizeof(tmp);
	} else {
		l_size += (m_Stereotype.length()*sizeof(QChar));
	}

	l_size += sizeof( int );//m_bAbstract
	l_size += sizeof( int );//m_bStatic

	return l_size;
}

bool UMLObject::operator==(UMLObject & rhs ) {
	if( this == &rhs )
		return true;

	//don't compare IDs, these are program specific and
	//don't mean the objects are the same
	if( m_Name != rhs.m_Name )
		return false;

	if( m_BaseType != rhs.m_BaseType )
		return false;

	if( m_Doc != rhs.m_Doc )
		return false;

	if( m_Package != rhs.m_Package )
		return false;

	if( m_Scope != rhs.m_Scope )
		return false;

	if( m_Stereotype != rhs.m_Stereotype )
		return false;

	if( m_bAbstract != rhs.m_bAbstract )
		return false;

	return true;

}

bool UMLObject::getAbstract() {
	return m_bAbstract;
}

void UMLObject::setAbstract(bool bAbstract) {
	m_bAbstract = bAbstract;
}

bool UMLObject::saveToXMI( QDomDocument & /*qDoc*/, QDomElement & qElement ) {
	/*
	  Call after required actions in child class.
	  Type must be set in the child class.
	*/
	qElement.setAttribute( "xmi.id", m_nId );
	qElement.setAttribute( "name", m_Name );
	qElement.setAttribute( "documentation", m_Doc );
	qElement.setAttribute( "package", m_Package );
	qElement.setAttribute( "scope", m_Scope );
	qElement.setAttribute( "stereotype", m_Stereotype );
	qElement.setAttribute( "abstract", m_bAbstract );
 	qElement.setAttribute( "static", m_bStatic );
	return true;
}

bool UMLObject::loadFromXMI( QDomElement & element ) {
	QString id = element.attribute( "xmi.id", "-1" );
	m_Name = element.attribute( "name", "" );
	m_Doc = element.attribute( "documentation", "" );
	m_Package = element.attribute( "package", "" );
	QString scope = element.attribute( "scope", "-1" );
	m_Stereotype = element.attribute( "stereotype", "" );
	QString abstract = element.attribute( "abstract", "0" );
   QString staticScope = element.attribute( "static", "0" );

	m_nId = id.toInt();
	m_Scope = (Scope)scope.toInt();
	m_bAbstract = (bool)abstract.toInt();
	m_bStatic = (bool)staticScope.toInt();
	if( m_nId == -1 || m_Scope == -1 )
		return false;
	return true;
}

/** Returns true if this UMLObject has classifier scope, otherwise false (the default). */
bool UMLObject::getStatic() const
{
	return m_bStatic;
}
/** Sets the value for m_bStatic. */
void UMLObject::setStatic(const bool bStatic)
{
	m_bStatic = bStatic;
}
#include "umlobject.moc"
