/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>

#include "classifierlistitem.h"
#include "classifier.h"
#include "uml.h"
#include "umldoc.h"

UMLClassifierListItem::UMLClassifierListItem(const UMLObject *parent, QString Name, int id)
  : UMLObject(parent, Name, id) {
	m_pType = NULL;
}

UMLClassifierListItem::UMLClassifierListItem(const UMLObject *parent)
  : UMLObject(parent) {
	m_pType = NULL;
}

UMLClassifierListItem::~UMLClassifierListItem() {
}

void UMLClassifierListItem::copyInto(UMLClassifierListItem *rhs) const
{
	// Call the parent.
	UMLObject::copyInto(rhs);
}

QString UMLClassifierListItem::toString(Uml::Signature_Type /*sig*/) {
	return getName();
}

UMLClassifier * UMLClassifierListItem::getType() {
	return m_pType;
}

QString UMLClassifierListItem::getTypeName() {
	if (m_pType != NULL)
		return m_pType->getName();
	return m_TypeName;
}

void UMLClassifierListItem::setType(UMLClassifier *type) {
	if (m_pType != type) {
		m_pType = type;
		emit modified();
	}
}

void UMLClassifierListItem::setTypeName(QString type) {
	if(m_TypeName != type) {
		m_TypeName = type;
		emit modified();
	}
}


#include "classifierlistitem.moc"
