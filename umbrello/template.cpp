/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "template.h"
#include "class.h"
#include "dialogs/umltemplatedialog.h"
#include <kdebug.h>

UMLTemplate::UMLTemplate(UMLClass* parent, QString name, int id, QString type)
  : UMLClassifierListItem( parent->getParentUMLDoc(), name, id ) {
	m_TypeName = type;
	m_BaseType = ot_Template;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLTemplate::UMLTemplate(UMLClass* parent)
  : UMLClassifierListItem( parent->getParentUMLDoc() ) {
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
QString UMLTemplate::toString(Signature_Type /*sig = st_NoSig*/) {
	if (m_TypeName == "" || m_TypeName == "class") {
		return getName();
	} else {
		return getName() + " : " + m_TypeName;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
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

bool UMLTemplate::showPropertiesDialogue(QWidget* parent) {
	UMLTemplateDialog dialogue(parent, this);
	return dialogue.exec();
}
