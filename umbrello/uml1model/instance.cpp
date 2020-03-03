/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2016-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "instance.h"

//app includes
#include "cmds.h"
#include "classifier.h"
#include "classpropertiesdialog.h"
#include "debug_utils.h"
#include "instanceattribute.h"
#include "object_factory.h"
#include "uml.h"
#include "umldoc.h"
#include "umlinstanceattributedialog.h"
#include "uniqueid.h"

//kde includes
#include <KLocalizedString>
#include <KMessageBox>

/**
 * Construct UMLInstance
 * @param name Name of referenced classifier
 * @param id The unique id to assign
 */
UMLInstance::UMLInstance(const QString &name, Uml::ID::Type id)
  : UMLClassifier(QString(), id)
{
    m_BaseType = UMLObject::ot_Instance;
    if (UMLApp::app()->document()->loading())
        return;
    UMLClassifier *c = Object_Factory::createUMLObject(UMLObject::ot_Class, name)->asUMLClassifier();
    Q_ASSERT(c);
    m_classifier = c;
}

/**
 * Set undoable type name
 * This method is used from property dialogs.
 * @param name type name to set
 */
void UMLInstance::setClassifierName(const QString &name)
{
    if (m_classifier->name() == name || m_classifier->fullyQualifiedName() == name)
        return;
    UMLClassifier *c = Object_Factory::createUMLObject(UMLObject::ot_Class, name)->asUMLClassifier();
    Q_ASSERT(c);
    UMLApp::app()->executeCommand(new Uml::CmdRenameUMLInstanceType(this, c));
    m_classifier = c;
    emitModified();
}

/**
 * return type name
 * @return type name
 */
QString UMLInstance::classifierName()
{
    return m_classifier->name();
}

/**
 * Set undoable classifier
 * @param classifier
 */
void UMLInstance::setClassifier(UMLClassifier *classifier)
{
    if (m_classifier == classifier)
        return;

    UMLApp::app()->executeCommand(new Uml::CmdRenameUMLInstanceType(this, classifier));
    m_classifier = classifier;
    emitModified();
}

/**
 * Set classifier
 * @param classifier
 */
void UMLInstance::setClassifierCmd(UMLClassifier *classifier)
{
    if (m_classifier == classifier)
        return;

    m_classifier = classifier;
    emitModified();
}

UMLClassifier *UMLInstance::classifier()
{
    return m_classifier;
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
    instanceElement.setAttribute(QLatin1String("classifier"), Uml::ID::toString(m_classifier->id()));
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
    m_SecondaryId = element.attribute(QLatin1String("classifier"));
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
            subordinates().append(pInstanceAttribute);
        }
        node = node.nextSibling();
    }//end while
    return true;
}

/**
 * Resolve forwarded declaration of referenced classifier hold in m_pSecondaryId
 * after loading object from xmi file.
 * @return true - resolve was successful
 * @return false - resolve was not successful
 */
bool UMLInstance::resolveRef()
{
    if (!UMLClassifier::resolveRef())
        return false;
    if (!m_pSecondary)
        return false;
    UMLClassifier *c = m_pSecondary->asUMLClassifier();
    Q_ASSERT(c);
    m_classifier = c;
    m_pSecondary = 0;
    return true;
}

/**
 * Display the properties configuration dialog.
 * @param parent Parent widget
 * @return true - configuration has been applied
 * @return false - configuration has not been applied
 */
bool UMLInstance::showPropertiesDialog(QWidget* parent)
{
    ClassPropertiesDialog dialog(parent, this);
    return dialog.exec();
}
