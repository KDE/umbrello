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
#include "umlassociationlist.h"
#include "operation.h"
#include "attribute.h"
#include "stereotype.h"
#include "clipboard/idchangelog.h"
#include "umldoc.h"
#include <kdebug.h>
#include <klocale.h>

UMLClassifier::UMLClassifier(UMLDoc * parent, const QString & name, int id)
   : UMLCanvasObject(parent,name, id)
{
	init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClassifier::~UMLClassifier() {
  	m_OpsList.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// UMLObject* UMLClassifier::addOperation(QString name, int id) {
// 	UMLOperation *o = new UMLOperation(this, name, id);
// 	m_OpsList.append(o);
// 	emit modified();
// 	emit operationAdded(o);
// 	connect(o,SIGNAL(modified()),this,SIGNAL(modified()));
// 	return o;
// }

bool UMLClassifier::checkOperationSignature( UMLOperation *op )
{
	if( op->getName().length() == 0)
		return false;
	UMLObjectList list = findChildObject( Uml::ot_Operation, op->getName() );
	if( list.count() == 0 )
		return true;

	// there is at least one operation with the same name... compare the parameter list
	list.setAutoDelete(false);
	list.removeRef( op ); // dont compare against itself
	
	QPtrList<UMLAttribute> *testParams;
	QPtrList<UMLAttribute> *opParams;
	for( UMLOperation *test = dynamic_cast<UMLOperation*>(list.first()); 
	     test != 0; 
	     test = dynamic_cast<UMLOperation*>(list.next()) )
	{// Should we test for defautl values? ( ambiguous signatures, or is that language/compiler dependent?
		testParams = test->getParmList( );
		opParams   = op->getParmList( );
		
		if( testParams->count() != opParams->count() )
			continue;
		int pCount = testParams->count();
		int i = 0;
		for( ; i < pCount; ++i )
		{
			if( testParams->at(i)->getTypeName() != opParams->at(i)->getTypeName() )
				break;
		}
		if( i == pCount )
		{//all parameters matched -> the signature is not unique
			return false;
		}
	}
	// we did not find an exact match, so the signature is unique ( acceptable )
	return true;
}

bool UMLClassifier::addOperation(UMLOperation* op, int position )
{
	if( m_OpsList.findRef( op ) == -1  &&
	    checkOperationSignature( op ) == true ) 
	{
		if( op -> parent() )
			op -> parent() -> removeChild( op );
		this -> insertChild( op );
		if( position >= 0 && position <= (int)m_OpsList.count() )
			m_OpsList.insert(position,op);
		else
			m_OpsList.append( op );
		emit childObjectAdded(op);
		emit operationAdded(op);
		emit modified();
		connect(op,SIGNAL(modified()),this,SIGNAL(modified()));
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClassifier::addOperation(UMLOperation* Op, IDChangeLog* Log) {
	if( addOperation( Op, -1 ) )
		return true;
	else if( Log ) {
		Log->removeChangeByNewID( Op -> getID() );
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClassifier::removeOperation(UMLOperation *op) {
	if(!m_OpsList.remove(op)) {
		kdDebug() << "can't find opp given in list" << endl;
		return -1;
	}
	// disconnection needed.
	// note that we dont delete the operation, just remove it from the Classifier
	disconnect(op,SIGNAL(modified()),this,SIGNAL(modified()));
	emit childObjectRemoved(op);
	emit operationRemoved(op);
	emit modified();
	return m_OpsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClassifier::addStereotype(UMLStereotype* newStereotype, UMLObject_Type list, IDChangeLog* log /* = 0*/) {
	QString name = newStereotype->getName();
	if (findChildObject(Uml::ot_Template, name).count() == 0) {
		if(newStereotype->parent())
			newStereotype->parent()->removeChild(newStereotype);
		this->insertChild(newStereotype);
		if (list == ot_Operation) {
			m_OpsList.append(newStereotype);
			emit modified();
			emit childObjectAdded(newStereotype);
			connect(newStereotype, SIGNAL(modified()), this, SIGNAL(modified()));
//			emit operationAdded(newStereotype);
#warning "FIXME change operationAdded listeners to childObject, or create stereotypeAdded signal"
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
UMLObjectList UMLClassifier::findChildObject(UMLObject_Type t , QString n) {
  	UMLObjectList list;
 	if (t == ot_Association) {
		return UMLCanvasObject::findChildObject(t, n);
	} else if (t == ot_Operation) {
		UMLClassifierListItem* obj=0;
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
        UMLClassifierListItem * o=0;
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
UMLClassifierList UMLClassifier::findSubClassConcepts ( UMLDoc *doc, ClassifierType type) {
        UMLAssociationList list = this->getGeneralizations();
        UMLAssociationList rlist = this->getRealizations();

        UMLClassifierList inheritingConcepts;
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
			//FIXME does the addition of Datatypes break these if statements?
			if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
					|| (concept->isInterface() && type == INTERFACE)))
                                inheritingConcepts.append(concept);
                }

        }

        for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
	{
                if (a->getRoleAId() != myID)
		{
                        UMLObject* obj = doc->findUMLObject(a->getRoleAId());
                        UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
			if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
					|| (concept->isInterface() && type == INTERFACE)))
                                inheritingConcepts.append(concept);
		}
	}

        return inheritingConcepts;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// Same note as for the above findSubClassConcepts method. Need to have
// each Concept already know its UMLdocument.
UMLClassifierList UMLClassifier::findSuperClassConcepts ( UMLDoc *doc, ClassifierType type ) {
        UMLAssociationList list = this->getGeneralizations();
        UMLAssociationList rlist = this->getRealizations();

        UMLClassifierList parentConcepts;
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

                        if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
					|| (concept->isInterface() && type == INTERFACE)))
                                parentConcepts.append(concept);
                }
        }

        for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
	{
                if (a->getRoleBId() != myID)
		{
                        UMLObject* obj = doc->findUMLObject(a->getRoleBId());
                        UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
                        if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
                                        || (concept->isInterface() && type == INTERFACE)))
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

bool UMLClassifier::hasAbstractOps () {
        QPtrList<UMLOperation> *opl = getFilteredOperationsList();
        for(UMLOperation *op = opl->first(); op ; op = opl->next())
                if(op->getAbstract())
                        return true;
        return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLClassifier::operations() {
	return m_OpsList.count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLClassifierListItem>* UMLClassifier::getOpList() {
	return &m_OpsList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLOperationList* UMLClassifier::getFilteredOperationsList()  {
	UMLOperationList* operationList = new UMLOperationList;
	for(UMLClassifierListItem* listItem = m_OpsList.first(); listItem;
	    listItem = m_OpsList.next())  {
		if (listItem->getBaseType() == ot_Operation) {
			operationList->append(static_cast<UMLOperation*>(listItem));
		}
	}
	return operationList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLClassifier::init() {
	m_BaseType = ot_UMLObject;
	m_OpsList.clear();
	m_OpsList.setAutoDelete(false);
}

#include "classifier.moc"
