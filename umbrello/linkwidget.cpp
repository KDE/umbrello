/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "linkwidget.h"

#include <kdebug.h>
#include "umlview.h"
#include "umlobject.h"

LinkWidget::LinkWidget(UMLView *view)
  : QObject(view), m_Type(Uml::wt_UMLWidget), m_pView(view), m_pObject(NULL) {
}

void LinkWidget::setBaseType( Uml::Widget_Type type ) {
	m_Type = type;
}

Uml::Widget_Type LinkWidget::getBaseType() const {
	return m_Type;
}

UMLObject *LinkWidget::getUMLObject() {
	return m_pObject;
}

void LinkWidget::setUMLObject(UMLObject * o) {
	m_pObject = o;
}

QString LinkWidget::getDoc() const {
	if (m_pObject != NULL)
		return m_pObject->getDoc();
	return "";
}

void LinkWidget::setDoc( QString doc ) {
	if (m_pObject != NULL)
		m_pObject->setDoc( doc );
}

UMLClassifier *LinkWidget::getOperationOwner(FloatingText *) {
	// Only applicable to MessageWidget and AssociationWidget.
	return NULL;
}

void LinkWidget::setOperationText(FloatingText *, QString) {
	// Only applicable to MessageWidget and AssociationWidget.
}

void LinkWidget::resetTextPositions() {
	// Only applicable to MessageWidget and AssociationWidget.
}

void LinkWidget::setMessageText(FloatingText *) {
	// Only applicable to MessageWidget and AssociationWidget.
}

void LinkWidget::setText(FloatingText *, QString) {
	// Only applicable to MessageWidget and AssociationWidget.
}

bool LinkWidget::showDialog() {
	// Only applicable to MessageWidget and AssociationWidget.
	return true;
}

UMLClassifier *LinkWidget::getSeqNumAndOp(FloatingText *, QString&, QString&) {
	// Only applicable to MessageWidget and AssociationWidget.
	return NULL;
}

void LinkWidget::setSeqNumAndOp(QString /*seqNum*/, QString /*op*/) {
	// Only applicable to MessageWidget and AssociationWidget.
}

void LinkWidget::updateMessagePos(int /*textHeight*/, int& /*X*/, int& /*Y*/) {
	// Only applicable to MessageWidget and AssociationWidget.
}

void LinkWidget::constrainY(int & /*y*/, int /*height*/) {
	// Only applicable to MessageWidget.
}
 
void LinkWidget::calculateNameTextSegment() {
	// Only applicable to MessageWidget and AssociationWidget.
}


#include "linkwidget.moc"
