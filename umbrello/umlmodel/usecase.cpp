/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
void UMLUseCase::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("UseCase"));
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:UseCase> element (TODO).
 */
bool UMLUseCase::load1(QDomElement&)
{
    return true;
}


