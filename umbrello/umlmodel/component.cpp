/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "component.h"
// app includes
#include "association.h"
#include "debug_utils.h"
#include "object_factory.h"
#include "model_utils.h"
#include "clipboard/idchangelog.h"
#include "optionstate.h"
#include "umldoc.h"
// kde includes
#include <KLocalizedString>

/**
 * Sets up a Component.
 * @param name   The name of the Concept.
 * @param id     The unique id of the Concept.
 */
UMLComponent::UMLComponent(const QString & name, Uml::ID::Type id)
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
void UMLComponent::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QLatin1String("Component"));
    writer.writeAttribute(QLatin1String("executable"), QString::number(m_executable));
    // Save contained components if any.
    if (m_objects.count()) {
        if (! Settings::optionState().generalState.uml2) {
            writer.writeStartElement(QLatin1String("UML:Namespace.ownedElement"));
        }
        for (UMLObjectListIt objectsIt(m_objects); objectsIt.hasNext();) {
            UMLObject* obj = objectsIt.next();
            uIgnoreZeroPointer(obj);
            obj->saveToXMI (writer);
        }
        if (! Settings::optionState().generalState.uml2) {
            writer.writeEndElement();
        }
    }
    UMLObject::save1end(writer);
}

/**
 * Loads the UML:Component element including its ports et al.
 */
bool UMLComponent::load1(QDomElement& element)
{
    QString executable = element.attribute(QLatin1String("executable"), QLatin1String("0"));
    m_executable = (bool)executable.toInt();
    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString type = tempElement.tagName();
        if (Model_Utils::isCommonXMI1Attribute(type))
            continue;
        if (UMLDoc::tagEq(type, QLatin1String("Namespace.ownedElement")) ||
                UMLDoc::tagEq(type, QLatin1String("Namespace.contents"))) {
            //CHECK: Umbrello currently assumes that nested elements
            // are ownedElements anyway.
            // Therefore these tags are not further interpreted.
            if (! load1(tempElement))
                return false;
            continue;
        }
        if (UMLDoc::tagEq(type, QLatin1String("ownedAttribute"))) {
            type = tempElement.attribute(QLatin1String("xmi:type"));
        }
        UMLObject *pObject = Object_Factory::makeObjectFromXMI(type);
        if (!pObject) {
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
bool UMLComponent::getExecutable() const
{
    return m_executable;
}

