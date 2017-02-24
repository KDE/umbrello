/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2017                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "instance.h"

//local includes
#include "cmds/generic/cmdrenameumlobjectinstance.h"
#include "instanceattribute.h"
#include "umlinstanceattributedialog.h"
#include "umldoc.h"
#include "uml.h"
#include "debug_utils.h"
#include "uniqueid.h"
//kde includes
#include <KLocalizedString>
#include <KMessageBox>

UMLInstance::UMLInstance(const QString &name, Uml::ID::Type id)
  : UMLClassifier(name, id)
{
    m_BaseType = UMLObject::ot_Instance;
}

QString UMLInstance::instanceName() const
{
    return m_instanceName;
}

/**
 * @brief UMLObject::setInstanceName
 * Set object/instance name in case of a Object Diagram
 */
void UMLInstance::setInstanceName(const QString &strName)
{
     if(instanceName() != strName)
        UMLApp::app()->executeCommand(new Uml::CmdRenameUMLObjectInstance(this, strName));
}

void UMLInstance::setInstanceNameCmd(const QString &strName)
{
    m_instanceName = strName;
    emitModified();
}

UMLAttribute *UMLInstance::createAttribute(const QString &name, UMLObject *type, Uml::Visibility::Enum vis, const QString &init)
{
    Uml::ID::Type id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_InstanceAttribute);
    } else {
        currentName = name;
    }
    UMLInstanceAttribute* newAttribute = new UMLInstanceAttribute(this, currentName, id, vis, type, init);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == QDialog::Accepted && !goodName && name.isNull()) {
        QPointer<UMLInstanceAttributeDialog> attributeDialog = new UMLInstanceAttributeDialog(0, newAttribute);
        button = attributeDialog->exec();
        QString name = newAttribute->name();

        if(name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (findChildObject(name) != NULL) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
        delete attributeDialog;
    }

    if (button != QDialog::Accepted) {
        delete newAttribute;
        return NULL;
    }

    addAttribute(newAttribute);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newAttribute);
    return newAttribute;
}
/**
 * Creates the <UML:Instance> element including its entityliterals.
 */
void UMLInstance::saveToXMI1(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement instanceElement = UMLObject::save1(QLatin1String("UML:Instance"), qDoc);
    instanceElement.setAttribute(QLatin1String("instancename"), m_instanceName);
    //save attributes
    UMLClassifierListItemList instanceAttributes = getFilteredList(UMLObject::ot_InstanceAttribute);
    UMLClassifierListItem* pInstanceAttribute = 0;
    foreach (pInstanceAttribute, instanceAttributes) {
        pInstanceAttribute->saveToXMI1(qDoc, instanceElement);
    }
    qElement.appendChild(instanceElement);
}

/**
 * Loads the <UML:Instance> element including its instanceAttributes.
 */
bool UMLInstance::load1(QDomElement &element)
{
    m_instanceName = element.attribute(QLatin1String("instancename"));
    QDomNode node = element.firstChild();
    while(!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QLatin1String("InstanceAttribute"))) {   // for backward compatibility
            UMLInstanceAttribute* pInstanceAttribute = new UMLInstanceAttribute(this);
            if(!pInstanceAttribute->loadFromXMI1(tempElement)) {
                return false;
            }
            m_List.append(pInstanceAttribute);
        }
        node = node.nextSibling();
    }//end while
    return true;
}
