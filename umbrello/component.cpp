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

// own header
#include "component.h"
// qt/kde includes
#include <kdebug.h>
#include <klocale.h>
// app includes
#include "association.h"
#include "object_factory.h"
#include "model_utils.h"
#include "clipboard/idchangelog.h"

UMLComponent::UMLComponent(const QString & name, Uml::IDType id)
        : UMLPackage(name, id) {
    init();
}

UMLComponent::~UMLComponent() {
}

void UMLComponent::init() {
    m_BaseType = Uml::ot_Component;
    m_executable = false;
}

UMLObject* UMLComponent::clone() const {
    UMLComponent *clone = new UMLComponent();
    UMLObject::copyInto(clone);
    return clone;
}

void UMLComponent::saveToXMI(QDomDocument& qDoc, QDomElement& qElement) {
    QDomElement componentElement = UMLObject::save("UML:Component", qDoc);
    componentElement.setAttribute("executable", m_executable);
    // Save contained components if any.
    if (m_objects.count()) {
        QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );
        for (UMLObject *obj = m_objects.first(); obj; obj = m_objects.next())
            obj->saveToXMI (qDoc, ownedElement);
        componentElement.appendChild(ownedElement);
    }
    qElement.appendChild(componentElement);
}

bool UMLComponent::load(QDomElement& element) {
    QString executable = element.attribute("executable", "0");
    m_executable = (bool)executable.toInt();
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMIAttribute(type))
            continue;
        if (Uml::tagEq(type, "Namespace.ownedElement") ||
                Uml::tagEq(type, "Namespace.contents")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load(tempElement))
                return false;
            continue;
        }
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type);
        if( !pObject ) {
            kWarning() << "UMLComponent::load: "
                        << "Unknown type of umlobject to create: "
                        << type << endl;
            continue;
        }
        pObject->setUMLPackage(this);
        if (pObject->loadFromXMI(tempElement)) {
            addObject(pObject);
        } else {
            delete pObject;
        }
    }
    return true;
}

void UMLComponent::setExecutable(bool executable) {
    m_executable = executable;
}

bool UMLComponent::getExecutable() {
    return m_executable;
}

#include "component.moc"
