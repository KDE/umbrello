/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>

#include "operation.h"
#include "attribute.h"

UMLOperation::UMLOperation(QObject *parent, QString Name, int id, Scope s, QString rt) : UMLObject(parent,Name, id) {
	m_ReturnType = rt;
	m_Scope = s;
	m_BaseType = ot_Operation;
	m_nUniqueID = 0;//used for parm ids - local only to this op
	m_List.clear();
	m_List.setAutoDelete(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLOperation::UMLOperation(QObject * parent) : UMLObject(parent) {
	m_ReturnType = "";
	m_BaseType = ot_Operation;
	m_nUniqueID = 0;
	m_List.clear();
	m_List.setAutoDelete(true);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLOperation::~UMLOperation() {
	m_List.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLOperation::addParm(QString type, QString name, QString initialValue, QString doc) {
	UMLAttribute * a = new UMLAttribute(this, name, ++m_nUniqueID,type);
	a -> setDoc(doc);
	a -> setInitialValue(initialValue);
	m_List.append(a);
	emit modified();
	connect(a,SIGNAL(modified()),this,SIGNAL(modified()));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLOperation::removeParm(UMLAttribute * a) {
	if(!m_List.remove(a))
		kdDebug() << "Error removing parm" << endl;
	
	emit modified();
	disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLAttribute* UMLOperation::findParm(QString name) {
	UMLAttribute * obj=0;
	for(obj=m_List.first();obj != 0;obj=m_List.next()) {
		if(obj -> getName() == name)
			return obj;
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLOperation::toString(Signature_Type sig) {
	QString s = "";

	if(sig == st_ShowSig || sig == st_NoSig) {
		if(m_Scope == Public)
			s = "+ ";
		else if(m_Scope == Private)
			s = "- ";
		else if(m_Scope == Protected)
			s = "# ";
	}

	s += getName();
	s.append("(");

	if(sig == st_NoSig || sig == st_NoSigNoScope) {
		s.append(")");
		return s;
	}
	UMLAttribute * obj=0;
	int last = m_List.count(), i = 0;
	for(obj=m_List.first();obj != 0;obj=m_List.next()) {
		i++;
		s.append(obj -> toString(st_SigNoScope));
		if(i < last)
			s.append(", ");
	}
	s.append(")");
	if (m_ReturnType.length() > 0 ) {
		s.append(" : ");

		if (m_ReturnType.startsWith("virtual ")) {
			s += m_ReturnType.mid(8);
		} else {
			s += m_ReturnType;
		}
	}
	return s;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLOperation::serialize(QDataStream *s, bool archive, int fileversion) {
	bool status = UMLObject::serialize(s, archive, fileversion);
	if(!status)
		return status;
	if(archive) {
		*s << m_ReturnType
		<< m_nUniqueID
		<< m_List.count();

		UMLAttribute * obj;
		for(obj=m_List.first();obj != 0;obj=m_List.next())
			obj -> serialize(s, archive, fileversion);
	} else {
		int count, type;
		*s >> m_ReturnType
		>> m_nUniqueID
		>> count;
		for(int i=0;i<count;i++)//load each parm
		{
			*s >> type;
			UMLAttribute *a = new UMLAttribute(this);
			a -> serialize(s, archive, fileversion);
			m_List.append(a);
		}//end for i
	}//end else
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLOperation::addParm(UMLAttribute *parameter) {
	m_List.append(parameter);
	emit modified();
	connect(parameter,SIGNAL(modified()),this,SIGNAL(modified()));
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLOperation::getUniqueParameterName() {
	QString currentName = i18n("new_parameter");
	QString name = currentName;
	for (int number = 1; findParm(name); number++) {
	        name = currentName + "_" + QString::number(number);
	}
	return name;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

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
long UMLOperation::getClipSizeOf() {
	long l_size = UMLObject::getClipSizeOf();
	Q_UINT32 tmp; //tmp is used to calculate the size of each serialized null string

	if ( !m_ReturnType.length() ) //We assume we are working with QT 2.1.x or superior, that means
		//if unicode returns a null pointer then the serialization process of the QString object
		//will write a null marker 0xffffff, see QString::operator<< implementation
	{
		l_size += sizeof(tmp);
	} else {
		l_size += (m_ReturnType.length()*sizeof(QChar));



	}
	l_size += sizeof(m_nUniqueID);
	l_size += sizeof(m_List.count());

	UMLAttribute * obj;
	for(obj=m_List.first();obj != 0;obj=m_List.next()) {
		l_size += obj->getClipSizeOf();
	}

	return l_size;
}


bool UMLOperation::operator==( UMLOperation & rhs ) {
	if( this == &rhs )
		return true;

	if( !UMLObject::operator==( rhs ) )
		return false;

	if( m_nUniqueID != rhs.m_nUniqueID )
		return false;

	if( m_ReturnType != rhs.m_ReturnType )
		return false;

	if( m_List.count() != rhs.m_List.count() )
		return false;

	if(!(m_List == rhs.m_List))
		return false;

	return true;
}

bool UMLOperation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
	QDomElement operationElement = qDoc.createElement( "UML:Operation" );
	bool status = UMLObject::saveToXMI( qDoc, operationElement );
	operationElement.setAttribute( "type", m_ReturnType );
	//save each attribute here, type different
	UMLAttribute* pAtt = 0;
	for( pAtt = m_List.first(); pAtt != 0; pAtt = m_List.next() ) {
		QDomElement attElement = qDoc.createElement( "UML:Parameter" );
		pAtt -> UMLObject::saveToXMI( qDoc, attElement );
		attElement.setAttribute( "type", pAtt -> getTypeName() );
		attElement.setAttribute( "value", pAtt -> getInitialValue() );
		operationElement.appendChild( attElement );
	}
	qElement.appendChild( operationElement );
	return status;
}

bool UMLOperation::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI( element ) )
		return false;

	m_ReturnType = element.attribute( "type", "" );
	QDomNode node = element.firstChild();
	QDomElement attElement = node.toElement();
	while( !attElement.isNull() ) {
		//should be UML:Paramater tag name but check anyway
		if( attElement.tagName() == "UML:Parameter" ) {
			UMLAttribute * pAtt = new UMLAttribute( this );
			if( !pAtt -> UMLObject::loadFromXMI( attElement ) )
				return false;
			pAtt -> setTypeName( attElement.attribute( "type", "" ) );
			pAtt -> setInitialValue( attElement.attribute( "value", "" ) );
			if( m_nUniqueID < pAtt -> getID() )
				m_nUniqueID = pAtt -> getID();
			m_List.append( pAtt );
		}
		node = node.nextSibling();
		attElement = node.toElement();
	}//end while
	return true;
}






#include "operation.moc"
