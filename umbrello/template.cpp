/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "template.h"
#include "dialogs/umltemplatedialog.h"
#include <kdebug.h>

UMLTemplate::UMLTemplate(const UMLObject *parent, QString name, int id, QString type)
  : UMLClassifierListItem( parent, name, id ) {
	m_TypeName = type;
	m_BaseType = Uml::ot_Template;
}

UMLTemplate::UMLTemplate(const UMLObject *parent)
  : UMLClassifierListItem( parent ) {
	m_BaseType = Uml::ot_Template;
	m_TypeName = "";
}

UMLTemplate::~UMLTemplate() {}

QString UMLTemplate::toString(Uml::Signature_Type /*sig = st_NoSig*/) {
	if (m_TypeName == "" || m_TypeName == "class") {
		return getName();
	} else {
		return getName() + " : " + m_TypeName;
	}
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

void UMLTemplate::copyInto(UMLTemplate *rhs) const
{
	UMLClassifierListItem::copyInto(rhs);

	rhs->m_TypeName = m_TypeName;
}

UMLObject* UMLTemplate::clone() const
{
	UMLTemplate *clone = new UMLTemplate( (UMLTemplate*) parent());
	copyInto(clone);

	return clone;
}


void UMLTemplate::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	//FIXME: uml13.dtd compliance
	QDomElement attributeElement = UMLObject::save("template", qDoc);
	attributeElement.setAttribute("type", m_TypeName);
	qElement.appendChild(attributeElement);
}

bool UMLTemplate::load(QDomElement& element) {
	m_TypeName = element.attribute("type", "");
	return true;
}

bool UMLTemplate::showPropertiesDialogue(QWidget* parent) {
	UMLTemplateDialog dialogue(parent, this);
	return dialogue.exec();
}
