/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "attribute.h"

UMLAttribute::UMLAttribute(QObject *parent, QString Name, int id, QString type, Scope s, QString iv) : UMLObject(parent,Name, id) {
	m_TypeName = type;
	m_InitialValue = iv;
	m_BaseType = ot_Attribute;
	m_Scope = s;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute::UMLAttribute(QObject * parent) : UMLObject(parent) {
	m_BaseType = ot_Attribute;
	m_TypeName = m_InitialValue = "";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute::~UMLAttribute() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLAttribute::getTypeName() {
	return m_TypeName;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLAttribute::getInitialValue() {
	return m_InitialValue;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLAttribute::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if(!status)
		return status;
	if(archive) {
		*s << m_InitialValue
		<< m_TypeName;
	} else {
		*s >> m_InitialValue
		>> m_TypeName;
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLAttribute::toString(Signature_Type sig) {
	QString s;

	if(sig == st_ShowSig || sig == st_NoSig) {
		if(m_Scope == Public)
			s = "+ ";
		else if(m_Scope == Private)
			s = "- ";
		else if(m_Scope == Protected)
			s= "# ";
	} else
		s = "";

	if(sig == st_ShowSig || sig == st_SigNoScope) {
		QString string = s + getName() + " : " + m_TypeName;
		if(m_InitialValue.length() > 0)
			string += " = " + m_InitialValue;
		return string;
	} else
		return s + getName();
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
long UMLAttribute::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();
	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	if ( !m_InitialValue.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (m_InitialValue.length()*sizeof(QChar));
	}

	if ( !m_TypeName.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (m_TypeName.length()*sizeof(QChar));
	}

	return l_size;
}

bool UMLAttribute::operator==( UMLAttribute &rhs) {
	if( this == &rhs )
		return true;

	if( !UMLObject::operator==( rhs ) )
		return false;

	if( m_InitialValue != rhs.m_InitialValue )
		return false;

	if( m_TypeName != rhs.m_TypeName )
		return false;

	return true;
}

bool UMLAttribute::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement attributeElement = qDoc.createElement( "UML:Attribute" );
	bool status = UMLObject::saveToXMI( qDoc, attributeElement );
	attributeElement.setAttribute( "type", m_TypeName );
	attributeElement.setAttribute( "value", m_InitialValue );
	qElement.appendChild( attributeElement );
	return status;
}

bool UMLAttribute::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI( element ) )
		return false;
	m_TypeName = element.attribute( "type", "" );
	m_InitialValue = element.attribute( "value", "" );
	return true;
}



