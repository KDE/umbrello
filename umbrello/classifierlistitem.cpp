/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include <kdebug.h>
#include <klocale.h>

#include "classifierlistitem.h"
#include "classifier.h"
#include "uml.h"
#include "umldoc.h"
#include "model_utils.h"
#include "object_factory.h"

UMLClassifierListItem::UMLClassifierListItem(const UMLObject *parent, QString Name, Uml::IDType id)
        : UMLObject(parent, Name, id) {
    UMLObject *parentObj = const_cast<UMLObject*>(parent);
    UMLClassifier *pc = dynamic_cast<UMLClassifier*>(parentObj);
    if (pc)
        UMLObject::setUMLPackage(pc);
}

UMLClassifierListItem::UMLClassifierListItem(const UMLObject *parent)
        : UMLObject(parent) {
    UMLObject *parentObj = const_cast<UMLObject*>(parent);
    UMLClassifier *pc = dynamic_cast<UMLClassifier*>(parentObj);
    if (pc)
        UMLObject::setUMLPackage(pc);
}

UMLClassifierListItem::~UMLClassifierListItem() {
}

void UMLClassifierListItem::copyInto(UMLClassifierListItem *rhs) const
{
    // Call the parent.
    UMLObject::copyInto(rhs);
}

QString UMLClassifierListItem::toString(Uml::Signature_Type /*sig*/) {
    return getName();
}

UMLClassifier * UMLClassifierListItem::getType() {
    return static_cast<UMLClassifier*>(m_pSecondary);
}

QString UMLClassifierListItem::getTypeName() {
    if (m_pSecondary == NULL)
        return m_SecondaryId;
    const UMLPackage *typePkg = m_pSecondary->getUMLPackage();
    if (typePkg != NULL && typePkg != m_pUMLPackage)
        return m_pSecondary->getFullyQualifiedName();
    return m_pSecondary->getName();
}

void UMLClassifierListItem::setType(UMLObject *type) {
    if (m_pSecondary != type) {
        m_pSecondary = type;
        emit modified();
    }
}

void UMLClassifierListItem::setTypeName(const QString &type) {
    if (type.isEmpty() || type == "void") {
        m_pSecondary = NULL;
        m_SecondaryId = QString::null;
        return;
    }
    UMLDoc *pDoc = UMLApp::app()->getDocument();
    m_pSecondary = pDoc->findUMLObject(type);
    if (m_pSecondary == NULL) {
        // Make data type for easily identified cases
        if (Model_Utils::isCommonDataType(type) || type.contains('*')) {
            m_pSecondary = Object_Factory::createUMLObject(Uml::ot_Datatype, type);
            kdDebug() << "UMLClassifierListItem::setTypeName: "
            << "created datatype for " << type << endl;
        } else {
            m_SecondaryId = type;
        }
    }
    emit modified();
}


#include "classifierlistitem.moc"
