
#include "instance.h"

//local includes
#include "instanceattribute.h"
#include "umlinstanceattributedialog.h"
#include "umldoc.h"
#include "uml.h"
#include "debug_utils.h"

//kde includes
#include <KLocalizedString>
#include <KMessageBox>

UMLInstance::UMLInstance(const QString &name, Uml::ID::Type id)
    : UMLClassifier(name, id)
{
      m_BaseType = UMLObject::ot_Instance;
}


UMLAttribute *UMLInstance::createAttribute(const QString &name, UMLObject *type, Uml::Visibility::Enum vis, const QString &init)
{
    Uml::ID::Type id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_Attribute);
    } else {
        currentName = name;
    }
    UMLAttribute* newAttribute = new UMLAttribute(this, currentName, id, vis, type, init);

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
void UMLInstance::saveToXMI(QDomDocument &qDoc, QDomElement &qElement)
{
    QDomElement instanceElement = UMLObject::save(QLatin1String("UML:Instance"), qDoc);
    //save operations
    UMLClassifierListItemList instanceAttributes = getFilteredList(UMLObject::ot_instanceAttribute);
    UMLClassifierListItem* pinstanceAttribute = 0;
    foreach (pinstanceAttribute, instanceAttributes) {
        pinstanceAttribute->saveToXMI(qDoc, instanceElement);
    }

    UMLClassifierListItemList instanceConstraints = getFilteredList(UMLObject::ot_instanceConstraint);
    foreach(UMLClassifierListItem* cli, instanceConstraints) {
        cli->saveToXMI(qDoc, instanceElement);
    }

    qElement.appendChild(instanceElement);
}

/**
 * Loads the <UML:Instance> element including its instanceAttributes.
 */
bool UMLInstance::load(QDomElement &element)
{
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
            if(!pInstanceAttribute->loadFromXMI(tempElement)) {
                return false;
            }
            m_List.append(pInstanceAttribute);
        }
        node = node.nextSibling();
    }//end while
    return true;
}
