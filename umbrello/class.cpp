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
#include "umldoc.h"
#include "uml.h"
#include <kdebug.h>
#include <klocale.h>

UMLClass::UMLClass(const QString & name, int id) : UMLClassifier (name, id)
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
UMLAttribute* UMLClass::addAttribute(QString name, int id /* = -1 */) {
	UMLClassifierListItem *obj = NULL;
	for (obj = m_AttsList.first(); obj; obj = m_AttsList.next()) {
		if (obj->getBaseType() == ot_Attribute && obj->getName() == name)
			return static_cast<UMLAttribute*>(obj);
	}
	UMLDoc *umldoc = UMLApp::app()->getDocument();
	if (id == -1)
		id = umldoc->getUniqueID();
	Uml::Scope scope = umldoc->getOptionState().classState.defaultAttributeScope;
	UMLAttribute *a = new UMLAttribute(this, name, id, "int", scope);
	m_AttsList.append(a);
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
		if( position >= 0 && position <= (int)m_AttsList.count() )
			m_AttsList.insert(position,Att);
		else
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
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClass::removeAttribute(UMLObject* a) {
	if(!m_AttsList.remove((UMLAttribute *)a)) {
		kdDebug() << "can't find att given in list" << endl;
		return -1;
	}
	emit attributeRemoved(a);
	emit modified();
	// If we are deleteing the object, then we dont need to disconnect..this is done auto-magically
	// for us by QObject. -b.t.
	// disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	delete a;
	return m_AttsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute* UMLClass::takeAttribute(UMLAttribute* a) {
	int index = m_AttsList.findRef( a );
	a = (index == -1 ? 0 : dynamic_cast<UMLAttribute*>(m_AttsList.take( )));
	if (a) {
		emit attributeRemoved(a);
		emit modified();
	}
	return a;
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
int UMLClass::removeTemplate(UMLTemplate* umltemplate) {
	if ( !m_TemplateList.remove(umltemplate) ) {
		kdWarning() << "can't find att given in list" << endl;
		return -1;
	}
	emit templateRemoved(umltemplate);
	emit modified();
	disconnect(umltemplate,SIGNAL(modified()),this,SIGNAL(modified()));
	return m_TemplateList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLTemplate* UMLClass::takeTemplate(UMLTemplate* t) {
	int index = m_TemplateList.findRef( t );
	t = (index == -1 ? 0 : dynamic_cast<UMLTemplate*>(m_TemplateList.take( )));
	if (t) {
		emit templateRemoved(t);
		emit modified();
	}
	return t;
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
		kdWarning() << "uniqChildName() called for unknown child type " << type << endl;
		return "ERROR_in_UMLClass_uniqChildName";
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
	}
	if (t == ot_Template || t == ot_Stereotype) {
		UMLClassifierListItem* obj=0;
		for(obj=m_TemplateList.first(); obj != 0; obj=m_TemplateList.next()) {
			if (obj->getBaseType() == t && obj->getName() == n)
				list.append(obj);
		}
	}
	if (t == ot_Operation || t == ot_Stereotype) {
		UMLClassifierListItem* obj=0;
		for(obj=m_OpsList.first();obj != 0;obj=m_OpsList.next()) {
			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
	}
	if (t != ot_Association && t != ot_Attribute && t != ot_Template && t != ot_Operation && t != ot_Stereotype) {
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
	if (st == "CORBAEnum")
		return true;
	if (st.isEmpty() || st.at(0).upper() != 'E')
		return false;
	QString tail = st.right(st.length() - 1);
	return (tail == "num" || tail == "numeration");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLClass::init() {

	m_BaseType = ot_Class;
	setStereotype( i18n("class") );

	m_AttsList.setAutoDelete(false);
	m_TemplateList.setAutoDelete(false);

	UMLDoc * parent = UMLApp::app()->getDocument();
	connect(this,SIGNAL(attributeAdded(UMLObject*)),parent,SLOT(addUMLObject(UMLObject*)));
	connect(this,SIGNAL(attributeRemoved(UMLObject*)),parent,SLOT(slotRemoveUMLObject(UMLObject*)));
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

void UMLClass::copyInto(UMLClass *rhs) const
{
	UMLClassifier::copyInto(rhs);

	m_AttsList.copyInto(&(rhs->m_AttsList));
	m_TemplateList.copyInto(&(rhs->m_TemplateList));
}

UMLObject* UMLClass::clone() const
{
	UMLClass *clone = new UMLClass();
	copyInto(clone);

	return clone;
}

void UMLClass::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement classElement = UMLObject::save("UML:Class", qDoc);
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
}

bool UMLClass::loadSpecialized(QDomElement & element) {
	QString tag = element.tagName();
	if (tagEq(tag, "Attribute")) {
		UMLAttribute * pAtt = new UMLAttribute( this );
		if( !pAtt -> loadFromXMI( element ) )
			return false;
		addAttribute(pAtt);
		// connect( pAtt,SIGNAL(modified()),this,SIGNAL(modified()));
		// m_AttsList.append( pAtt );
	} else if (tag == "template") {
		//FIXME: Make UML DTD compliant.
		UMLTemplate* newTemplate = new UMLTemplate(this);
		if ( !newTemplate->loadFromXMI(element) ) {
			return false;
		}
		m_TemplateList.append(newTemplate);
	} else if (tag == "stereotype") {
		//FIXME: Make UML DTD compliant.
		UMLStereotype* newStereotype = new UMLStereotype(this);
		if ( !newStereotype->loadFromXMI(element) ) {
			return false;
		}
		QString listTypeString = element.attribute("listtype", "-1");
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
		kdWarning() << "UMLClass::load(" << getName()
			    << "): unknown child type " << tag << endl;
	}
	return true;
}

int UMLClass::templates() {
	return m_TemplateList.count();
}

int UMLClass::attributes() {
	return m_AttsList.count();
}

UMLClassifierListItemList* UMLClass::getAttList() {
	return &m_AttsList;
}

UMLAttributeList* UMLClass::getFilteredAttributeList() {
	UMLAttributeList* attributeList = new UMLAttributeList;
	for(UMLClassifierListItem* listItem = m_AttsList.first(); listItem;
	    listItem = m_AttsList.next())  {
		if (listItem->getBaseType() == ot_Attribute) {
			attributeList->append(static_cast<UMLAttribute*>(listItem));
		}
	}
	return attributeList;
}

UMLClassifierListItemList* UMLClass::getTemplateList() {
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
