 /*
  *  copyright (C) 2002-2004
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

#include "attribute.h"
#include "classifier.h"
#include "umldoc.h"
#include "uml.h"
#include "dialogs/umlattributedialog.h"

#include <qregexp.h>
#include <kdebug.h>

UMLAttribute::UMLAttribute( const UMLObject *parent, QString Name, Uml::IDType id,
			    Uml::Scope s, QString type, QString iv )
  : UMLClassifierListItem(parent, Name, id) {
	m_InitialValue = iv;
	m_BaseType = Uml::ot_Attribute;
	m_Scope = s;
	m_ParmKind = Uml::pd_In;
	if (!type.isEmpty()) {
		UMLDoc *pDoc = UMLApp::app()->getDocument();
		m_pSecondary = pDoc->findUMLObject(type);
		if (m_pSecondary == NULL) {
			if (type.contains( QRegExp("[\\*\\&]") ))
				m_pSecondary = pDoc->createUMLObject(Uml::ot_Datatype, type);
			else
				m_pSecondary = pDoc->createUMLObject(Uml::ot_Class, type);
		}
	}
}

UMLAttribute::UMLAttribute(const UMLObject *parent) : UMLClassifierListItem(parent) {
	m_BaseType = Uml::ot_Attribute;
	m_Scope = Uml::Private;
	m_ParmKind = Uml::pd_In;
}

UMLAttribute::~UMLAttribute() { }

QString UMLAttribute::getInitialValue() {
	return m_InitialValue;
}

void UMLAttribute::setInitialValue(const QString &iv) {
	if(m_InitialValue != iv) {
		m_InitialValue = iv;
		emit modified();
	}
}

void UMLAttribute::setParmKind (Uml::Parameter_Direction pk) {
	m_ParmKind = pk;
}

Uml::Parameter_Direction UMLAttribute::getParmKind () const {
	return m_ParmKind;
}

QString UMLAttribute::toString(Uml::Signature_Type sig) {
	QString s;

	if(sig == Uml::st_ShowSig || sig == Uml::st_NoSig) {
		if(m_Scope == Uml::Public)
			s = "+ ";
		else if(m_Scope == Uml::Private)
			s = "- ";
		else if(m_Scope == Uml::Protected)
			s= "# ";
	} else
		s = "";

	if(sig == Uml::st_ShowSig || sig == Uml::st_SigNoScope) {
		QString string = s + getName() + " : " + getTypeName();
		if(m_InitialValue.length() > 0)
			string += " = " + m_InitialValue;
		return string;
	} else
		return s + getName();
}

bool UMLAttribute::operator==( UMLAttribute &rhs) {
	if( this == &rhs )
		return true;

	if( !UMLObject::operator==( rhs ) )
		return false;

	// The type name is the only distinguishing criterion.
	// (Some programming languages might support more, but others don't.)
	if (m_pSecondary != rhs.m_pSecondary)
		return false;

	return true;
}

void UMLAttribute::copyInto(UMLAttribute *rhs) const
{
	// call the parent first.
	UMLClassifierListItem::copyInto(rhs);

	// Copy all datamembers
	rhs->m_pSecondary = m_pSecondary;
	rhs->m_SecondaryId = m_SecondaryId;
	rhs->m_InitialValue = m_InitialValue;
	rhs->m_ParmKind = m_ParmKind;
}

UMLObject* UMLAttribute::clone() const
{
	UMLAttribute *clone = new UMLAttribute( (UMLAttribute *)parent() );
	copyInto(clone);

	return clone;
}


void UMLAttribute::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement attributeElement = UMLObject::save("UML:Attribute", qDoc);
	if (m_pSecondary == NULL) {
		kdDebug() << "UMLAttribute::saveToXMI(" << m_Name
			  << "): m_pSecondary is NULL, using local name "
			  << m_SecondaryId << endl;
		attributeElement.setAttribute( "type", m_SecondaryId );
	} else {
		attributeElement.setAttribute( "type", ID2STR(m_pSecondary->getID()) );
	}
	attributeElement.setAttribute( "initialValue", m_InitialValue );
	qElement.appendChild( attributeElement );
}

bool UMLAttribute::load( QDomElement & element ) {
	m_SecondaryId = element.attribute( "type", "" );
	// We use the m_SecondaryId as a temporary store for the xmi.id
	// of the attribute type model object.
	// It is resolved later on, when all classes have been loaded.
	// This deferred resolution is required because the xmi.id may
	// be a forward reference, i.e. it may identify a model object
	// that has not yet been loaded.
	if (m_SecondaryId.isEmpty()) {
		// Perhaps the type is stored in a child node:
		QDomNode node = element.firstChild();
		while (!node.isNull()) {
			if (node.isComment()) {
				node = node.nextSibling();
				continue;
			}
			QDomElement tempElement = node.toElement();
			QString tag = tempElement.tagName();
			if (!Uml::tagEq(tag, "type")) {
				node = node.nextSibling();
				continue;
			}
			m_SecondaryId = tempElement.attribute( "xmi.id", "" );
			if (m_SecondaryId.isEmpty())
				m_SecondaryId = tempElement.attribute( "xmi.idref", "" );
			if (m_SecondaryId.isEmpty()) {
				QDomNode inner = node.firstChild();
				QDomElement tmpElem = inner.toElement();
				m_SecondaryId = tmpElem.attribute( "xmi.id", "" );
				if (m_SecondaryId.isEmpty())
					m_SecondaryId = tmpElem.attribute( "xmi.idref", "" );
			}
			break;
		}
		if (m_SecondaryId.isEmpty()) {
			kdError() << "UMLAttribute::load(" << m_Name << "): "
				  << "cannot find type." << endl;
			return false;
		}
	}
	m_InitialValue = element.attribute( "initialValue", "" );
	if (m_InitialValue.isEmpty()) {
		// for backward compatibility
		m_InitialValue = element.attribute( "value", "" );
	}
	return true;
}

bool UMLAttribute::showPropertiesDialogue(QWidget* parent) {
	UMLAttributeDialog dialogue(parent, this);
	return dialogue.exec();
}



