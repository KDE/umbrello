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
#include "uml.h"
#include <kdebug.h>
#include <klocale.h>

UMLClassifier::UMLClassifier(const QString & name, int id)
  : UMLCanvasObject(name, id)
{
	init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClassifier::~UMLClassifier() {
}

UMLOperation * UMLClassifier::checkOperationSignature( QString name,
																UMLAttributeList *opParams,
																UMLOperation *exemptOp)
{
	UMLObjectList list = findChildObject( Uml::ot_Operation, name );
	if( list.count() == 0 )
		return NULL;

	// there is at least one operation with the same name... compare the parameter list
	for( UMLOperation *test = dynamic_cast<UMLOperation*>(list.first()); 
	     test != 0; 
	     test = dynamic_cast<UMLOperation*>(list.next()) )
	{
		if (test == exemptOp)
			continue;
		UMLAttributeList *testParams = test->getParmList( );
		if( testParams->count() != opParams->count() )
			continue;
		int pCount = testParams->count();
		int i = 0;
		for( ; i < pCount; ++i )
		{
			// The only criterion for equivalence is the parameter types.
			// (Default values are not considered.)
			if( testParams->at(i)->getTypeName() != opParams->at(i)->getTypeName() )
				break;
		}
		if( i == pCount )
		{//all parameters matched -> the signature is not unique
			return test;
		}
	}
	// we did not find an exact match, so the signature is unique ( acceptable )
	return NULL;
}

bool UMLClassifier::addOperation(UMLOperation* op, int position )
{
	if( m_OpsList.findRef( op ) != -1  ||
	    checkOperationSignature(op->getName(), op->getParmList()) ) 
		return false;

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
		// What is this?? do we really want to store stereotypes in opsList?!?? -b.t.
#warning "addStereotype method needs review..conflicts with set/getStereoType in umlobject aswell as opList storage issues"
		if (list == ot_Operation) {
			m_OpsList.append(newStereotype);
			emit modified();
			emit childObjectAdded(newStereotype);
			emit stereotypeAdded(newStereotype);
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
int UMLClassifier::removeStereotype(UMLStereotype * /* stype*/) {
#warning "removeStereotype method not implemented yet"
	kdError() << "can't find stereotype given in list" << endl;
	return -1;
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
UMLClassifierList UMLClassifier::findSubClassConcepts (ClassifierType type) {
	UMLClassifierList list = this->getSubClasses();
	UMLAssociationList rlist = this->getRealizations();

	UMLClassifierList inheritingConcepts;
	int myID = this->getID();
	for (UMLClassifier *c = list.first(); c; c = list.next())
	{
		if (type == ALL || (!c->isInterface() && type == CLASS)
				|| (c->isInterface() && type == INTERFACE))
			inheritingConcepts.append(c);
	}

	for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
	{
		if (a->getRoleAId() != myID)
		{
			UMLObject* obj = a->getObjectA();
			UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
			if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
					|| (concept->isInterface() && type == INTERFACE)))
				inheritingConcepts.append(concept);
		}
	}

	return inheritingConcepts;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLClassifierList UMLClassifier::findSuperClassConcepts (ClassifierType type) {
	UMLClassifierList list = this->getSuperClasses();
	UMLAssociationList rlist = this->getRealizations();

	UMLClassifierList parentConcepts;
	int myID = this->getID();
	for (UMLClassifier *concept = list.first(); concept; concept = list.next())
	{
		if (type == ALL || (!concept->isInterface() && type == CLASS)
				|| (concept->isInterface() && type == INTERFACE))
			parentConcepts.append(concept);
	}

	for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
	{
		if (a->getRoleBId() != myID)
		{
			UMLObject* obj = a->getObjectB();
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
		case at_Association_Self:
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
        UMLOperationList *opl = getFilteredOperationsList();
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
UMLClassifierListItemList* UMLClassifier::getOpList() {
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
	m_OpsList.setAutoDelete(false);

	// make connections so that parent document is updated of list of uml objects
#warning "Cheap add/removeOperation fix for slot add/RemoveUMLObject calls. Need long-term solution"
        UMLDoc * parent = UMLApp::app()->getDocument();
        connect(this,SIGNAL(childObjectAdded(UMLObject *)),parent,SLOT(addUMLObject(UMLObject*)));
        connect(this,SIGNAL(childObjectRemoved(UMLObject *)),parent,SLOT(slotRemoveUMLObject(UMLObject*)));

}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClassifier::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement classElement = qDoc.createElement("UML:Interface");
	bool status = UMLObject::saveToXMI( qDoc, classElement );
	//save operations
	UMLClassifierListItem* pOp = 0;
	for ( pOp = m_OpsList.first(); pOp != 0; pOp = m_OpsList.next() ) {
		pOp->saveToXMI(qDoc, classElement);
	}
	qElement.appendChild( classElement );
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLClassifier::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI(element) ) {
		return false;
	}
	return load(element);
}

bool UMLClassifier::load(QDomElement& element) {
	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();
	while( !tempElement.isNull() ) {
		QString tag = tempElement.tagName();
		if (tag == "UML:Classifier.feature") {
			//CHECK: Umbrello currently assumes that nested elements
			// are features anyway.
			// Therefore the <UML:Classifier.feature> tag is of no
			// significance.
			if (! load(tempElement))
				return false;
		} else if (tag == "UML:Operation") {
			UMLOperation* op = new UMLOperation(NULL);
			if( !op->loadFromXMI(tempElement) ||
			    !this->addOperation(op) ) {
				delete op;
				return false;
			}
		}
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while
	return true;
}


#include "classifier.moc"
