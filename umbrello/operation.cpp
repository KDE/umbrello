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
#include "classifier.h"
#include "umldoc.h"
#include "dialogs/umloperationdialog.h"

UMLOperation::UMLOperation(UMLClassifier *parent, QString Name, int id, Scope s, QString rt) 
    : UMLClassifierListItem((UMLObject*)parent,Name, id) 
{
	m_ReturnType = rt;
	m_Scope = s;
	m_BaseType = ot_Operation;
	m_nUniqueID = 0;//used for parm ids - local only to this op
	m_List.clear();
	m_List.setAutoDelete(false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLOperation::UMLOperation(UMLClassifier * parent) 
    : UMLClassifierListItem ((UMLObject*) parent) 
{
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
	// make the new parameter (attribute) public, just to be safe
	UMLAttribute * a = new UMLAttribute(this, name, ++m_nUniqueID,type, Uml::Public);
	a -> setDoc(doc);
	a -> setInitialValue(initialValue);
	addParm(a);
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
void UMLOperation::addParm(UMLAttribute *parameter, int position) {
	if( position >= 0 && position <= (int)m_List.count() )
		m_List.insert(position,parameter);
	else
		m_List.append( parameter );
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

bool UMLOperation::isConstructorOperation ( ) {
        UMLClassifier * c = dynamic_cast<UMLClassifier*>(this->parent());

        QString cName = c->getName();
        QString opName = getName();
        QString opReturn = getReturnType();

        // its a constructor operation if the operation name and return type
        // match that of the parent classifier
        return (cName == opName && cName == opReturn) ? true : false;
}

bool UMLOperation::showPropertiesDialogue(QWidget* parent) {
	UMLOperationDialog dialogue(parent, this);
	return dialogue.exec();
}

#include "operation.moc"
