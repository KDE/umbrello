/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "interface.h"
#include "association.h"
#include "operation.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLInterface::UMLInterface(QObject* parent, QString Name, int id) : UMLObject(parent,Name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLInterface::UMLInterface(QObject* parent) : UMLObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLInterface::~UMLInterface() {
 	m_AssocsList.clear();
 	m_TmpAssocs.clear();
  	m_OpsList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLAssociation> UMLInterface::getSpecificAssocs(Uml::Association_Type assocType) {
 	QPtrList<UMLAssociation> list;
 	for (UMLAssociation* a = m_AssocsList.first(); a; a = m_AssocsList.next())
 		if (a->getAssocType() == assocType)
 			list.append(a);
 	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::addAssociation(UMLAssociation* assoc) {
 	m_AssocsList.append( assoc );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::hasAssociation(UMLAssociation *a) {
 	if(m_AssocsList.containsRef(a) > 0)
 		return true;
 	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLInterface::removeAssociation(UMLObject *a) {
 	if(!m_AssocsList.remove((UMLAssociation *)a)) {
 		kdDebug() << "can't find assoc given in list" << endl;
 		return -1;
 	}
 	return m_AssocsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLInterface::addOperation(QString name, int id) {
	UMLOperation* operation = new UMLOperation(this, name, id);
	m_OpsList.append(operation);
	return operation;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::addOperation(UMLOperation* Op) {
	m_OpsList.append( Op );
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::addOperation(UMLOperation* Op, IDChangeLog* Log) {
	QString name = (QString)Op->getName();
	if( findChildObject( Uml::ot_Operation, name).count() == 0 ) {
		Op->parent()->removeChild( Op );
		this->insertChild( Op );
		m_OpsList.append( Op );
		return true;
	} else if( Log ) {
		Log->removeChangeByNewID( Op->getID() );
		delete Op;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLInterface::removeOperation(UMLObject *o) {
	if(!m_OpsList.remove((UMLOperation *)o)) {
		kdDebug() << "can't find opp given in list" << endl;
		return -1;
	}
	return m_OpsList.count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLInterface::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		currentName = i18n("new_association");
	} else if (type == ot_Operation) {
		currentName = i18n("new_operation");
	} else {
		kdWarning() << "creating child object which isn't association or op for interface" << endl;
	}

	QString name = currentName;
	for (int number = 0; findChildObject(type, name).count(); ++number,
	        name = currentName + "_" + QString::number(number))
		;
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLObject> UMLInterface::findChildObject(UMLObject_Type t , QString n) {
  	QPtrList<UMLObject> list;
 	if (t == ot_Association) {
 		UMLAssociation * obj=0;
 		for (obj = m_AssocsList.first(); obj != 0; obj = m_AssocsList.next()) {
 			if (obj->getBaseType() == t && obj->getName() == n)
 				list.append( obj );
 		}
	} else if (t == ot_Operation) {
		UMLOperation * obj=0;
		for(obj=m_OpsList.first();obj != 0;obj=m_OpsList.next()) {
			if(obj->getBaseType() == t && obj->getName() == n)
				list.append( obj );
		}
	} else {
		kdWarning() << "finding child object which isn't association or op for interface" << endl;
	}
	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLInterface::findChildObject(int id) {
	UMLOperation * o=0;
	for(o=m_OpsList.first();o != 0;o=m_OpsList.next()) {
		if(o->getID() == id)
			return o;
	}
	UMLAssociation * asso = 0;
	for (asso = m_AssocsList.first(); asso != 0; asso = m_AssocsList.next()) {
		if (asso->getID() == id)
			return asso;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if(!status)
		return status;
	// Note: The m_AssocsList is filled by UMLDoc::serialize().
	// UMLDoc::serialize() serializes the UMLInterfaces before the
	// UMLAssociations.
	if(archive) {
		*s << m_OpsList.count();
		//save operations
		UMLOperation * o=0;
		for(o=m_OpsList.first();o != 0;o=m_OpsList.next()) {
			o->serialize(s, archive, fileversion);
		}
	} else {
		int opCount, type;
		//load operations
		*s >> opCount;
		//load operations
		for(int i=0;i<opCount;i++) {
			*s >> type;
			UMLOperation *o = new UMLOperation(this);
			o->serialize(s, archive, fileversion);
			m_OpsList.append(o);
		}
	}//end else
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLInterface::init() {
	m_BaseType = ot_Interface;
	m_OpsList.clear();
	m_OpsList.setAutoDelete(true);
	m_AssocsList.clear();
	m_AssocsList.setAutoDelete(false);
	m_TmpAssocs.clear();
	m_TmpAssocs.setAutoDelete(false);
	setStereotype( i18n("interface") );
}

/** Returns the amount of bytes needed to serialize this object */
/* If the serialization method of this class is changed this function will have to be CHANGED TOO*/
/*This function is used by the Copy and Paste Functionality*/
/*The Size in bytes of a serialized QString Object is long sz:
		if ( (sz =str.length()*sizeof(QChar)) && !(const char*)str.unicode() )
		{
			sz = size of Q_UINT32; //  typedef unsigned int	Q_UINT32;		// 32 bit unsigned
		}
	This calculation is valid only for QT 2.1.x or superior, this is totally incompatible with QT 2.0.x or QT 1.x or inferior
	That means the copy and paste functionality will work on with QT 2.1.x or superior
*/
long UMLInterface::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();
	//  Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string
	l_size += sizeof(m_OpsList.count());

	UMLOperation * o=0;
	for(o=m_OpsList.first();o != 0;o=m_OpsList.next()) {
		l_size += o->getClipSizeOf();
	}

	return l_size;
}

bool UMLInterface::operator==( UMLInterface & rhs ) {
	if (this == &rhs) {
		return true;
	}
	if ( !UMLObject::operator==(rhs) ) {
		return false;
	}
	if ( m_OpsList.count() != rhs.m_OpsList.count() ) {
		return false;
	}
	if ( m_AssocsList.count() != rhs.m_AssocsList.count() ) {
		return false;
	}
	if ( &m_AssocsList != &(rhs.m_AssocsList) ) {
		return false;
	}
	return true;
}

bool UMLInterface::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement classElement = qDoc.createElement("UML:Interface");
	bool status = UMLObject::saveToXMI( qDoc, classElement );
	//save operations
	UMLOperation* pOp = 0;
	for ( pOp = m_OpsList.first(); pOp != 0; pOp = m_OpsList.next() ) {
		pOp->saveToXMI(qDoc, classElement);
	}
	qElement.appendChild( classElement );
	return status;
}

bool UMLInterface::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI(element) ) {
		return false;
	}

	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();
	while( !tempElement.isNull() ) {
		QString tag = tempElement.tagName();
		if (tag == "UML:Operation") {
			UMLOperation* pOp = new UMLOperation(this);
			if( !pOp->loadFromXMI(tempElement) ) {
				return false;
			}
			m_OpsList.append(pOp);
		}
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while
	return true;
}

int UMLInterface::operations() {
	return m_OpsList.count();
}

int UMLInterface::associations() {
	return m_AssocsList.count();
}

const QPtrList<UMLAssociation>& UMLInterface::getAssociations() {
	return m_AssocsList;
}

QPtrList<UMLAssociation> UMLInterface::getGeneralizations() {
	return getSpecificAssocs(Uml::at_Generalization);
}

QPtrList<UMLAssociation> UMLInterface::getAggregations() {
	return getSpecificAssocs(Uml::at_Aggregation);
}

QPtrList<UMLAssociation> UMLInterface::getCompositions() {
	return getSpecificAssocs(Uml::at_Composition);
}

QPtrList<UMLOperation>* UMLInterface::getOpList() {
	return &m_OpsList;
}
