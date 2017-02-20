/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "category.h"

/**
 * Constructs a Category.
 *
 * @param name   The name of the Category.
 * @param id     The unique id to assign to this Category.
 */
UMLCategory::UMLCategory(const QString & name, Uml::ID::Type id)
  : UMLCanvasObject(name, id)
{
    init();
}

/**
 * Standard destructor.
 */
UMLCategory::~UMLCategory()
{
}

/**
 * Initializes key variables of the class.
 */
void UMLCategory::init()
{
    m_BaseType = UMLObject::ot_Category;
    m_CategoryType = ct_Disjoint_Specialisation;
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLCategory::copyInto(UMLObject *lhs) const
{
    UMLCategory *target = lhs->asUMLCategory();

    // call the parent first
    UMLCanvasObject::copyInto(target);

    target->m_CategoryType = m_CategoryType;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLCategory::clone() const
{
    UMLCategory *clone = new UMLCategory();
    copyInto(clone);
    return clone;
}

/**
 * Creates the <UML:Category> XMI element.
 */
void UMLCategory::SaveToXMI1(QDomDocument& qDoc, QDomElement& qElement)
{
     QDomElement categoryElement = UMLObject::save1(QLatin1String("UML:Category"), qDoc);
     categoryElement.setAttribute(QLatin1String("categoryType"), (int)m_CategoryType);
     qElement.appendChild(categoryElement);
}

/**
 * Loads the <UML:Category> XMI element (empty.)
 */
bool UMLCategory::load1(QDomElement& element)
{
    m_CategoryType = (Category_Type)element.attribute(QLatin1String("categoryType"),
                                                      QLatin1String("0")).toInt();
    return true;
}

/**
 * Get the category type
 */
UMLCategory::Category_Type UMLCategory::getType()
{
    return m_CategoryType;
}

/**
 * Set the category type
 */
void UMLCategory::setType(Category_Type type)
{
    m_CategoryType = type;
    emitModified();
}

