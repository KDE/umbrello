/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "usecase.h"

/**
 * Creates a UseCase object
 *
 * @param name   The name of the object.
 * @param id     The id of the object.
 */
UMLUseCase::UMLUseCase(const QString & name, Uml::IDType id)
  : UMLCanvasObject(name, id)
{
    init();
}

/**
 * Standard decstructor.
 */
UMLUseCase::~UMLUseCase()
{
}

/**
 * Initializes key variables of the class.
 */
void UMLUseCase::init()
{
    m_BaseType = UMLObject::ot_UseCase;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLUseCase::clone() const
{
    UMLUseCase *clone = new UMLUseCase();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the <UML:UseCase> element.
 */
void UMLUseCase::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement usecaseElement = UMLObject::save("UML:UseCase", qDoc);
    qElement.appendChild(usecaseElement);
}

/**
 * Loads the <UML:UseCase> element (TODO).
 */
bool UMLUseCase::load(QDomElement& )
{
    return true;
}


