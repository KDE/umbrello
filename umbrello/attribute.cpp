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

UMLAttribute::UMLAttribute( const UMLObject *parent, QString Name, int id,
			    Scope s, QString type, QString iv )
  : UMLClassifierListItem(parent, Name, id) {
	m_InitialValue = iv;
	m_BaseType = ot_Attribute;
	m_Scope = s;
	m_ParmKind = pk_In;
	m_TypeName = type;
	UMLDoc *pDoc = UMLApp::app()->getDocument();
	UMLObject *typeObj = pDoc->findUMLObject(type);
	if (typeObj == NULL) {
		if (type.contains( QRegExp("\\W") ))
			typeObj = pDoc->createUMLObject(ot_Datatype, type);
		else
			typeObj = pDoc->createUMLObject(ot_Class, type);
	}
	UMLClassifierListItem::m_pType = static_cast<UMLClassifier*>(typeObj);
}

UMLAttribute::UMLAttribute(const UMLObject *parent) : UMLClassifierListItem(parent) {
	m_BaseType = ot_Attribute;
	m_Scope = Private;
	m_ParmKind = pk_In;
}

UMLAttribute::~UMLAttribute() { }

QString UMLAttribute::getInitialValue() {
	return m_InitialValue;
}

void UMLAttribute::setInitialValue(QString iv) {
	if(m_InitialValue != iv) {
		m_InitialValue = iv;
		emit modified();
	}
}

void UMLAttribute::setParmKind (Uml::Parameter_Kind pk) {
	m_ParmKind = pk;
}

Uml::Parameter_Kind UMLAttribute::getParmKind () const {
	return m_ParmKind;
}

QString UMLAttribute::toString(Signature_Type sig) {
	QString s;

	if(sig == st_ShowSig || sig == st_NoSig) {
		if(m_Scope == Public)
			s = "+ ";
		else if(m_Scope == Private)
			s = "- ";
		else if(m_Scope == Protected)
			s= "# ";
	} else
		s = "";

	if(sig == st_ShowSig || sig == st_SigNoScope) {
		QString string = s + getName() + " : " + m_TypeName;
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
	if (m_pType != rhs.m_pType)
		return false;
	if( m_TypeName != rhs.m_TypeName )
		return false;

	return true;
}

void UMLAttribute::copyInto(UMLAttribute *rhs) const
{
	// call the parent first.
	UMLClassifierListItem::copyInto(rhs);

	// Copy all datamembers
	rhs->m_pType = m_pType;
	rhs->m_TypeName = m_TypeName;
	rhs->m_InitialValue = m_InitialValue;
	rhs->m_ParmKind = m_ParmKind;
}

UMLObject* UMLAttribute::clone() const
{
	UMLAttribute *clone = new UMLAttribute( (UMLAttribute *)parent() );
	copyInto(clone);

	return clone;
}


bool UMLAttribute::resolveType() {
	UMLDoc *pDoc = UMLApp::app()->getDocument();
	if (m_TypeName.contains(QRegExp("\\D"))) {
		// Check whether this is a foreign XMI file.
		UMLObject *typeObj = pDoc->findObjectByIdStr( m_TypeName );
		if (typeObj == NULL) {
			// We're dealing with the older Umbrello format where the
			// attribute type name was saved in the "type" rather than the
			// xmi.id of the model object of the attribute type.
			kdDebug() << "UMLAttribute::resolveType: Creating new type for "
				  << m_TypeName << endl;
			if (m_TypeName.contains( QRegExp("\\W") ))
				typeObj = pDoc->createUMLObject(ot_Datatype, m_TypeName);
			else
				typeObj = pDoc->createUMLObject(ot_Class, m_TypeName);
		} else {
			// It's not an Umbrello format.
			m_pType = dynamic_cast<UMLClassifier*>(typeObj);
			if (m_pType == NULL) {
				kdError() << "UMLAttribute::resolveType(" << m_Name
					  << "): type with id " << m_TypeName
					  << " is not a UMLClassifier" << endl;
				return false;
			}
			m_TypeName = "";
		}
	} else {
		// New, XMI standard compliant save format:
		// The type is the xmi.id of a UMLClassifier.
		int id = m_TypeName.toInt();
		UMLObject *typeObj = pDoc->findUMLObject(id);
		if (typeObj == NULL) {
			kdError() << "UMLAttribute::resolveType(" << m_Name
				  << "): cannot find type with id "
				  << id << endl;
			return false;
		}
		m_pType = dynamic_cast<UMLClassifier*>(typeObj);
		if (m_pType == NULL) {
			kdError() << "UMLAttribute::resolveType(" << m_Name
				  << "): type with id " << id
				  << " is not a UMLClassifier" << endl;
			return false;
		}
		m_TypeName = "";
	}
	return true;
}

void UMLAttribute::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement attributeElement = UMLObject::save("UML:Attribute", qDoc);
	if (m_pType == NULL) {
		kdDebug() << "UMLAttribute::saveToXMI: m_pType is NULL, using "
			  << "local name " << m_TypeName << endl;
		attributeElement.setAttribute( "type", m_TypeName );
	} else {
		attributeElement.setAttribute( "type", m_pType->getID() );
	}
	attributeElement.setAttribute( "initialValue", m_InitialValue );
	qElement.appendChild( attributeElement );
}

bool UMLAttribute::load( QDomElement & element ) {
	m_TypeName = element.attribute( "type", "" );
	// We use the m_TypeName as a temporary store for the xmi.id
	// of the attribute type model object.
	// It is resolved later on, when all classes have been loaded.
	// This deferred resolution is required because the xmi.id may
	// be a forward reference, i.e. it may identify a model object
	// that has not yet been loaded.

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



