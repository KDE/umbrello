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
#include <kdebug.h>
#include <klocale.h>

UMLClass::UMLClass(QObject * parent, QString Name, int id) : UMLClassifier (parent,Name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClass::UMLClass(QObject * parent) : UMLClassifier (parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClass::~UMLClass() {
	// we should just call the parent (UMLClassifier) method here instead of being explicit
  	m_AttsList.clear();
  	m_TemplateList.clear();
  	m_OpsList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLClass::addAttribute(QString name, int id) {
  	UMLAttribute *a = new UMLAttribute(this, name, id);
  	m_AttsList.append(a);
	emit modified();
	connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	emit attributeAdded(a);
	return a;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClass::addAttribute(UMLAttribute* Att, IDChangeLog* Log /* = 0*/) {
	QString name = (QString)Att->getName();
	if( findChildObject( Uml::ot_Attribute, name).count() == 0 ) {
		Att -> parent() -> removeChild( Att );
		this -> insertChild( Att );
		m_AttsList.append( Att );
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
bool UMLClass::addAttribute(UMLAttribute* Att, int position )
{
	QString name = (QString)Att->getName();
	if( findChildObject( Uml::ot_Attribute, name).count() == 0 )
	{
		Att -> parent() -> removeChild( Att );
		this -> insertChild( Att );
		if( position >= 0 && position <= (int)m_AttsList.count() )
			m_AttsList.insert(position,Att);
		else
			m_AttsList.append( Att );
		emit modified();
		connect(Att,SIGNAL(modified()),this,SIGNAL(modified()));
		emit attributeAdded(Att);
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClass::removeAttribute(UMLObject* a) {
	if(!m_AttsList.remove((UMLAttribute *)a)) {
		kdDebug() << "can't find att given in list" << endl;
		return -1;
	}
	emit attributeRemoved(a);
	emit modified();
	disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	return m_AttsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLClass::addTemplate(QString name, int id) {
  	UMLTemplate* newTemplate = new UMLTemplate(this, name, id);
  	m_TemplateList.append(newTemplate);
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
		m_TemplateList.append(newTemplate);
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
		if( position >= 0 && position <= (int)m_TemplateList.count() )
			m_TemplateList.insert(position,Template);
		else
			m_TemplateList.append(Template);
		emit modified();
		connect(Template,SIGNAL(modified()),this,SIGNAL(modified()));
		emit templateAdded(Template);
		return true;
	}
	//else
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClass::removeTemplate(UMLTemplate *temp) {
	if ( !m_TemplateList.remove(temp) ) {
		kdWarning() << "can't find att given in list" << endl;
		return -1;
	}
	emit templateRemoved(temp);
	emit modified();
	disconnect(temp,SIGNAL(modified()),this,SIGNAL(modified()));
	return m_TemplateList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClass::addStereotype(UMLStereotype* newStereotype, UMLObject_Type list, IDChangeLog* log /* = 0*/) {
	QString name = newStereotype->getName();
	if (findChildObject(Uml::ot_Template, name).count() == 0) {
		newStereotype->parent()->removeChild(newStereotype);
		this->insertChild(newStereotype);
		if (list == ot_Attribute) {
			m_AttsList.append(newStereotype);
			emit modified();
			connect(newStereotype, SIGNAL(modified()), this, SIGNAL(modified()));
		} else if (list == ot_Operation) {
			m_OpsList.append(newStereotype);
			emit modified();
			connect(newStereotype, SIGNAL(modified()), this, SIGNAL(modified()));
		} else if (list == ot_Template) {
			m_TemplateList.append(newStereotype);
			emit modified();
			connect(newStereotype, SIGNAL(modified()), this, SIGNAL(modified()));
		} else {
			kdWarning() << "unknown list type in addStereotype()" << endl;
		}
		return true;
	} else if (log) {
		log->removeChangeByNewID( newStereotype->getID() );
		delete newStereotype;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLClass::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		return UMLCanvasObject::uniqChildName(type);
	} else if (type == ot_Attribute) {
		currentName = i18n("new_attribute");
	} else if (type == ot_Template) {
		currentName = i18n("new_template");
	} else if (type == ot_Operation) {
		currentName = i18n("new_operation");
	} else if (type == ot_Stereotype) {
		currentName = i18n("new_stereotype");
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
UMLObjectList UMLClass::findChildObject(UMLObject_Type t , QString n) {
  	UMLObjectList list;
 	if (t == ot_Association) {
		return UMLClassifier::findChildObject(t, n);
 	} else if (t == ot_Attribute || t == ot_Stereotype) {
  		UMLClassifierListItem * obj=0;
  		for(obj=m_AttsList.first();obj != 0;obj=m_AttsList.next()) {
  			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
 	} else if (t == ot_Template || t == ot_Stereotype) {
  		UMLClassifierListItem* obj=0;
  		for(obj=m_TemplateList.first(); obj != 0; obj=m_TemplateList.next()) {
  			if (obj->getBaseType() == t && obj->getName() == n)
				list.append(obj);
		}
	} else if (t == ot_Operation || t == ot_Stereotype) {
		UMLClassifierListItem* obj=0;
		for(obj=m_OpsList.first();obj != 0;obj=m_OpsList.next()) {
			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
	} else {
		kdWarning() << "finding child object of unknown type" <<t<<" (requested name = "<<n<<")"<<endl;
	}

	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLClass::findChildObject(int id) {
	UMLClassifierListItem * a = 0;
	for(a=m_AttsList.first();a != 0;a=m_AttsList.next()) {
		if(a->getID() == id)
			return a;
	}
	UMLClassifierListItem* t = 0;
	for (t=m_TemplateList.first();t != 0;t=m_TemplateList.next()) {
		if (t->getID() == id) {
			return t;
		}
	}
	return UMLClassifier::findChildObject(id);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClass::isEnumeration() {
	QString st = getStereotype();
	return st == "Enumeration" || st == "enumeration" || st == "CORBAEnum";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLClass::init() {

	UMLClassifier::init(); // call super-class init

	m_BaseType = ot_Class;
	setStereotype( i18n("class") );

	m_AttsList.clear();
	m_AttsList.setAutoDelete(false);
	m_TemplateList.clear();
	m_TemplateList.setAutoDelete(false);
}

bool UMLClass::operator==( UMLClass & rhs ) {
	if ( m_AttsList.count() != rhs.m_AttsList.count() ) {
		return false;
	}
	if ( m_TemplateList.count() != rhs.m_TemplateList.count() ) {
		return false;
	}
	if ( &m_AttsList != &(rhs.m_AttsList) ) {
		return false;
	}
	if ( &m_TemplateList != &(rhs.m_TemplateList) ) {
		return false;
	}
	return UMLClassifier::operator==(rhs);
}

bool UMLClass::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement classElement = qDoc.createElement( "UML:Class" );
	bool status = UMLObject::saveToXMI( qDoc, classElement );
	//save operations
	UMLClassifierListItem * pOp = 0;
	for( pOp = m_OpsList.first(); pOp != 0; pOp = m_OpsList.next() )
		pOp -> saveToXMI( qDoc, classElement );
	//save attributes
	UMLClassifierListItem * pAtt = 0;
	for( pAtt = m_AttsList.first(); pAtt != 0; pAtt = m_AttsList.next() )
		pAtt -> saveToXMI( qDoc, classElement );
	//save templates
	UMLClassifierListItem* newTemplate = 0;
	for (newTemplate = m_TemplateList.first(); newTemplate != 0; newTemplate = m_TemplateList.next() ) {
		newTemplate->saveToXMI(qDoc, classElement);
	}

	qElement.appendChild( classElement );
	return status;
}

bool UMLClass::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI( element ) ) {
		return false;
	}

	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();
	while( !tempElement.isNull() ) {
		QString tag = tempElement.tagName();
		if( tag == "UML:Operation" ) {
			UMLOperation * pOp = new UMLOperation( this );
			if( !pOp -> loadFromXMI( tempElement ) )
				return false;
			m_OpsList.append( pOp );
		} else if( tag == "UML:Attribute" ) {
			UMLAttribute * pAtt = new UMLAttribute( this );
			if( !pAtt -> loadFromXMI( tempElement ) )
				return false;
			m_AttsList.append( pAtt );
			connect( pAtt,SIGNAL(modified()),this,SIGNAL(modified()));
		} else if (tag == "template") {
			UMLTemplate* newTemplate = new UMLTemplate(this);
			if ( !newTemplate->loadFromXMI(tempElement) ) {
				return false;
			}
			m_TemplateList.append(newTemplate);
		} else if (tag == "stereotype") {
			UMLStereotype* newStereotype = new UMLStereotype(this);
			if ( !newStereotype->loadFromXMI(tempElement) ) {
				return false;
			}
			QString listTypeString = tempElement.attribute("listtype", "-1");
			UMLObject_Type listType = (UMLObject_Type)listTypeString.toInt();
			if (listType == ot_Attribute) {
				m_AttsList.append(newStereotype);
			} else if (listType == ot_Operation) {
				m_OpsList.append(newStereotype);
			} else if (listType == ot_Template) {
				m_TemplateList.append(newStereotype);
			} else {
				kdWarning() << "unknown listtype with stereotype:" << listType << endl;
			}
		} else {
			kdWarning() << "loading unknown child type in UMLClass::loadFromXMI" << endl;
		}
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while
	return true;
}

int UMLClass::templates() {
	return m_TemplateList.count();
}

int UMLClass::attributes() {
	return m_AttsList.count();
}

QPtrList<UMLClassifierListItem>* UMLClass::getAttList() {
	return &m_AttsList;
}

QPtrList<UMLAttribute>* UMLClass::getFilteredAttributeList() {
	QPtrList<UMLAttribute>* attributeList = new QPtrList<UMLAttribute>;
	for(UMLClassifierListItem* listItem = m_AttsList.first(); listItem;
	    listItem = m_AttsList.next())  {
		if (listItem->getBaseType() == ot_Attribute) {
			attributeList->append(static_cast<UMLAttribute*>(listItem));
		}
	}
	return attributeList;
}

QPtrList<UMLClassifierListItem>* UMLClass::getTemplateList() {
	return &m_TemplateList;
}

QPtrList<UMLTemplate>* UMLClass::getFilteredTemplateList() {
	QPtrList<UMLTemplate>* templateList = new QPtrList<UMLTemplate>;
	for(UMLClassifierListItem* listItem = m_TemplateList.first(); listItem;
	    listItem = m_TemplateList.next())  {
		if (listItem->getBaseType() == ot_Template) {
			templateList->append(static_cast<UMLTemplate*>(listItem));
		}
	}
	return templateList;
}

#include "class.moc"
