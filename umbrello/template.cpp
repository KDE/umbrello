/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "template.h"
#include <kdebug.h>

UMLTemplate::UMLTemplate(QObject* parent, QString name, int id, QString type) : UMLObject(parent, name, id) {
	m_TypeName = type;
	m_BaseType = ot_Template;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLTemplate::UMLTemplate(QObject* parent) : UMLObject(parent) {
	m_BaseType = ot_Template;
	m_TypeName = "";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLTemplate::~UMLTemplate() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLTemplate::getTypeName() {
	return m_TypeName;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLTemplate::setTypeName(QString type) {
	m_TypeName = type;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLTemplate::serialize(QDataStream* s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if (!status) {
		return status;
	}
	if (archive) {
		*s << m_TypeName;
	} else {
		*s >> m_TypeName;
	}
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLTemplate::toString() {
	if (m_TypeName == "" || m_TypeName == "class") {
		return getName();
	} else {
		return getName() + " : " + m_TypeName;
	}
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
long UMLTemplate::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();
	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	//We assume we are working with QT 2.1.x or superior, that means
	//if unicode returns a null pointer then the serialization process of the QString object
	//will write a null marker 0xffffff, see QString::operator<< implementation
	if ( !m_TypeName.length() ) {
		l_size += sizeof(tmp);
	} else {
		l_size += (m_TypeName.length()*sizeof(QChar));
	}

	return l_size;
}

bool UMLTemplate::operator==(UMLTemplate &rhs) {
	if (this == &rhs) {
		return true;
	}
	if ( !UMLObject::operator==( rhs ) ) {
		return false;
	}
	if (m_TypeName != rhs.m_TypeName) {
		return false;
	}
	return true;
}

bool UMLTemplate::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement attributeElement = qDoc.createElement("template");
	bool status = UMLObject::saveToXMI(qDoc, attributeElement);
	attributeElement.setAttribute("type", m_TypeName);
	qElement.appendChild(attributeElement);
	return status;
}

bool UMLTemplate::loadFromXMI(QDomElement& element) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	m_TypeName = element.attribute("type", "");
	return true;
}
