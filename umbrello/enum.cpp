/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "enum.h"
#include "enumliteral.h"
#include "stereotype.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLEnum::UMLEnum(UMLDoc* parent, const QString& name, int id): UMLClassifier(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLEnum::~UMLEnum() {
  	m_EnumLiteralList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLEnum::operator==( UMLEnum & rhs ) {
	if ( m_OpsList.count() != rhs.m_OpsList.count() ) {
		return false;
	}
	return UMLClassifier::operator==(rhs);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLEnum::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		return UMLCanvasObject::uniqChildName(type);
	} else if (type == ot_EnumLiteral) {
		currentName = i18n("new_literal");
	} else if (type == ot_Stereotype) {
		currentName = i18n("new_stereotype");
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
bool UMLEnum::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement enumElement = qDoc.createElement("UML:Enum");
	bool status = UMLObject::saveToXMI(qDoc, enumElement);
	//save operations
	UMLClassifierListItem* pEnumLiteral = 0;
	for ( pEnumLiteral = m_EnumLiteralList.first(); pEnumLiteral != 0;
	      pEnumLiteral = m_EnumLiteralList.next() ) {
		pEnumLiteral->saveToXMI(qDoc, enumElement);
	}
	qElement.appendChild(enumElement);
	return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLEnum::loadFromXMI(QDomElement& element) {
	if( !UMLObject::loadFromXMI(element) ) {
		return false;
	}

	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();
	while( !tempElement.isNull() ) {
		QString tag = tempElement.tagName();
		if (tag == "UML:EnumLiteral") {
			UMLEnumLiteral* pEnumLiteral = new UMLEnumLiteral(this);
			if( !pEnumLiteral->loadFromXMI(tempElement) ) {
				return false;
			}
			m_EnumLiteralList.append(pEnumLiteral);
		} else if (tag == "stereotype") {
			UMLStereotype* newStereotype = new UMLStereotype(this);
			if ( !newStereotype->loadFromXMI(tempElement) ) {
				return false;
			}
			QString listTypeString = tempElement.attribute("listtype", "-1");
			UMLObject_Type listType = (UMLObject_Type)listTypeString.toInt();
			if (listType == ot_EnumLiteral) {
				m_EnumLiteralList.append(newStereotype);
			} else {
				kdWarning() << "unknown listtype with stereotype:" << listType << endl;
			}
		} else {
			kdWarning() << "loading unknown child type in UMLClass::loadFromXMI" << endl;
		}
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLEnum::init() {
	m_BaseType = ot_Enum;
	setStereotype( i18n("enum") );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLEnum::addEnumLiteral(QString name, int id) {
  	UMLEnumLiteral* literal = new UMLEnumLiteral(this, name, id);
  	m_EnumLiteralList.append(literal);
	emit modified();
	connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
	emit enumLiteralAdded(literal);
	return literal;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLEnum::addEnumLiteral(UMLEnumLiteral* literal, IDChangeLog* Log /* = 0*/) {
	QString name = (QString)literal->getName();
	if (findChildObject(Uml::ot_EnumLiteral, name).count() == 0) {
		literal->parent()->removeChild(literal);
		this->insertChild(literal);
		m_EnumLiteralList.append(literal);
		emit modified();
		connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
		emit enumLiteralAdded(literal);
		return true;
	} else if (Log) {
		Log->removeChangeByNewID( literal->getID() );
		delete literal;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLEnum::addEnumLiteral(UMLEnumLiteral* literal, int position) {
	QString name = (QString)literal->getName();
	if (findChildObject( Uml::ot_EnumLiteral, name).count() == 0) {
		literal->parent()->removeChild(literal);
		this->insertChild(literal);
		if ( position >= 0 && position <= (int)m_EnumLiteralList.count() )  {
			m_EnumLiteralList.insert(position,literal);
		} else {
			m_EnumLiteralList.append(literal);
		}
		emit modified();
		connect(literal,SIGNAL(modified()),this,SIGNAL(modified()));
		emit enumLiteralAdded(literal);
		return true;
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLEnum::removeEnumLiteral(UMLObject* literal) {
	if (!m_EnumLiteralList.remove((UMLEnumLiteral*)literal)) {
		kdDebug() << "can't find att given in list" << endl;
		return -1;
	}
	emit enumLiteralRemoved(literal);
	emit modified();
	// If we are deleteing the object, then we dont need to disconnect..this is done auto-magically
	// for us by QObject. -b.t.
	// disconnect(a,SIGNAL(modified()),this,SIGNAL(modified()));
	delete literal;
	return m_EnumLiteralList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLEnum::addStereotype(UMLStereotype* newStereotype, UMLObject_Type list, IDChangeLog* log /* = 0*/) {
	QString name = newStereotype->getName();
	if (findChildObject(Uml::ot_Template, name).count() == 0) {
		newStereotype->parent()->removeChild(newStereotype);
		this->insertChild(newStereotype);
		if (list == ot_EnumLiteral) {
			m_EnumLiteralList.append(newStereotype);
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
UMLObjectList UMLEnum::findChildObject(UMLObject_Type t, QString n) {
  	UMLObjectList list;
 	if (t == ot_Association) {
		return UMLClassifier::findChildObject(t, n);
 	} else if (t == ot_EnumLiteral || t == ot_Stereotype) {
  		UMLClassifierListItem * obj=0;
  		for(obj=m_EnumLiteralList.first();obj != 0;obj=m_EnumLiteralList.next()) {
  			if(obj->getBaseType() == t && obj -> getName() == n)
				list.append( obj );
		}
 	} else {
		kdWarning() << "finding child object of unknown type" <<t<<" (requested name = "<<n<<")"<<endl;
	}

	return list;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLObject* UMLEnum::findChildObject(int id) {
	UMLClassifierListItem * a = 0;
	for(a=m_EnumLiteralList.first();a != 0;a=m_EnumLiteralList.next()) {
		if(a->getID() == id)
			return a;
	}
	return UMLClassifier::findChildObject(id);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int UMLEnum::enumLiterals() {
	return m_EnumLiteralList.count();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
QPtrList<UMLClassifierListItem>* UMLEnum::getEnumLiteralList() {
	return &m_EnumLiteralList;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLEnumLiteralList UMLEnum::getFilteredEnumLiteralList() {
	UMLEnumLiteralList enumLiteralList;
	for(UMLClassifierListItem* listItem = m_EnumLiteralList.first(); listItem;
	    listItem = m_EnumLiteralList.next())  {
		if (listItem->getBaseType() == ot_EnumLiteral) {
			enumLiteralList.append(static_cast<UMLEnumLiteral*>(listItem));
		}
	}
	return enumLiteralList;
}

#include "enum.moc"
