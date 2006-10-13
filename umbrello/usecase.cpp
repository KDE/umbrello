/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "usecase.h"

UMLUseCase::UMLUseCase(const QString & name, Uml::IDType id)
        : UMLCanvasObject(name, id) {
    init();
}

UMLUseCase::~UMLUseCase() {}

void UMLUseCase::init() {
    m_BaseType = Uml::ot_UseCase;
}

UMLObject* UMLUseCase::clone() const {
    UMLUseCase *clone = new UMLUseCase();
    UMLObject::copyInto(clone);
    return clone;
}

void UMLUseCase::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement usecaseElement = UMLObject::save("UML:UseCase", qDoc);
    qElement.appendChild(usecaseElement);
}

bool UMLUseCase::load(QDomElement& ) {
    return true;
}


