/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// own header
#include "classifier.h"
// qt/kde includes
#include <qregexp.h>
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "association.h"
#include "umlassociationlist.h"
#include "operation.h"
#include "attribute.h"
#include "stereotype.h"
#include "clipboard/idchangelog.h"
#include "umldoc.h"
#include "uml.h"
#include "model_utils.h"

using namespace Uml;

UMLClassifier::UMLClassifier(const QString & name, Uml::IDType id)
  : UMLPackage(name, id)
{
	init();
}


UMLClassifier::~UMLClassifier() {
}

UMLOperation * UMLClassifier::checkOperationSignature( QString name,
						       UMLAttributeList *opParams,
						       UMLOperation *exemptOp)
{
	UMLObjectList list = findChildObject( Uml::ot_Operation, name );
	if( list.count() == 0 )
		return NULL;
	int inputParmCount = (opParams ? opParams->count() : 0);

	// there is at least one operation with the same name... compare the parameter list
	for (UMLObjectListIt oit(list); oit.current(); ++oit)
	{
		UMLOperation* test = dynamic_cast<UMLOperation*>( oit.current() );
		if (test == NULL || test == exemptOp)
			continue;
		UMLAttributeList *testParams = test->getParmList( );
		if (!opParams) {
			if (0 == testParams->count())
				return test;
			continue;
		}
		int pCount = testParams->count();
		if( pCount != inputParmCount )
			continue;
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
	if (m_List.findRef(op) != -1) {
		kdDebug() << "UMLClassifier::addOperation: findRef("
			  << op->getName() << ") finds op (bad)"
			  << endl;
		return false;
	}
	if (checkOperationSignature(op->getName(), op->getParmList()) ) {
		kdDebug() << "UMLClassifier::addOperation: checkOperationSignature("
			  << op->getName() << ") op is non-unique" << endl;
		return false;
	}

	if( position >= 0 && position <= (int)m_List.count() )
		m_List.insert(position,op);
	else
		m_List.append( op );
	emit childObjectAdded(op);
	emit operationAdded(op);
	emit modified();
	connect(op,SIGNAL(modified()),this,SIGNAL(modified()));
	return true;
}

bool UMLClassifier::addOperation(UMLOperation* Op, IDChangeLog* Log) {
	if( addOperation( Op, -1 ) )
		return true;
	else if( Log ) {
		Log->removeChangeByNewID( Op -> getID() );
	}
	return false;
}

int UMLClassifier::removeOperation(UMLOperation *op) {
	if (op == NULL) {
		kdDebug() << "UMLClassifier::removeOperation called on NULL op"
			 << endl;
		return -1;
	}
	if(!m_List.remove(op)) {
		kdDebug() << "UMLClassifier::removeOperation: can't find op "
			  << op->getName() << " in list" << endl;
		return -1;
	}
	// disconnection needed.
	// note that we dont delete the operation, just remove it from the Classifier
	disconnect(op,SIGNAL(modified()),this,SIGNAL(modified()));
	emit childObjectRemoved(op);
	emit operationRemoved(op);
	emit modified();
	return m_List.count();
}

UMLOperation* UMLClassifier::takeOperation(UMLOperation* o) {
	if (removeOperation(o) >= 0) {
		return o;
	}
	return 0;
}

UMLClassifier::OpParseStatus UMLClassifier::parseOperation(QString m, OpDescriptor& desc) {
	UMLDoc *pDoc = UMLApp::app()->getDocument();

	m = m.stripWhiteSpace();
	if (m.isEmpty())
		return Op_Empty;
	QRegExp pat( "^(\\w+)" );
	int pos = pat.search(m);
	if (pos == -1)
		return Op_Illegal_MethodName;
	desc.m_name = pat.cap(1);
	desc.m_pReturnType = NULL;
	pat = QRegExp( ":\\s*(\\w+)$" );
	pos = pat.search(m);
	if (pos != -1) {  // return type is optional
		QString retType = pat.cap(1);
		UMLObject *pRetType = pDoc->findUMLObject(retType, Uml::ot_UMLObject, m_pUMLPackage);
		if (pRetType == NULL)
			return Op_Unknown_ReturnType;
		desc.m_pReturnType = dynamic_cast<UMLClassifier*>(pRetType);
	}
	desc.m_args.clear();
	pat = QRegExp( "\\((.*)\\)" );
	pos = pat.search(m);
	if (pos == -1)  // argument list is optional
		return Op_OK;
	QString arglist = pat.cap(1);
	arglist = arglist.stripWhiteSpace();
	if (arglist.isEmpty())
		return Op_OK;
	QStringList args = QStringList::split( QRegExp("\\s*,\\s*"), arglist);
	for (QStringList::Iterator lit = args.begin(); lit != args.end(); ++lit) {
		QStringList nameAndType = QStringList::split( QRegExp("\\s*:\\s*"), *lit);
		if (nameAndType.count() != 2)
			return Op_Malformed_Arg;
		UMLObject *pType = pDoc->findUMLObject(nameAndType[1], Uml::ot_UMLObject, m_pUMLPackage);
		if (pType == NULL)
			return Op_Unknown_ArgType;
		OpDescriptor::NameAndType nmTpPair(nameAndType[0], dynamic_cast<UMLClassifier*>(pType));
		desc.m_args.append(nmTpPair);
	}
	return Op_OK;
}

QString UMLClassifier::opParseStatusText(OpParseStatus value) {
	const QString text[] = { 
		i18n("OK"), i18n("Empty"), i18n("Illegal method name"), i18n("Malformed argument"),
		i18n("Unknown argument type"), i18n("Unknown return type"), i18n("Unspecified error")
	};
	return text[(unsigned) value];
}

UMLObjectList UMLClassifier::findChildObject(Object_Type t , const QString &n) {
	if (t == ot_Association) {
		return UMLCanvasObject::findChildObject(t, n);
	}
	UMLObjectList list;
	for (UMLClassifierListItemListIt lit(m_List); lit.current(); ++lit) {
		UMLClassifierListItem* obj = lit.current();
		if (obj->getBaseType() != t)
			continue;
		if (obj->getName() == n)
			list.append( obj );
	}
	return list;
}

UMLObject* UMLClassifier::findChildObject(Uml::IDType id) {
	for (UMLClassifierListItemListIt lit(m_List); lit.current(); ++lit) {
		UMLClassifierListItem* o = lit.current();
		if (o->getID() == id)
			return o;
	}
	return UMLCanvasObject::findChildObject(id);
}

UMLClassifierList UMLClassifier::findSubClassConcepts (ClassifierType type) {
	UMLClassifierList list = getSubClasses();
	UMLAssociationList rlist = getRealizations();

	UMLClassifierList inheritingConcepts;
	Uml::IDType myID = getID();
	for (UMLClassifier *c = list.first(); c; c = list.next())
	{
		if (type == ALL || (!c->isInterface() && type == CLASS)
				|| (c->isInterface() && type == INTERFACE))
			inheritingConcepts.append(c);
	}

	for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
	{
		if (a->getRoleId(A) != myID)
		{
			UMLObject* obj = a->getObject(A);
			UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
			if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
					|| (concept->isInterface() && type == INTERFACE)))
				inheritingConcepts.append(concept);
		}
	}

	return inheritingConcepts;
}

UMLClassifierList UMLClassifier::findSuperClassConcepts (ClassifierType type) {
	UMLClassifierList list = getSuperClasses();
	UMLAssociationList rlist = getRealizations();

	UMLClassifierList parentConcepts;
	Uml::IDType myID = getID();
	for (UMLClassifier *concept = list.first(); concept; concept = list.next())
	{
		if (type == ALL || (!concept->isInterface() && type == CLASS)
				|| (concept->isInterface() && type == INTERFACE))
			parentConcepts.append(concept);
	}

	for (UMLAssociation *a = rlist.first(); a; a = rlist.next())
	{
		if (a->getRoleId(B) != myID)
		{
			UMLObject* obj = a->getObject(B);
			UMLClassifier *concept = dynamic_cast<UMLClassifier*>(obj);
			if (concept && (type == ALL || (!concept->isInterface() && type == CLASS)
					|| (concept->isInterface() && type == INTERFACE)))
				parentConcepts.append(concept);
		}
	}

	return parentConcepts;
}

bool UMLClassifier::operator==( UMLClassifier & rhs ) {
	if ( m_List.count() != rhs.m_List.count() ) {
		return false;
	}
	if ( &m_List != &(rhs.m_List) ) {
		return false;
	}
	return UMLCanvasObject::operator==(rhs);
}


void UMLClassifier::copyInto(UMLClassifier *rhs) const
{
	UMLCanvasObject::copyInto(rhs);

	m_List.copyInto(&(rhs->m_List));
}


bool UMLClassifier::resolveRef() {
	bool success = UMLPackage::resolveRef();
	/**** Mysterious. The following loop does not work:
	for (UMLClassifierListItem *obj = m_List.first(); obj; obj = m_List.next())
	 {  ....  }
	 It will only iterate exactly once, and then the obj pointer becomes NULL.
	 Here's the replacement: ****/
	for (UMLClassifierListItemListIt oit(m_List); oit.current(); ++oit) {
		UMLClassifierListItem* obj = oit.current();
	/**** End of replacement ****/
		if (! obj->resolveRef())
			success = false;
		else
			emit childObjectAdded(obj);
	}
	return success;
}

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
		case at_Containment:
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
	UMLOperationList opl( getOpList() );
	for(UMLOperation *op = opl.first(); op ; op = opl.next())
		if(op->getAbstract())
			return true;
	return false;
}

int UMLClassifier::operations() {
	return getOpList().count();
}

UMLOperationList UMLClassifier::getOpList(bool includeInherited) {
	UMLOperationList ops;
	for (UMLClassifierListItemListIt lit(m_List); lit.current(); ++lit) {
		UMLClassifierListItem *li = lit.current();
		if (li->getBaseType() == ot_Operation)
			ops.append(static_cast<UMLOperation*>(li));
	}
	if (includeInherited) {
		UMLClassifierList parents(findSuperClassConcepts());
		for (UMLClassifierListIt pit(parents); pit.current(); ++pit) {
			// get operations for each parent by recursive call
			UMLOperationList pops = pit.current()->getOpList(true);
			// add these operations to operation list, but only if unique.
			for (UMLOperation *po = pops.first(); po; po = pops.next()) {
				QString po_as_string(po->toString(Uml::st_SigNoScope));
				UMLOperation *o = NULL;
				for (o = ops.first(); o; o = ops.next())
					if (o->toString(Uml::st_SigNoScope) == po_as_string)
						break;
				if (!o)
					ops.append(po);
			}
		}
	}
	return ops;
}

UMLClassifierListItemList UMLClassifier::getFilteredList(Object_Type ot) {
	UMLClassifierListItemList resultList;
	for (UMLClassifierListItemListIt lit(m_List); lit.current(); ++lit) {
		UMLClassifierListItem *listItem = lit.current();
		if (listItem->getBaseType() == ot)
			resultList.append(listItem);
	}
	return resultList;
}

void UMLClassifier::init() {
	m_BaseType = ot_UMLObject;
	m_List.setAutoDelete(false);
}

bool UMLClassifier::load(QDomElement& element) {

	for (QDomNode node = element.firstChild(); !node.isNull();
	     node = node.nextSibling()) {
		if (node.isComment())
			continue;
		element = node.toElement();
		QString tag = element.tagName();
		if (tagEq(tag, "Classifier.feature") ||
		    tagEq(tag, "Namespace.ownedElement") ||
		    tagEq(tag, "Namespace.contents")) {
			//CHECK: Umbrello currently assumes that nested elements
			// are features/ownedElements anyway.
			// Therefore these tags are not further interpreted.
			if (! load(element))
				return false;
		} else if (tagEq(tag, "Operation")) {
			UMLOperation* op = new UMLOperation(this);
			if (!op->loadFromXMI(element)) {
				kdError() << "UMLClassifier::load: error from op->loadFromXMI()"
					  << endl;
				delete op;
				return false;
			}
			if (!addOperation(op) ) {
				kdError() << "UMLClassifier::load: error from addOperation(op)"
					  << endl;
				delete op;
				//return false;
				// Returning false here will spoil the entire
				// load. At this point the user has been warned
				// that something went wrong so let's still try
				// our best effort.
			}
		} else if (!Umbrello::isCommonXMIAttribute(tag) &&
			   !loadSpecialized(element)) {
			UMLDoc *umldoc = UMLApp::app()->getDocument();
			UMLObject *pObject = umldoc->makeNewUMLObject(tag);
			if( !pObject )
				continue;
			pObject->setUMLPackage(this);
			if (pObject->loadFromXMI(element)) {
				addObject(pObject);
				if (tagEq(tag, "Generalization"))
					umldoc->addAssocToConcepts((UMLAssociation *) pObject);
			} else {
				delete pObject;
			}
		}
	}
	return true;
}

bool UMLClassifier::loadSpecialized(QDomElement& ) {
	// The UMLClass will override this for reading UMLAttributes.
	return true;
}

#include "classifier.moc"
