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
#include "component.h"
// app includes
#include "association.h"
#include "debug_utils.h"
#include "object_factory.h"
#include "model_utils.h"
#include "clipboard/idchangelog.h"
#include "umldoc.h"
// kde includes
#include <klocale.h>

/**
 * Sets up a Component.
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLComponent::UMLComponent(const QString & name, Uml::IDType id)
  : UMLPackage(name, id),
    m_executable(false)
{
    m_BaseType = UMLObject::ot_Component;
}

/**
 * Destructor.
 */
UMLComponent::~UMLComponent()
{
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLComponent::clone() const
{
    UMLComponent *clone = new UMLComponent();
    UMLObject::copyInto(clone);
    return clone;
}

/**
 * Creates the UML:Component element including its operations,
 * attributes and templates
 */
void UMLComponent::saveToXMI(QDomDocument& qDoc, QDomElement& qElement)
{
    QDomElement componentElement = UMLObject::save("UML:Component", qDoc);
    componentElement.setAttribute("executable", m_executable);
    // Save contained components if any.
    if (m_objects.count()) {
        QDomElement ownedElement = qDoc.createElement( "UML:Namespace.ownedElement" );
        for (UMLObjectListIt objectsIt( m_objects ); objectsIt.hasNext(); ) {
            UMLObject* obj = objectsIt.next();
            obj->saveToXMI (qDoc, ownedElement);
        }
        componentElement.appendChild(ownedElement);
    }
    qElement.appendChild(componentElement);
}

/**
 * Loads the UML:Component element including its operations,
 * attributes and templates
 */
bool UMLComponent::load(QDomElement& element)
{
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
        if (UMLDoc::tagEq(type, "Namespace.ownedElement") ||
                UMLDoc::tagEq(type, "Namespace.contents")) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load(tempElement))
                return false;
            continue;
        }
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type);
        if( !pObject ) {
            uWarning() << "Unknown type of umlobject to create: " << type;
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

/**
 * Sets m_executable.
 */
void UMLComponent::setExecutable(bool executable)
{
    m_executable = executable;
}

/**
 * Returns the value of m_executable.
 */
bool UMLComponent::getExecutable()
{
    return m_executable;
}

#include "component.moc"
