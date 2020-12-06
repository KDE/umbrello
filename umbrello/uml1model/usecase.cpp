/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "usecase.h"

/**
 * Creates a UseCase object
 *
 * @param name   The name of the object.
 * @param id     The id of the object.
 */
UMLUseCase::UMLUseCase(const QString & name, Uml::ID::Type id)
  : UMLCanvasObject(name, id)
{
    init();
}

/**
 * Standard destructor.
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
void UMLUseCase::saveToXMI1(QXmlStreamWriter& writer)
{
    UMLObject::save1(QLatin1String("UML:UseCase"), writer);
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:UseCase> element (TODO).
 */
bool UMLUseCase::load1(QDomElement&)
{
    return true;
}


