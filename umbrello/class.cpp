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

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClass::~UMLClass() {
	// The parent (UMLClassifier) destructor is called automatically (C++)
	// Also, no need for explicitly clear()ing any lists - the QList
	// destructor does this for us. (Similarly, the QList constructor
	// already gives us clear()ed lists.)
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute* UMLClass::addAttribute(QString name, Uml::IDType id /* = Uml::id_None */) {
	UMLClassifierListItem *obj = NULL;
	for (obj = m_List.first(); obj; obj = m_List.next()) {
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
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute* UMLClass::takeAttribute(UMLAttribute* a) {
	int index = m_List.findRef( a );
	a = (index == -1 ? 0 : dynamic_cast<UMLAttribute*>(m_List.take( )));
	if (a) {
		emit attributeRemoved(a);
		emit modified();
	}
	return a;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLClass::addTemplate(QString name, Uml::IDType id) {
	UMLTemplate* newTemplate = new UMLTemplate(this, name, id);
	m_List.append(newTemplate);
	emit modified();
	connect(newTemplate,SIGNAL(modified()),this,SIGNAL(modified()));
	emit templateAdded(newTemplate);
	return newTemplate;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClass::addTemplate(UMLTemplate* newTemplate, IDChangeLog* log /* = 0*/) {
	QString name = newTemplate->getName();
	if (findChildObject(Uml::ot_Template, name).count() == 0) {
		newTemplate->parent()->removeChild(newTemplate);
		this->insertChild(newTemplate);
		m_List.append(newTemplate);
		emit modified();
		connect(newTemplate,SIGNAL(modified()),this,SIGNAL(modified()));
		emit templateAdded(newTemplate);
		return true;
	} else if (log) {
		log->removeChangeByNewID( newTemplate->getID() );
		delete newTemplate;
	}
	return false;
}
bool UMLClass::addTemplate(UMLTemplate* Template, int position)
{
	QString name = Template->getName();
	if (findChildObject(Uml::ot_Template, name).count() == 0) {
		Template->parent()->removeChild(Template);
		this->insertChild(Template);
		if( position >= 0 && position <= (int)m_List.count() )
			m_List.insert(position,Template);
		else
			m_List.append(Template);
		emit modified();
		connect(Template,SIGNAL(modified()),this,SIGNAL(modified()));
		emit templateAdded(Template);
		return true;
	}
	//else
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClass::removeTemplate(UMLTemplate* umltemplate) {
	if ( !m_List.remove(umltemplate) ) {
		kdWarning() << "can't find att given in list" << endl;
		return -1;
	}
	emit templateRemoved(umltemplate);
	emit modified();
	disconnect(umltemplate,SIGNAL(modified()),this,SIGNAL(modified()));
	return m_List.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLTemplate* UMLClass::takeTemplate(UMLTemplate* t) {
	int index = m_List.findRef( t );
	t = (index == -1 ? 0 : dynamic_cast<UMLTemplate*>(m_List.take( )));
	if (t) {
		emit templateRemoved(t);
		emit modified();
	}
	return t;
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
	QDomElement featureElement = qDoc.createElement( "UML:Classifier.feature" );
	//save operations
	UMLOperationList opList = getOpList();
	UMLOperation * pOp = 0;
	for( pOp = opList.first(); pOp != 0; pOp = opList.next() )
		pOp -> saveToXMI( qDoc, featureElement );
	UMLClassifierListItemList list;
	//save attributes
	list = getFilteredList(Uml::ot_Attribute);
	UMLClassifierListItem * pAtt = 0;
	for( pAtt = list.first(); pAtt != 0; pAtt = list.next() )
		pAtt -> saveToXMI( qDoc, featureElement );
	//save templates
	list = getFilteredList(Uml::ot_Template);
	UMLClassifierListItem* newTemplate = 0;
	for (newTemplate = list.first(); newTemplate != 0; newTemplate = list.next() ) {
		newTemplate->saveToXMI(qDoc, featureElement);
	}
	//save contained objects
	for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
		obj->saveToXMI (qDoc, featureElement);
	if (featureElement.hasChildNodes())
		classElement.appendChild( featureElement );
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
	} else if (tag == "template") {
		//FIXME: Make UML DTD compliant.
		UMLTemplate* newTemplate = new UMLTemplate(this);
		if ( !newTemplate->loadFromXMI(element) ) {
			delete newTemplate;
			return false;
		}
		addTemplate(newTemplate);
		return true;
	} else if (tag == "stereotype") {
		kdDebug() << "UMLClass::loadSpecialized(" << m_Name
			  << "): losing old-format stereotype." << endl;
		return true;
	}
	return false;
}

int UMLClass::templates() {
	UMLClassifierListItemList tempList = getFilteredList(Uml::ot_Template);
	return tempList.count();
}

int UMLClass::attributes() {
	UMLClassifierListItemList atts = getFilteredList(Uml::ot_Attribute);
	return atts.count();
}

UMLAttributeList UMLClass::getFilteredAttributeList() {
	UMLAttributeList attributeList;
	for (UMLClassifierListItemListIt lit(m_List); lit.current(); ++lit) {
		UMLClassifierListItem *listItem = lit.current();
		if (listItem->getBaseType() == Uml::ot_Attribute) {
			attributeList.append(static_cast<UMLAttribute*>(listItem));
		}
	}
	return attributeList;
}

UMLTemplateList UMLClass::getFilteredTemplateList() {
	UMLTemplateList templateList;
	for(UMLClassifierListItem* listItem = m_List.first(); listItem;
	    listItem = m_List.next())  {
		if (listItem->getBaseType() == Uml::ot_Template) {
			templateList.append(static_cast<UMLTemplate*>(listItem));
		}
	}
	return templateList;
}



#include "class.moc"
