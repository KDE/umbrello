/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "classifier.h"
#include "association.h"
#include "operation.h"
#include "clipboard/idchangelog.h"
#include "umldoc.h"
#include <kdebug.h>
#include <klocale.h>

UMLClassifier::UMLClassifier(QObject * parent, QString Name, int id) : UMLCanvasObject(parent,Name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClassifier::UMLClassifier(QObject * parent) : UMLCanvasObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClassifier::~UMLClassifier() {
  	m_OpsList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLClassifier::addOperation(QString name, int id) {
	UMLOperation *o = new UMLOperation(this, name, id);
	m_OpsList.append(o);
	emit modified();
	emit operationAdded(o);
	connect(o,SIGNAL(modified()),this,SIGNAL(modified()));
	return o;
}

bool UMLClassifier::addOperation(UMLOperation* op, int position )
{
	QString name = (QString)op->getName();
	op -> parent() -> removeChild( op );
	this -> insertChild( op );
	if( position >= 0 && position <= (int)m_OpsList.count() )
		m_OpsList.insert(position,op);
	else
		m_OpsList.append( op );
	emit modified();
	connect(op,SIGNAL(modified()),this,SIGNAL(modified()));
	emit operationAdded(op);
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClassifier::addOperation(UMLOperation* Op, IDChangeLog* Log) {
	QString name = (QString)Op -> getName();
	if( findChildObject( Uml::ot_Operation, name).count() == 0 ) {
		Op -> parent() -> removeChild( Op );
		this -> insertChild( Op );
		m_OpsList.append( Op );
		emit operationAdded(Op);
		emit modified();
		connect(Op,SIGNAL(modified()),this,SIGNAL(modified()));
		return true;
	} else if( Log ) {
		Log->removeChangeByNewID( Op -> getID() );
		delete Op;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClassifier::removeOperation(UMLObject *o) {
	if(!m_OpsList.remove((UMLOperation *)o)) {
		kdDebug() << "can't find opp given in list" << endl;
		return -1;
	}
	emit operationRemoved(o);
	emit modified();
	disconnect(o,SIGNAL(modified()),this,SIGNAL(modified()));
	return m_OpsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLObject> UMLClassifier::findChildObject(UMLObject_Type t , QString n) {
  	QPtrList<UMLObject> list;
 	if (t == ot_Association) {
		return UMLCanvasObject::findChildObject(t, n);
	} else if (t == ot_Operation) {
		UMLOperation * obj=0;
		for(obj=m_OpsList.first();obj != 0;obj=m_OpsList.next()) {
			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
	} else {
		kdWarning() << "finding child object of unknown type: " << t << endl;
	}

	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLClassifier::findChildObject(int id) {
        UMLOperation * o=0;
	for(o=m_OpsList.first();o != 0;o=m_OpsList.next()) {
		if(o->getID() == id)
			return o;
	}
	return UMLCanvasObject::findChildObject(id);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// the sad thing here is that we have to pass along a UMLDocument pointer.
// It would be better if each concept knew what document it belonged to.
// This should be changed in the future.
QPtrList<UMLClassifier> UMLClassifier::findSubClassConcepts ( UMLDoc *doc) {
        QPtrList<UMLAssociation> list = this->getGeneralizations();
        QPtrList<UMLClassifier> inheritingConcepts;
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
                        UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
                        if (concept)
                                inheritingConcepts.append(concept);
                }

        }
        return inheritingConcepts;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Same note as for the above findSubClassConcepts method. Need to have
// each Concept already know its UMLdocument.
QPtrList<UMLClassifier> UMLClassifier::findSuperClassConcepts ( UMLDoc *doc) {
        QPtrList<UMLAssociation> list = this->getGeneralizations();
        QPtrList<UMLClassifier> parentConcepts;
        int myID = this->getID();
        for (UMLAssociation *a = list.first(); a; a = list.next())
        {
                // Concepts on the "B" side are parent (super) classes of this one
                // So check for roleB id, it DOESNT match this concepts ID,
                // then its a concept which we inherit from
                if (a->getRoleBId() != myID)
                {
                        UMLObject* obj = doc->findUMLObject(a->getRoleBId());
                        UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
                        if (concept)
                                parentConcepts.append(concept);
                }
        }
        return parentConcepts;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClassifier::operator==( UMLClassifier & rhs ) {
	if ( m_OpsList.count() != rhs.m_OpsList.count() ) {
		return false;
	}
	if ( &m_OpsList != &(rhs.m_OpsList) ) {
		return false;
	}
	return UMLCanvasObject::operator==(rhs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// this perhaps should be in UMLClass/UMLInterface classes instead.
bool UMLClassifier::acceptAssociationType(Uml::Association_Type type)
{
	switch(type)
	{
		case at_Generalization:
		case at_Aggregation:
		case at_Dependency:
		case at_Association:
		case at_Implementation:
		case at_Composition:
		case at_Realization:
		case at_UniAssociation:
	 		return true;
		default:
			return false;
	}
	return false; //shutup compiler warning
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClassifier::operations() {
	return m_OpsList.count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLOperation>* UMLClassifier::getOpList() {
	return &m_OpsList;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLClassifier::init() {
	m_BaseType = ot_UMLObject;
	m_OpsList.clear();
	m_OpsList.setAutoDelete(false);
}

#include "classifier.moc"
