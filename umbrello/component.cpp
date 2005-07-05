/*
 *  copyright (C) 2003-2004
 *  Umbrello UML Modeller Authors <uml-devel@ uml.sf.net>
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "component.h"
#include "association.h"
#include "clipboard/idchangelog.h"
#include <kdebug.h>
#include <klocale.h>

UMLComponent::UMLComponent(const QString & name, Uml::IDType id)
        : UMLCanvasObject(name, id) {
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
    qElement.appendChild(componentElement);
}

bool UMLComponent::load(QDomElement& element) {
    QString executable = element.attribute("executable", "0");
    m_executable = (bool)executable.toInt();
    return true;
}

void UMLComponent::setExecutable(bool executable) {
    m_executable = executable;
}

bool UMLComponent::getExecutable() {
    return m_executable;
}

#include "component.moc"
