/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usecase.h"
#include "umldoc.h"

UMLUseCase::UMLUseCase(QObject * parent, QString Name, int id) : UMLObject(parent, Name, id) {
	m_BaseType = ot_UseCase;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLUseCase::UMLUseCase(QObject * parent) : UMLObject(parent) {
	m_BaseType = ot_UseCase;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLUseCase::~UMLUseCase() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLUseCase::serialize(QDataStream * s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if(!status)
		return status;
	if(archive) {
		//doesn't need to do anything
	} else {
		//doesn't need to do anything
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
long UMLUseCase::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();

	return l_size;
}

bool UMLUseCase::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement usecaseElement = qDoc.createElement( "UML:UseCase" );
	bool status = UMLObject::saveToXMI( qDoc, usecaseElement );
	qElement.appendChild( usecaseElement );
	return status;
}

bool UMLUseCase::loadFromXMI( QDomElement & element ) {
	return UMLObject::loadFromXMI( element );
}







