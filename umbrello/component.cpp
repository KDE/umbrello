/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "component.h"
#include "association.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLComponent::UMLComponent(QObject* parent, QString name, int id) : UMLObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLComponent::UMLComponent(QObject* parent) : UMLObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLComponent::~UMLComponent() {
 	m_AssocsList.clear();
 	m_TmpAssocs.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLAssociation> UMLComponent::getSpecificAssocs(Uml::Association_Type assocType) {
 	QPtrList<UMLAssociation> list;
 	for (UMLAssociation* a = m_AssocsList.first(); a; a = m_AssocsList.next())
 		if (a->getAssocType() == assocType)
 			list.append(a);
 	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLComponent::addAssociation(UMLAssociation* assoc) {
 	m_AssocsList.append( assoc );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLComponent::hasAssociation(UMLAssociation* assoc) {
 	if(m_AssocsList.containsRef(assoc) > 0)
 		return true;
 	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLComponent::removeAssociation(UMLObject* a) {
 	if(!m_AssocsList.remove((UMLAssociation *)a)) {
 		kdDebug() << "can't find assoc given in list" << endl;
 		return -1;
 	}
 	return m_AssocsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLComponent::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		currentName = i18n("new_association");
	} else {
		kdWarning() << "creating child object which isn't association for component" << endl;
	}

	QString name = currentName;
	for (int number = 0; findChildObject(type, name).count(); ++number,
	        name = currentName + "_" + QString::number(number))
		;
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLObject> UMLComponent::findChildObject(UMLObject_Type t, QString n) {
  	QPtrList<UMLObject> list;
 	if (t == ot_Association) {
 		UMLAssociation * obj=0;
 		for (obj = m_AssocsList.first(); obj != 0; obj = m_AssocsList.next()) {
 			if (obj->getBaseType() == t && obj -> getName() == n)
 				list.append( obj );
 		}
 	} else {
		kdWarning() << "type must be association" << endl;
	}
	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLComponent::findChildObject(int id) {
	UMLAssociation * asso = 0;
	for (asso = m_AssocsList.first(); asso != 0; asso = m_AssocsList.next()) {
		if (asso->getID() == id)
			return asso;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLComponent::serialize(QDataStream *s, bool archive, int fileversion) {
	// Note: The m_AssocsList is filled by UMLDoc::serialize().
	// UMLDoc::serialize() serializes the UMLComponents before the
	// UMLAssociations.
	bool status = UMLObject::serialize(s, archive, fileversion);
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLComponent::init() {
	m_BaseType = ot_Component;
	m_AssocsList.clear();
	m_AssocsList.setAutoDelete(false);
	m_TmpAssocs.clear();
	m_TmpAssocs.setAutoDelete(false);
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
long UMLComponent::getClipSizeOf() {
	return UMLObject::getClipSizeOf();
}

bool UMLComponent::operator==(UMLComponent& rhs) {
	if (this == &rhs) {
		return true;
	}
	if ( !UMLObject::operator==(rhs) ) {
		return false;
	}
	if ( m_AssocsList.count() != rhs.m_AssocsList.count() ) {
		return false;
	}
	if ( &m_AssocsList != &(rhs.m_AssocsList) ) {
		return false;
	}
	return true;
}

bool UMLComponent::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement componentElement = qDoc.createElement("UML:Component");
	bool status = UMLObject::saveToXMI(qDoc, componentElement);
	qElement.appendChild(componentElement);
	return status;
}

bool UMLComponent::loadFromXMI( QDomElement & element ) {
	if ( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	return true;
}

int UMLComponent::associations() {
	return m_AssocsList.count();
}

const QPtrList<UMLAssociation>& UMLComponent::getAssociations() {
	return m_AssocsList;
}

QPtrList<UMLAssociation> UMLComponent::getGeneralizations() {
	return getSpecificAssocs(Uml::at_Generalization);
}

QPtrList<UMLAssociation> UMLComponent::getAggregations() {
	return getSpecificAssocs(Uml::at_Aggregation);
}

QPtrList<UMLAssociation> UMLComponent::getCompositions() {
	return getSpecificAssocs(Uml::at_Composition);
}
