/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "umlcanvasobject.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "template.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLCanvasObject::UMLCanvasObject(QObject* parent, QString name, int id) : UMLObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLCanvasObject::UMLCanvasObject(QObject* parent) : UMLObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLCanvasObject::~UMLCanvasObject() {
 	m_AssocsList.clear();
 	m_TmpAssocs.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLAssociation> UMLCanvasObject::getSpecificAssocs(Uml::Association_Type assocType) {
 	QPtrList<UMLAssociation> list;
 	for (UMLAssociation* a = m_AssocsList.first(); a; a = m_AssocsList.next())
 		if (a->getAssocType() == assocType)
 			list.append(a);
 	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLCanvasObject::addAssociation(UMLAssociation* assoc) {
 	m_AssocsList.append( assoc );
	emit modified();
	emit sigAssociationAdded(assoc);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLCanvasObject::hasAssociation(UMLAssociation* assoc) {
 	if(m_AssocsList.containsRef(assoc) > 0)
 		return true;
 	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLCanvasObject::removeAssociation(UMLAssociation * assoc) {
 	if(!m_AssocsList.remove(assoc)) {
 		kdWarning() << "can't find assoc given in list" << endl;
 		return -1;
 	}
	emit modified();
	emit sigAssociationRemoved(assoc);
 	return m_AssocsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLCanvasObject::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		currentName = i18n("new_association");
	} else {
		kdWarning() << "uniqChildName() called for unknown child type" << endl;
	}

	QString name = currentName;
	for (int number = 1; findChildObject(type, name).count(); ++number) {
	        name = currentName + "_" + QString::number(number);
	}
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLObject> UMLCanvasObject::findChildObject(UMLObject_Type t, QString n) {
  	QPtrList<UMLObject> list;
 	if (t == ot_Association) {
 		UMLAssociation * obj=0;
 		for (obj = m_AssocsList.first(); obj != 0; obj = m_AssocsList.next()) {
 			if (obj->getBaseType() == t && obj -> getName() == n)
 				list.append( obj );
 		}
 	} else {
		kdWarning() << "unknown type in findChildObject()" << endl;
	}
	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLCanvasObject::findChildObject(int id) {
	UMLAssociation * asso = 0;
	for (asso = m_AssocsList.first(); asso != 0; asso = m_AssocsList.next()) {
		if (asso->getID() == id)
			return asso;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLCanvasObject::init() {
	m_AssocsList.clear();
	m_AssocsList.setAutoDelete(false);
	m_TmpAssocs.clear();
	m_TmpAssocs.setAutoDelete(false);
}

bool UMLCanvasObject::operator==(UMLCanvasObject& rhs) {
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

int UMLCanvasObject::associations() {
	return m_AssocsList.count();
}

const QPtrList<UMLAssociation>& UMLCanvasObject::getAssociations() {
	return m_AssocsList;
}

QPtrList<UMLAssociation> UMLCanvasObject::getGeneralizations() {
	return getSpecificAssocs(Uml::at_Generalization);
}

QPtrList<UMLAssociation> UMLCanvasObject::getAggregations() {
	return getSpecificAssocs(Uml::at_Aggregation);
}

QPtrList<UMLAssociation> UMLCanvasObject::getCompositions() {
	return getSpecificAssocs(Uml::at_Composition);
}
