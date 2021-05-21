/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
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
 * @param classifier
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
        foreach (UMLClassifierListItem *item, attrDefs) {
            uIgnoreZeroPointer(item);
            UMLAttribute *umla = item->asUMLAttribute();
            if (umla) {
                UMLInstanceAttribute *ia = new UMLInstanceAttribute(this, umla, umla->getInitialValue());
                subordinates().append(ia);
            } else {
                uError() << "UMLInstance::setClassifierCmd : item is not an attriubte";
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

UMLClassifier *UMLInstance::classifier()
{
    return (m_pSecondary ? m_pSecondary->asUMLClassifier() : nullptr);
}

/**
 * Creates the <UML:Instance> element including its slots.
 */
void UMLInstance::saveToXMI1(QXmlStreamWriter& writer)
{
    UMLObject::save1(QLatin1String("UML:Instance"), writer);
    if (m_pSecondary) {
        writer.writeAttribute(QLatin1String("classifier"), Uml::ID::toString(m_pSecondary->id()));
        //save attributes
        foreach (UMLObject *pObject, subordinates()) {
            pObject->saveToXMI1(writer);
        }
    }
    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:Instance> element including its instanceAttributes.
 */
bool UMLInstance::load1(QDomElement &element)
{
    m_SecondaryId = element.attribute(QLatin1String("classifier"));
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QLatin1String("slot"))) {
            UMLInstanceAttribute *pInstanceAttribute = new UMLInstanceAttribute(this, 0);
            if (!pInstanceAttribute->loadFromXMI1(tempElement)) {
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
            uDebug() << "UMLInstance::attributeAdded(" << item->name()
                     << ") : instanceAttribute already present";
            return;
        }
    }
    UMLAttribute *umla = item->asUMLAttribute();
    if (umla) {
        uDebug() << "UMLInstance::attributeAdded(" << item->name()
                 << ") : creating UMLInstanceAttribute";
        UMLInstanceAttribute *ia = new UMLInstanceAttribute(this, umla, umla->getInitialValue());
        subordinates().append(ia);
    } else {
        uError() << "UMLInstance::attributeAdded(" << item->name()
                 << ") : item is not a UMLAttribute";
    }
}

void UMLInstance::attributeRemoved(UMLClassifierListItem *item)
{
    for (int i = 0; i < subordinates().count(); i++) {
        UMLObject *o = subordinates().at(i);
        if (o->parent() == item) {
            uDebug() << "UMLInstance::attributeRemoved(" << item->name()
                     << ") : removing instanceAttribute at index " << i;
            subordinates().removeAt(i);
            return;
        }
    }
    uWarning() << "UMLInstance::attributeRemoved(" << item->name()
               << ") : instanceAttribute not found";
}

