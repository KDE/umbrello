/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "uml.h"
#include "umldoc.h" 
#include "interface.h"
#include "operation.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLInterface::UMLInterface(UMLDoc * parent, const QString & name, int id)
   : UMLClassifier(parent, name, id)
{
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLInterface::~UMLInterface() {
  	m_OpsList.clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool UMLInterface::operator==( UMLInterface & rhs ) {
	if ( m_OpsList.count() != rhs.m_OpsList.count() ) {
		return false;
	}
	return UMLClassifier::operator==(rhs);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
QString UMLInterface::uniqChildName(UMLObject_Type type) {
	QString currentName;
	if (type == ot_Association) {
		return UMLCanvasObject::uniqChildName(type);
	} else if (type == ot_Operation) {
		currentName = i18n("new_operation");
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
bool UMLInterface::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
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
bool UMLInterface::loadFromXMI( QDomElement & element ) {
	if( !UMLObject::loadFromXMI(element) ) {
		return false;
	}

	QDomNode node = element.firstChild();
	QDomElement tempElement = node.toElement();
	while( !tempElement.isNull() ) {
		QString tag = tempElement.tagName();
		if (tag == "UML:Operation") {
			UMLOperation* op = UMLApp::app()->getDocument()->createOperation( );
			if( !op->loadFromXMI(tempElement) ||
			    !this->addOperation(op) ) {
				delete op;
				return false;
			}
		} else {
			kdWarning() << "loading unknown child type in UMLInterface::loadFromXMI" << endl;
		}
		node = node.nextSibling();
		tempElement = node.toElement();
	}//end while
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLInterface::init() {

	m_BaseType = ot_Interface;
	setStereotype( i18n("interface") );
}

#include "interface.moc"
