 /*
  *  copyright (C) 2003-2004
  *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
  */

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

UMLClassifierListItem::UMLClassifierListItem(const UMLObject *parent, QString Name, Uml::IDType id)
  : UMLObject(parent, Name, id) {
}

UMLClassifierListItem::UMLClassifierListItem(const UMLObject *parent)
  : UMLObject(parent) {
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
	return static_cast<UMLClassifier*>(m_pSecondary);
}

QString UMLClassifierListItem::getTypeName() {
	if (m_pSecondary != NULL)
		return m_pSecondary->getName();
	return m_SecondaryId;
}

void UMLClassifierListItem::setType(UMLObject *type) {
	if (m_pSecondary != type) {
		m_pSecondary = type;
		emit modified();
	}
}

void UMLClassifierListItem::setTypeName(const QString &type) {
	if (type.isEmpty()) {
		m_pSecondary = NULL;
		m_SecondaryId = type;
		return;
	}
	UMLDoc *pDoc = UMLApp::app()->getDocument();
	m_pSecondary = pDoc->findUMLObject(type);
	if (m_pSecondary == NULL) {
		// Make data type for easily identified cases
		const int n_types = 12;
		const char *types[] = {
			"void", "bool",
			"char", "unsigned char",
			"short", "unsigned short",
			"int", "unsigned int",
			"long", "unsigned long",
			"float", "double"
		};
		int i = 0;
		for (; i < n_types; i++) {
			if (type == types[i])
				break;
		}
		if (i < n_types || type.contains('*')) {
			m_pSecondary = pDoc->createUMLObject(Uml::ot_Datatype, type);
			kdDebug() << "UMLClassifierListItem::setTypeName: "
				  << "created datatype for " << type << endl;
		} else {
			m_SecondaryId = type;
		}
	}
	emit modified();
}


#include "classifierlistitem.moc"
