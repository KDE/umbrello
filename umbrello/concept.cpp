/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "concept.h"
#include "association.h"
#include "attribute.h"
#include "operation.h"
#include "template.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLConcept::UMLConcept(QObject * parent, QString Name, int id) : UMLCanvasObject(parent,Name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLConcept::UMLConcept(QObject * parent) : UMLCanvasObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLConcept::~UMLConcept() {
  	m_AttsList.clear();
  	m_TemplateList.clear();
  	m_OpsList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLConcept::addAttribute(QString name, int id) {
  	UMLAttribute *a = new UMLAttribute(this, name, id);
  	m_AttsList.append(a);
	emit modified();
	return a;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::addAttribute(UMLAttribute* Att, IDChangeLog* Log /* = 0*/) {
	QString name = (QString)Att->getName();
	if( findChildObject( Uml::ot_Attribute, name).count() == 0 ) {
		Att -> parent() -> removeChild( Att );
		this -> insertChild( Att );
		m_AttsList.append( Att );
		emit modified();
		return true;
	} else if( Log ) {
		Log->removeChangeByNewID( Att -> getID() );
		delete Att;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLConcept::removeAttribute(UMLObject* a) {
	if(!m_AttsList.remove((UMLAttribute *)a)) {
		kdDebug() << "can't find att given in list" << endl;
		return -1;
	}
	emit modified();
	return m_AttsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLConcept::addTemplate(QString name, int id) {
  	UMLTemplate* newTemplate = new UMLTemplate(this, name, id);
  	m_TemplateList.append(newTemplate);
	return newTemplate;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::addTemplate(UMLTemplate* newTemplate, IDChangeLog* log /* = 0*/) {
	QString name = newTemplate->getName();
	if (findChildObject(Uml::ot_Template, name).count() == 0) {
		newTemplate->parent()->removeChild(newTemplate);
		this->insertChild(newTemplate);
		m_TemplateList.append(newTemplate);
		return true;
	} else if (log) {
		log->removeChangeByNewID( newTemplate->getID() );
		delete newTemplate;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLConcept::removeTemplate(UMLTemplate* newTemplate) {
	if ( !m_TemplateList.remove(newTemplate) ) {
		kdWarning() << "can't find att given in list" << endl;
		return -1;
	}
	return m_TemplateList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLConcept::addOperation(QString name, int id) {
	UMLOperation *o = new UMLOperation(this, name, id);
	m_OpsList.append(o);
	emit modified();
	return o;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::addOperation(UMLOperation* Op) {
	m_OpsList.append( Op );
	emit modified();
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::addOperation(UMLOperation* Op, IDChangeLog* Log) {
	QString name = (QString)Op -> getName();
	if( findChildObject( Uml::ot_Operation, name).count() == 0 ) {
		Op -> parent() -> removeChild( Op );
		this -> insertChild( Op );
		m_OpsList.append( Op );
		emit modified();
		return true;
	} else if( Log ) {
		Log->removeChangeByNewID( Op -> getID() );
		delete Op;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLConcept::removeOperation(UMLObject *o) {
	if(!m_OpsList.remove((UMLOperation *)o)) {
		kdDebug() << "can't find opp given in list" << endl;
		return -1;
	}
	emit modified();
	return m_OpsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLConcept::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		return UMLCanvasObject::uniqChildName(type);
	} else if (type == ot_Attribute) {
		currentName = i18n("new_attribute");
	} else if (type == ot_Template) {
		currentName = i18n("new_template");
	} else if (type == ot_Operation) {
		currentName = i18n("new_operation");
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
QPtrList<UMLObject> UMLConcept::findChildObject(UMLObject_Type t , QString n) {
  	QPtrList<UMLObject> list;
 	if (t == ot_Association) {
		return UMLCanvasObject::findChildObject(t, n);
 	} else if (t == ot_Attribute) {
  		UMLAttribute * obj=0;
  		for(obj=m_AttsList.first();obj != 0;obj=m_AttsList.next()) {
  			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
 	} else if (t == ot_Template) {
  		UMLTemplate* obj=0;
  		for(obj=m_TemplateList.first(); obj != 0; obj=m_TemplateList.next()) {
  			if (obj->getBaseType() == t && obj->getName() == n)
				list.append(obj);
		}
	} else if (t == ot_Operation) {
		UMLOperation * obj=0;
		for(obj=m_OpsList.first();obj != 0;obj=m_OpsList.next()) {
			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
	} else {
		kdWarning() << "finding child object of unknown type" << endl;
	}

	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLConcept::findChildObject(int id) {
        UMLOperation * o=0;
	for(o=m_OpsList.first();o != 0;o=m_OpsList.next()) {
		if(o->getID() == id)
			return o;
	}
	UMLAttribute * a = 0;
	for(a=m_AttsList.first();a != 0;a=m_AttsList.next()) {
		if(a->getID() == id)
			return a;
	}
	UMLTemplate* t = 0;
	for (t=m_TemplateList.first();t != 0;t=m_TemplateList.next()) {
		if (t->getID() == id) {
			return t;
		}
	}
	return UMLCanvasObject::findChildObject(id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// the sad thing here is that we have to pass along a UMLDocument pointer. 
// It would be better if each concept knew what document it belonged to.
// This should be changed in the future. 
QPtrList<UMLConcept> UMLConcept::findSubClassConcepts ( UMLDoc *doc) {
        QPtrList<UMLAssociation> list = this->getGeneralizations();
        QPtrList<UMLConcept> inheritingConcepts;
        int myID = this->getID();
        for (UMLAssociation *a = list.first(); a; a = list.next())
        {
                // Concepts on the "A" side inherit FROM this class
                // as long as the ID of the role A class isnt US (in 
                // that case, the generalization describes how we inherit
                // from another class). 
                // SO check for roleA id, it DOESNT match this concepts ID, 
                // then its a concept which inherits from us
                if (a->getRoleAId() != myID)
                {
                        UMLObject* obj = doc->findUMLObject(a->getRoleAId());
                        UMLConcept *concept = dynamic_cast<UMLConcept*>(obj);
                        if (concept)
                                inheritingConcepts.append(concept);
                }

        }
        return inheritingConcepts;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Same note as for the above findSubClassConcepts method. Need to have
// each Concept already know its UMLdocument.
QPtrList<UMLConcept> UMLConcept::findSuperClassConcepts ( UMLDoc *doc) {
        QPtrList<UMLAssociation> list = this->getGeneralizations();
        QPtrList<UMLConcept> parentConcepts;
        int myID = this->getID();
        for (UMLAssociation *a = list.first(); a; a = list.next())
        {
                // Concepts on the "B" side are parent (super) classes of this one
                // So check for roleB id, it DOESNT match this concepts ID, 
                // then its a concept which we inherit from
                if (a->getRoleBId() != myID)
                {
                        UMLObject* obj = doc->findUMLObject(a->getRoleBId());
                        UMLConcept *concept = dynamic_cast<UMLConcept*>(obj);
                        if (concept)
                                parentConcepts.append(concept);
                }
        }
        return parentConcepts;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if(!status)
		return status;
	// Note: The m_AssocsList is filled by UMLDoc::serialize().
	// UMLDoc::serialize() serializes the UMLConcepts before the
	// UMLAssociations.
	if(archive) {
		*s << m_OpsList.count()
		<< m_AttsList.count()
		<< m_TemplateList.count();
		//save operations
		UMLOperation * o=0;
		for(o=m_OpsList.first();o != 0;o=m_OpsList.next())
			o -> serialize(s, archive, fileversion);
		//save attributes
		UMLAttribute * a = 0;
		for(a=m_AttsList.first();a != 0;a=m_AttsList.next())
			a -> serialize(s, archive, fileversion);
		UMLTemplate* t = 0;
		for ( t=m_TemplateList.first(); t != 0; t=m_TemplateList.next() ) {
			t->serialize(s, archive, fileversion);
		}
	} else {
		int opCount, attCount, templateCount, type;
		//load concept instance and ops/atts
		*s >> opCount
		   >> attCount
		   >> templateCount;
		//load operations
		for(int i=0;i<opCount;i++) {
			*s >> type;
			UMLOperation *o = new UMLOperation(this);
			o -> serialize(s, archive, fileversion);
			m_OpsList.append(o);
		}
		//load attributes
		for(int i=0;i<attCount;i++) {
			*s >> type;
			UMLAttribute * a = new UMLAttribute(this);
			a -> serialize(s, archive, fileversion);
			m_AttsList.append(a);
		}
		//load templates
		for (int i=0;i<templateCount;i++) {
			*s >> type;
			UMLTemplate* newTemplate = new UMLTemplate(this);
			newTemplate->serialize(s, archive, fileversion);
			m_TemplateList.append(newTemplate);
		}
	}//end else
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLConcept::init() {
	m_BaseType = ot_Concept;
	m_OpsList.clear();
	m_OpsList.setAutoDelete(true);
	m_AttsList.clear();
	m_AttsList.setAutoDelete(true);
	m_TemplateList.clear();
	m_TemplateList.setAutoDelete(true);
}

long UMLConcept::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();
	//  Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string
	l_size += sizeof(m_OpsList.count());
	l_size += sizeof(m_AttsList.count());
	l_size += sizeof(m_TemplateList.count());

	UMLOperation * o=0;
	for(o=m_OpsList.first();o != 0;o=m_OpsList.next()) {
		l_size += o->getClipSizeOf();
	}
	UMLAttribute * a = 0;
	for(a=m_AttsList.first();a != 0;a=m_AttsList.next()) {
		l_size += a->getClipSizeOf();
	}
	UMLTemplate* t = 0;
	for ( t=m_TemplateList.first(); t != 0; t=m_TemplateList.next() ) {
		l_size += t->getClipSizeOf();
	}

	return l_size;
}

bool UMLConcept::operator==( UMLConcept & rhs ) {
	if ( m_AttsList.count() != rhs.m_AttsList.count() ) {
		return false;
	}
	if ( m_TemplateList.count() != rhs.m_TemplateList.count() ) {
		return false;
	}
	if ( m_OpsList.count() != rhs.m_OpsList.count() ) {
		return false;
	}
	if ( &m_AttsList != &(rhs.m_AttsList) ) {
		return false;
	}
	if ( &m_TemplateList != &(rhs.m_TemplateList) ) {
		return false;
	}
	if ( &m_OpsList != &(rhs.m_OpsList) ) {
		return false;
	}
	return UMLCanvasObject::operator==(rhs);
}

bool UMLConcept::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement classElement = qDoc.createElement( "UML:Class" );
	bool status = UMLObject::saveToXMI( qDoc, classElement );
	//save operations
	UMLOperation * pOp = 0;
	for( pOp = m_OpsList.first(); pOp != 0; pOp = m_OpsList.next() )
		pOp -> saveToXMI( qDoc, classElement );
	//save attributes
	UMLAttribute * pAtt = 0;
	for( pAtt = m_AttsList.first(); pAtt != 0; pAtt = m_AttsList.next() )
		pAtt -> saveToXMI( qDoc, classElement );
	//save templates
	UMLTemplate* newTemplate = 0;
	for (newTemplate = m_TemplateList.first(); newTemplate != 0; newTemplate = m_TemplateList.next() ) {
		newTemplate->saveToXMI(qDoc, classElement);
	}

	qElement.appendChild( classElement );
	return status;
}

bool UMLConcept::loadFromXMI( QDomElement & element ) {
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
		} else if (tag == "template") {
			UMLTemplate* newTemplate = new UMLTemplate(this);
			if ( !newTemplate->loadFromXMI(tempElement) ) {
				return false;
			}
			m_TemplateList.append(newTemplate);
		}
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while
	return true;
}

bool UMLConcept::acceptAssociationType(Uml::Association_Type type)
{
	switch(type)
	{
	case at_Generalization:
	case at_Aggregation:
	case at_Dependency:
	case at_Association:
//	    at_Association_Self,
	case at_Implementation:
	case at_Composition:
	case at_Realization:
	case at_UniAssociation:
	 	return true;
	default: return false;
	}
return false; //shutup compiler warning

}

int UMLConcept::templates() {
	return m_TemplateList.count();
}

int UMLConcept::operations() {
	return m_OpsList.count();
}

int UMLConcept::attributes() {
	return m_AttsList.count();
}

QPtrList<UMLAttribute>* UMLConcept::getAttList() {
	return &m_AttsList;
}

QPtrList<UMLTemplate>* UMLConcept::getTemplateList() {
	return &m_TemplateList;
}

QPtrList<UMLOperation>* UMLConcept::getOpList() {
	return &m_OpsList;
}
