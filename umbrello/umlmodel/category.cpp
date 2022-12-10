/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
void UMLCategory::saveToXMI(QXmlStreamWriter& writer)
{
     UMLObject::save1(writer, QLatin1String("Category"));
     writer.writeAttribute(QLatin1String("categoryType"), QString::number(m_CategoryType));
     UMLObject::save1end(writer);
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

