/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "usecase.h"
#include "umldoc.h"

UMLUseCase::UMLUseCase(UMLDoc * parent, const QString & name, int id) : UMLCanvasObject(parent, name, id) {
	init();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
UMLUseCase::~UMLUseCase() {}
////////////////////////////////////////////////////////////////////////////////////////////////////
void UMLUseCase::init() {
	m_BaseType = ot_UseCase;
}

bool UMLUseCase::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
	QDomElement usecaseElement = qDoc.createElement("UML:UseCase");
	bool status = UMLObject::saveToXMI(qDoc, usecaseElement);
	qElement.appendChild(usecaseElement);
	return status;
}

bool UMLUseCase::loadFromXMI(QDomElement& element) {
	return UMLObject::loadFromXMI(element);
}







