/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "concept.h"
#include "operation.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLConcept::UMLConcept(QObject * parent, QString Name, int id) : UMLObject(parent,Name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLConcept::UMLConcept(QObject * parent) : UMLObject(parent) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLConcept::~UMLConcept() {
	m_AttsList.clear();
	m_OpsList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLConcept::addAttribute(QString name, int id) {
	UMLAttribute *a = new UMLAttribute(this, name, id);
	m_AttsList.append(a);
	return a;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::addAttribute(UMLAttribute* Att, IDChangeLog* Log /* = 0*/) {
	QString name = (QString)Att->getName();
	if( findChildObject( Uml::ot_Attribute, name).count() == 0 ) {
		Att -> parent() -> removeChild( Att );
		this -> insertChild( Att );
		m_AttsList.append( Att );
		return true;
	} else if( Log ) {
		Log->removeChangeByNewID( Att -> getID() );
		delete Att;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLConcept::removeAttribute(UMLObject *a) {
	if(!m_AttsList.remove((UMLAttribute *)a)) {
		kdDebug() << "can't find att given in list" << endl;
		return -1;
	}
	return m_AttsList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLConcept::addOperation(QString name, int id) {
	UMLOperation *o = new UMLOperation(this, name, id);
	m_OpsList.append(o);
	return o;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::addOperation(UMLOperation* Op, IDChangeLog* Log /* = 0*/) {
	QString name = (QString)Op -> getName();
	if( findChildObject( Uml::ot_Operation, name).count() == 0 ) {
		Op -> parent() -> removeChild( Op );
		this -> insertChild( Op );
		m_OpsList.append( Op );
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
	return m_OpsList.count();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLConcept::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if(type == ot_Attribute)
		currentName = i18n("new_attribute");
	else
		currentName = i18n("new_operation");
	QString name = currentName;
	for (int number = 0; findChildObject(type, name).count(); ++number,
	        name = currentName + "_" + QString::number(number))
		;
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLObject> UMLConcept::findChildObject(UMLObject_Type t , QString n) {
	QPtrList<UMLObject> list;
	if(t == ot_Attribute) {
		UMLAttribute * obj=0;
		for(obj=m_AttsList.first();obj != 0;obj=m_AttsList.next()) {
			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
	} else {
		UMLOperation * obj=0;
		for(obj=m_OpsList.first();obj != 0;obj=m_OpsList.next()) {
			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
	}
	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject * UMLConcept::findChildObject(int id) {
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
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLConcept::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if(!status)
		return status;
	if(archive) {
		*s << m_OpsList.count()
		<< m_AttsList.count();
		//save operations
		UMLOperation * o=0;
		for(o=m_OpsList.first();o != 0;o=m_OpsList.next())
			o -> serialize(s, archive, fileversion);
		//save attributes
		UMLAttribute * a = 0;
		for(a=m_AttsList.first();a != 0;a=m_AttsList.next())
			a -> serialize(s, archive, fileversion);
	} else {
		int opCount, attCount, type;
		//load concept instance and ops/atts
		*s >> opCount
		>> attCount;
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
long UMLConcept::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();
	//  Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string
	l_size += sizeof(m_OpsList.count());
	l_size += sizeof(m_AttsList.count());

	UMLOperation * o=0;
	for(o=m_OpsList.first();o != 0;o=m_OpsList.next()) {
		l_size += o->getClipSizeOf();
	}
	UMLAttribute * a = 0;
	for(a=m_AttsList.first();a != 0;a=m_AttsList.next()) {
		l_size += a->getClipSizeOf();
	}

	return l_size;
}

bool UMLConcept::operator==( UMLConcept & rhs ) {
	if( this == &rhs )
		return true;

	if( ! UMLObject::operator==( rhs) )
		return false;

	if( m_AttsList.count() != rhs.m_AttsList.count() )
		return false;


	if( m_OpsList.count() != rhs.m_OpsList.count() )
		return false;

	if( &m_AttsList != &(rhs.m_AttsList) )
		return false;

	if( &m_OpsList != &(rhs.m_OpsList) )
		return false;
	return true;
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

	qElement.appendChild( classElement );
	return status;
}

bool UMLConcept::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI( element ) )
		return false;

	UMLOperation * pOp = 0;
	UMLAttribute * pAtt = 0;
	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();
	while( !tempElement.isNull() ) {
		QString tag = tempElement.tagName();
		if( tag == "UML:Operation" ) {
			pOp = new UMLOperation( this );
			if( !pOp -> loadFromXMI( tempElement ) )
				return false;
			m_OpsList.append( pOp );
		} else if( tag == "UML:Attribute" ) {
			pAtt = new UMLAttribute( this );
			if( !pAtt -> loadFromXMI( tempElement ) )
				return false;

			m_AttsList.append( pAtt );
		}
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while

	return true;
}




