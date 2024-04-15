/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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

DEBUG_REGISTER(UMLInstance)

/**
 * Construct UMLInstance
 * @param instanceName Name of the instance
 * @param id The unique id to assign
 * @param c  The UMLClassifier that this instance represents, or
 *           nullptr in case of an untyped instance.
 *           The classifier can also be set after construction
 *           by calling setClassifier().
 */
UMLInstance::UMLInstance(const QString &instanceName, Uml::ID::Type id,
                         UMLClassifier *c /* = nullptr */)
  : UMLCanvasObject(instanceName, id)
{
    m_BaseType = UMLObject::ot_Instance;
    setClassifierCmd(c, false);  // Signal shall not be emitted here
                                 // because we are stll in the constructor.
}

/**
 * Set undoable classifier
 * @param classifier
 */
void UMLInstance::setClassifier(UMLClassifier *classifier)
{
    if (m_pSecondary == classifier)
        return;

    //UMLApp::app()->executeCommand(new Uml::CmdRenameUMLInstanceType(this, classifier));
    m_pSecondary = classifier;
    emitModified();
}

/**
 * Set classifier
 * @param classifier  the classifier which is the type of this instance
 * @param emitSignal  flag controlling whether to Q_EMIT a modification signal (default: true)
 */
void UMLInstance::setClassifierCmd(UMLClassifier *classifier, bool emitSignal /* = true */)
{
    if (m_pSecondary == classifier)
        return;

    if (m_pSecondary) {
        subordinates().clear();
        disconnect(m_pSecondary, SIGNAL(attributeAdded(UMLClassifierListItem*)),
                   this, SLOT(attributeAdded(UMLClassifierListItem*)));
        disconnect(m_pSecondary, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                   this, SLOT(attributeRemoved(UMLClassifierListItem*)));
    }
    m_pSecondary = classifier;
    if (classifier) {
        UMLClassifierListItemList attrDefs = classifier->getFilteredList(UMLObject::ot_Attribute);
        for(UMLClassifierListItem *item : attrDefs) {
            uIgnoreZeroPointer(item);
            UMLAttribute *umla = item->asUMLAttribute();
            if (umla) {
                UMLInstanceAttribute *ia = new UMLInstanceAttribute(this, umla, umla->getInitialValue());
                subordinates().append(ia);
            } else {
                logError1("UMLInstance::setClassifierCmd : item %1 is not an attribute", item->name());
            }
        }
        connect(classifier, SIGNAL(attributeAdded(UMLClassifierListItem*)),
                this, SLOT(attributeAdded(UMLClassifierListItem*)));
        connect(classifier, SIGNAL(attributeRemoved(UMLClassifierListItem*)),
                this, SLOT(attributeRemoved(UMLClassifierListItem*)));
    }
    if (emitSignal)
        emitModified();
}

UMLClassifier *UMLInstance::classifier() const
{
    return (m_pSecondary ? m_pSecondary->asUMLClassifier() : nullptr);
}

/**
 * Creates the <UML:Instance> element including its slots.
 */
void UMLInstance::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("Instance"));
    if (m_pSecondary) {
        writer.writeAttribute(QStringLiteral("classifier"), Uml::ID::toString(m_pSecondary->id()));
        //save attributes
        for(UMLObject *pObject : subordinates()) {
            pObject->saveToXMI(writer);
        }
    }
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:Instance> element including its instanceAttributes.
 */
bool UMLInstance::load1(QDomElement &element)
{
    m_SecondaryId = element.attribute(QStringLiteral("classifier"));
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QStringLiteral("slot"))) {
            UMLInstanceAttribute *pInstanceAttribute = new UMLInstanceAttribute(this, 0);
            if (!pInstanceAttribute->loadFromXMI(tempElement)) {
                return false;
            }
            subordinates().append(pInstanceAttribute);
        }
        node = node.nextSibling();
    } // end while
    return true;
}

/**
 * Resolve forward declaration of referenced classifier held in m_secondaryId
 * after loading object from xmi file.
 * @return true - resolve was successful
 * @return false - resolve was not successful
 */
bool UMLInstance::resolveRef()
{
    if (m_SecondaryId.isEmpty()) {
        maybeSignalObjectCreated();
        return true;
    }
    if (m_pSecondary) {
        m_SecondaryId.clear();
        maybeSignalObjectCreated();
        return true;
    }
    if (!UMLObject::resolveRef()) {
        return false;
    }
    return (m_pSecondary != nullptr);
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

void UMLInstance::attributeAdded(UMLClassifierListItem *item)
{
    for (int i = 0; i < subordinates().count(); i++) {
        UMLObject *o = subordinates().at(i);
        if (o->parent() == item) {
            logDebug2("UMLInstance %1 attributeAdded(%2) : instanceAttribute already present",
                      name(), item->name());
            return;
        }
    }
    UMLAttribute *umla = item->asUMLAttribute();
    if (umla) {
        logDebug2("UMLInstance %1 attributeAdded(%2) : creating UMLInstanceAttribute",
                  name(), item->name());
        UMLInstanceAttribute *ia = new UMLInstanceAttribute(this, umla, umla->getInitialValue());
        subordinates().append(ia);
    } else {
        logError2("UMLInstance %1 attributeAdded(%2) : item is not a UMLAttribute",
                  name(), item->name());
    }
}

void UMLInstance::attributeRemoved(UMLClassifierListItem *item)
{
    for (int i = 0; i < subordinates().count(); i++) {
        UMLObject *o = subordinates().at(i);
        if (o->parent() == item) {
            logDebug3("UMLInstance %1 attributeRemoved(%2) : removing instanceAttribute at index %3",
                      name(), item->name(), i);
            subordinates().removeAt(i);
            return;
        }
    }
    logWarn1("UMLInstance::attributeRemoved(%1) : instanceAttribute not found", item->name());
}

