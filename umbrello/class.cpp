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

#include "class.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "stereotype.h"
#include "classifierlistitem.h"
#include "template.h"
#include "clipboard/idchangelog.h"
#include "uml.h"
#include <kdebug.h>
#include <klocale.h>

UMLClass::UMLClass(const QString & name, Uml::IDType id) : UMLClassifier (name, id)
{
	init();
}


UMLClass::~UMLClass() {
	// The parent (UMLClassifier) destructor is called automatically (C++)
	// Also, no need for explicitly clear()ing any lists - the QList
	// destructor does this for us. (Similarly, the QList constructor
	// already gives us clear()ed lists.)
}

UMLAttribute* UMLClass::addAttribute(const QString &name, Uml::IDType id /* = Uml::id_None */) {
	for (UMLClassifierListItemListIt lit(m_List); lit.current(); ++lit) {
		UMLClassifierListItem *obj = lit.current();
		if (obj->getBaseType() == Uml::ot_Attribute && obj->getName() == name)
			return static_cast<UMLAttribute*>(obj);
	}
	UMLApp *app = UMLApp::app();
	Uml::Scope scope = app->getOptionState().classState.defaultAttributeScope;
	UMLAttribute *a = new UMLAttribute(this, name, id, scope);
	m_List.append(a);
	emit modified();
	connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	emit attributeAdded(a);
	return a;
}

UMLAttribute* UMLClass::addAttribute(const QString &name, UMLObject *type, Uml::Scope scope) {
	UMLAttribute *a = new UMLAttribute(this, name, Uml::id_None, scope);
	if (type)
		a->setType(type);
	m_List.append(a);
	emit modified();
	connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	emit attributeAdded(a);
	return a;
}

bool UMLClass::addAttribute(UMLAttribute* Att, IDChangeLog* Log /* = 0 */,
			    int position /* = -1 */) {
	QString name = (QString)Att->getName();
	if( findChildObject( Uml::ot_Attribute, name).count() == 0 ) {
		Att -> parent() -> removeChild( Att );
		this -> insertChild( Att );
		if( position >= 0 && position <= (int)m_List.count() )
			m_List.insert(position,Att);
		else
			m_List.append( Att );
		emit modified();
		connect(Att,SIGNAL(modified()),this,SIGNAL(modified()));
		emit attributeAdded(Att);
		return true;
	} else if( Log ) {
		Log->removeChangeByNewID( Att -> getID() );
		delete Att;
	}
	return false;
}

int UMLClass::removeAttribute(UMLObject* a) {
	if(!m_List.remove((UMLAttribute *)a)) {
		kdDebug() << "can't find att given in list" << endl;
		return -1;
	}
	emit attributeRemoved(a);
	emit modified();
	// If we are deleteing the object, then we dont need to disconnect..this is done auto-magically
	// for us by QObject. -b.t.
	// disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	delete a;
	return m_List.count();
}

UMLAttribute* UMLClass::takeAttribute(UMLAttribute* a) {
	int index = m_List.findRef( a );
	a = (index == -1 ? 0 : dynamic_cast<UMLAttribute*>(m_List.take( )));
	if (a) {
		emit attributeRemoved(a);
		emit modified();
	}
	return a;
}

bool UMLClass::isEnumeration() {
	QString st = getStereotype();
	return st.contains("enum", false);
}

void UMLClass::init() {
	m_BaseType = Uml::ot_Class;
}

bool UMLClass::operator==( UMLClass & rhs ) {
	return UMLClassifier::operator==(rhs);
}

void UMLClass::copyInto(UMLClass *rhs) const
{
	UMLClassifier::copyInto(rhs);
}

UMLObject* UMLClass::clone() const
{
	UMLClass *clone = new UMLClass();
	copyInto(clone);

	return clone;
}

void UMLClass::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement classElement = UMLObject::save("UML:Class", qDoc);

	// save templates
	UMLClassifier::saveToXMI(qDoc, classElement);

	// save attributes and operations
	QDomElement featureElement = qDoc.createElement( "UML:Classifier.feature" );
	UMLClassifierListItemList list = getFilteredList(Uml::ot_Attribute);
	for (UMLClassifierListItem *pAtt = list.first(); pAtt; pAtt = list.next() )
		pAtt -> saveToXMI( qDoc, featureElement );
	UMLOperationList opList = getOpList();
	for (UMLOperation *pOp = opList.first(); pOp; pOp = opList.next() )
		pOp -> saveToXMI( qDoc, featureElement );
	if (featureElement.hasChildNodes())
		classElement.appendChild( featureElement );

	// save contained objects
	if (m_objects.count()) {
		QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );
		for (UMLObjectListIt oit(m_objects); oit.current(); ++oit) {
			UMLObject *obj = oit.current();
			obj->saveToXMI (qDoc, ownedElement);
		}
		classElement.appendChild( ownedElement );
	}
	qElement.appendChild( classElement );
}

bool UMLClass::loadSpecialized(QDomElement & element) {
	QString tag = element.tagName();
	if (Uml::tagEq(tag, "Attribute")) {
		UMLAttribute * pAtt = new UMLAttribute( this );
		if (!pAtt -> loadFromXMI( element )) {
			delete pAtt;
			return false;
		}
		addAttribute(pAtt);
		return true;
	} else if (tag == "template") { // for backward compatibility
		// DEPRECATED : Remove this code after the 1.4 release
		UMLTemplate* newTemplate = new UMLTemplate(this);
		if ( !newTemplate->loadFromXMI(element) ) {
			delete newTemplate;
			return false;
		}
		addTemplate(newTemplate);
		return true;
	} else if (tag == "stereotype") {  // sorry, no longer supported
		kdDebug() << "UMLClass::loadSpecialized(" << m_Name
			  << "): losing old-format stereotype." << endl;
		return true;
	}
	return false;
}

int UMLClass::attributes() {
	UMLClassifierListItemList atts = getFilteredList(Uml::ot_Attribute);
	return atts.count();
}

UMLAttributeList UMLClass::getAttributeList() {
	UMLAttributeList attributeList;
	for (UMLClassifierListItemListIt lit(m_List); lit.current(); ++lit) {
		UMLClassifierListItem *listItem = lit.current();
		if (listItem->getBaseType() == Uml::ot_Attribute) {
			attributeList.append(static_cast<UMLAttribute*>(listItem));
		}
	}
	return attributeList;
}


#include "class.moc"
