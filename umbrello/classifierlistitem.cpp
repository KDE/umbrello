/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
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
#include <klocale.h>

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
                                             const QString& name, Uml::IDType id)
  : UMLObject(parent, name, id)
{
    UMLObject *parentObj = const_cast<UMLObject*>(parent);
    UMLClassifier *pc = dynamic_cast<UMLClassifier*>(parentObj);
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
    UMLObject *parentObj = const_cast<UMLObject*>(parent);
    UMLClassifier *pc = dynamic_cast<UMLClassifier*>(parentObj);
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
}

/**
 * Returns a string representation of the list item.
 *
 * @param sig   What type of operation string to show.
 * @return  The string representation of the operation.
 */
QString UMLClassifierListItem::toString(Uml::SignatureType sig)
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
    return static_cast<UMLClassifier*>(m_pSecondary);
}

/**
 * Returns the type name of the UMLClassifierListItem.
 *
 * @return  The type name of the UMLClassifierListItem.
 */
QString UMLClassifierListItem::getTypeName() const
{
    if (m_pSecondary == NULL)
        return m_SecondaryId;
    const UMLPackage *typePkg = m_pSecondary->umlPackage();
    if (typePkg != NULL && typePkg != m_pUMLPackage)
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
    if (type.isEmpty() || type == "void") {
        m_pSecondary = NULL;
        m_SecondaryId.clear();
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->document();
    m_pSecondary = pDoc->findUMLObject(type);
    if (m_pSecondary == NULL) {
        // Make data type for easily identified cases
        if (Model_Utils::isCommonDataType(type) || type.contains('*')) {
            m_pSecondary = Object_Factory::createUMLObject(UMLObject::ot_Datatype, type);
            uDebug() << "created datatype for " << type;
        } else {
            m_SecondaryId = type;
        }
    }
    UMLObject::emitModified();
}

#include "classifierlistitem.moc"
