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

void LinkWidget::constrainTextPos(int & /*textX*/, int & /*textY*/,
				  int /*textWidth*/, int /*textHeight*/,
				  Uml::Text_Role /*tr*/) {
	// Only applicable to MessageWidget.
}
 
void LinkWidget::calculateNameTextSegment() {
	// Only applicable to MessageWidget and AssociationWidget.
}


