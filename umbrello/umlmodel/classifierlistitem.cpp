/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

// own header
#include "classifierlistitem.h"

// local includes
#include "debug_utils.h"
#include "classifier.h"
#include "model_utils.h"
#include "object_factory.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

/**
 * Constructor.
 *
 * @param parent   The parent to this operation.
 *          At first sight it would appear that the type of the
 *          parent should be UMLClassifier. However, the class
 *          UMLAttribute is also used for the parameters of
 *          operations, and in this case the UMLOperation is the
 *          parent.
 * @param name    The name of the operation.
 * @param id      The id of the operation.
 */
UMLClassifierListItem::UMLClassifierListItem(UMLObject *parent,
                                             const QString& name, Uml::ID::Type id)
  : UMLObject(parent, name, id)
{
    UMLClassifier *pc = parent->asUMLClassifier();
    if (pc)
        UMLObject::setUMLPackage(pc);
}

/**
 * Constructor.
 *
 * @param parent    The parent to this operation.
 *          At first sight it would appear that the type of the
 *          parent should be UMLClassifier. However, the class
 *          UMLAttribute is also used for the parameters of
 *          operations, and in this case the UMLOperation is the
 *          parent.
 */
UMLClassifierListItem::UMLClassifierListItem(UMLObject *parent)
  : UMLObject(parent)
{
    UMLClassifier *pc = parent->asUMLClassifier();
    if (pc)
        UMLObject::setUMLPackage(pc);
}

/**
 * Destructor.  Empty.
 */
UMLClassifierListItem::~UMLClassifierListItem()
{
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLClassifierListItem::copyInto(UMLObject *lhs) const
{
    // Call the parent.
    UMLObject::copyInto(lhs);
    UMLClassifierListItem *o = lhs->asUMLClassifierListItem();
    if (o)
        o->setType(getType());
}

/**
 * Returns a string representation of the list item.
 *
 * @param sig   What type of operation string to show.
 * @return  The string representation of the operation.
 */
QString UMLClassifierListItem::toString(Uml::SignatureType::Enum sig)
{
    Q_UNUSED(sig);
    return name();
}

/**
 * Returns the type of the UMLClassifierListItem.
 *
 * @return  The type of the UMLClassifierListItem.
 */
UMLClassifier * UMLClassifierListItem::getType() const
{
    return m_pSecondary->asUMLClassifier();
}

/**
 * Returns the type name of the UMLClassifierListItem.
 *
 * @return  The type name of the UMLClassifierListItem.
 */
QString UMLClassifierListItem::getTypeName() const
{
    if (m_pSecondary == 0)
        return m_SecondaryId;
    const UMLPackage *typePkg = m_pSecondary->umlPackage();
    if (typePkg != 0 && typePkg != umlPackage())
        return m_pSecondary->fullyQualifiedName();
    return m_pSecondary->name();
}

/**
 * Sets the type of the UMLAttribute.
 *
 * @param type      Pointer to the UMLObject of the type.
 */
void UMLClassifierListItem::setType(UMLObject *type)
{
    if (m_pSecondary != type) {
        m_pSecondary = type;
        UMLObject::emitModified();
    }
}

/**
 * Sets the type name of the UMLClassifierListItem.
 * DEPRECATED - use setType() instead.
 *
 * @param type      The type name of the UMLClassifierListItem.
 */
void UMLClassifierListItem::setTypeName(const QString &type)
{
    if (type.isEmpty() || type == QLatin1String("void")) {
        m_pSecondary = 0;
        m_SecondaryId.clear();
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    m_pSecondary = pDoc->findUMLObject(type);
    if (m_pSecondary == 0) {
        // Make data type for easily identified cases
        if (Model_Utils::isCommonDataType(type) || type.contains(QLatin1Char('*'))) {
            m_pSecondary = Object_Factory::createUMLObject(UMLObject::ot_Datatype, type);
            uDebug() << "created datatype for " << type;
        } else {
            m_SecondaryId = type;
        }
    }
    UMLObject::emitModified();
}

