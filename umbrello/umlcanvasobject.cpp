/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "umldoc.h"
#include "umlcanvasobject.h"
#include "classifier.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "template.h"
#include "stereotype.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLCanvasObject::UMLCanvasObject(const QString & name, Uml::IDType id)
   : UMLObject(name, id)
{
	init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLCanvasObject::~UMLCanvasObject() {
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAssociationList UMLCanvasObject::getSpecificAssocs(Uml::Association_Type assocType) {
	UMLAssociationList list;
	for (UMLAssociationListIt ait(m_AssocsList); ait.current(); ++ait) {
		UMLAssociation *a = ait.current();
		if (a->getAssocType() == assocType)
			list.append(a);
	}
	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLCanvasObject::addAssociation(UMLAssociation* assoc) {
	// add association only if not already present in list
	if(!hasAssociation(assoc))
	{
		m_AssocsList.append( assoc );
		emit modified();
		emit sigAssociationAdded(assoc);
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLCanvasObject::hasAssociation(UMLAssociation* assoc) {
	if(m_AssocsList.containsRef(assoc) > 0)
		return true;
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLCanvasObject::removeAssociation(UMLAssociation * assoc) {
	if(!hasAssociation(assoc) || !m_AssocsList.remove(assoc)) {
		kdWarning() << "can't find assoc given in list" << endl;
		return -1;
	}
	emit modified();
	emit sigAssociationRemoved(assoc);
	return m_AssocsList.count();
}

QString UMLCanvasObject::uniqChildName( const Uml::Object_Type type,
					bool seekStereo /* = false */ ) {
	QString currentName;
	if (seekStereo) {
		currentName = i18n("new_stereotype");
	} else if (type == Uml::ot_Association) {
		currentName = i18n("new_association");
	} else if (type == Uml::ot_Attribute) {
		currentName = i18n("new_attribute");
	} else if (type == Uml::ot_Template) {
		currentName = i18n("new_template");
	} else if (type == Uml::ot_Operation) {
		currentName = i18n("new_operation");
	} else if (type == Uml::ot_EnumLiteral) {
		currentName = i18n("new_literal");
	} else if (type == Uml::ot_EntityAttribute) {
		currentName = i18n("new_field");
	} else {
		kdWarning() << "uniqChildName() called for unknown child type " << type << endl;
		return "ERROR_in_UMLCanvasObject_uniqChildName";
	}

	QString name = currentName;
	for (int number = 1; findChildObject(type, name).count(); ++number) {
	        name = currentName + "_" + QString::number(number);
	}
	return name;
}

UMLObjectList UMLCanvasObject::findChildObject(Uml::Object_Type t, const QString &n) {
	UMLObjectList list;
	if (t == Uml::ot_Association) {
		for (UMLAssociationListIt ait(m_AssocsList); ait.current(); ++ait) {
			UMLAssociation *obj = ait.current();
			if (obj->getBaseType() != t)
				continue;
			if (obj->getName() == n)
				list.append( obj );
		}
	} else {
		kdWarning() << "unknown type in findChildObject()" << endl;
	}
	return list;
}

UMLObject* UMLCanvasObject::findChildObject(Uml::IDType id) {
	for (UMLAssociationListIt ait(m_AssocsList); ait.current(); ++ait) {
		UMLAssociation *asso = ait.current();
		if (asso->getID() == id)
			return asso;
	}
	return 0;
}

void UMLCanvasObject::init() {
	m_AssocsList.setAutoDelete(false);
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

void UMLCanvasObject::copyInto(UMLCanvasObject *rhs) const
{
	UMLObject::copyInto(rhs);

	// TODO Associations are not copied at the moment. This because
	// the duplicate function (on umlwidgets) do not copy the associations.
	//
	//rhs->m_AssocsList = m_AssocsList;
}

int UMLCanvasObject::associations() {
	return m_AssocsList.count();
}

const UMLAssociationList& UMLCanvasObject::getAssociations() {
	return m_AssocsList;
}

UMLClassifierList UMLCanvasObject::getSuperClasses() {
	UMLClassifierList list;
	for (UMLAssociation* a = m_AssocsList.first(); a; a = m_AssocsList.next()) {
		if ( a->getAssocType() != Uml::at_Generalization ||
		     a->getObjectId(Uml::A) != getID() )
			continue;
		UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::B));
		if (c)
			list.append(c);
		else
			kdDebug() << "UMLCanvasObject::getSuperClasses(" << m_Name
				  << "): generalization's other end is not a "
				  << "UMLClassifier (id= " << ID2STR(a->getObjectId(Uml::B)) << ")"
				  << endl;
	}
	return list;
}

UMLClassifierList UMLCanvasObject::getSubClasses() {
	UMLClassifierList list;
	// WARNING: See remark at getSuperClasses()
	for (UMLAssociation* a = m_AssocsList.first(); a; a = m_AssocsList.next()) {
		if ( a->getAssocType() != Uml::at_Generalization ||
		     a->getObjectId(Uml::B) != getID() )
			continue;
		UMLClassifier *c = dynamic_cast<UMLClassifier*>(a->getObject(Uml::A));
		if (c)
			list.append(c);
		else
			kdDebug() << "UMLCanvasObject::getSubClasses: specialization's"
				  << " other end is not a UMLClassifier"
				  << " (id=" << ID2STR(a->getObjectId(Uml::A)) << ")" << endl;
	}
	return list;
}

UMLAssociationList UMLCanvasObject::getRealizations() {
	return getSpecificAssocs(Uml::at_Realization);
}

UMLAssociationList UMLCanvasObject::getAggregations() {
	return getSpecificAssocs(Uml::at_Aggregation);
}

UMLAssociationList UMLCanvasObject::getCompositions() {
	return getSpecificAssocs(Uml::at_Composition);
}

UMLAssociationList UMLCanvasObject::getRelationships() {
	return getSpecificAssocs(Uml::at_Relationship);
}
#include "umlcanvasobject.moc"
