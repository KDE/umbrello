/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "entity.h"

// app includes
#include "debug_utils.h"
#include "entityattribute.h"
#include "uniqueconstraint.h"
#include "foreignkeyconstraint.h"
#include "checkconstraint.h"
#include "umldoc.h"
#include "uml.h"
#include "uniqueid.h"
#include "umlentityattributelist.h"
#include "umlentityconstraintlist.h"
#include "idchangelog.h"
#include "umlentityattributedialog.h"
#include "umluniqueconstraintdialog.h"
#include "umlforeignkeyconstraintdialog.h"
#include "umlcheckconstraintdialog.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes
#include <QPointer>

DEBUG_REGISTER(UMLEntity)

/**
 * Constructor.
 */
UMLEntity::UMLEntity(const QString& name, Uml::ID::Type id)
  : UMLClassifier(name, id),
    m_PrimaryKey(0)
{
    m_BaseType = UMLObject::ot_Entity;
    connect(this, SIGNAL(entityAttributeRemoved(UMLClassifierListItem*)),
            this, SLOT(slotEntityAttributeRemoved(UMLClassifierListItem*)));
}

/**
 * Standard destructor.
 */
UMLEntity::~UMLEntity()
{
    subordinates().clear();
}

/**
 * Overloaded '==' operator.
 */
bool UMLEntity::operator==(const UMLEntity& rhs) const
{
    return UMLClassifier::operator==(rhs);
}

/**
 * Copy the internal presentation of this object into the new
 * object.
 */
void UMLEntity::copyInto(UMLObject *lhs) const
{
    UMLEntity *target = lhs->asUMLEntity();

    // call base class copy function
    UMLClassifier::copyInto(target);

    // copy local data items
    target->m_PrimaryKey = m_PrimaryKey;
}

/**
 * Make a clone of this object.
 */
UMLObject* UMLEntity::clone() const
{
    UMLEntity* clone = new UMLEntity();
    copyInto(clone);

    return clone;
}

/**
 * Create a UMLAttribute.
 * @param name   an optional name for the attribute
 * @param type   an optional type object for the attribute
 * @param vis    the visibility of the attribute
 * @param iv     the initial value for the attribute
 * @return   the just created attribute or null 
 */
UMLAttribute *UMLEntity::createAttribute(const QString &name /*= QString()*/, UMLObject *type /*= nullptr*/,
                                         Uml::Visibility::Enum vis /* = Uml::Visibility::Private*/,
                                         const QString& iv /* = QString()*/)
{
    Uml::ID::Type id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_EntityAttribute);
    } else {
        currentName = name;
    }

    UMLEntityAttribute* newAttribute = new UMLEntityAttribute(this, currentName, id, vis, type, iv);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == QDialog::Accepted && !goodName && name.isNull()) {
        QPointer<UMLEntityAttributeDialog> dialog = new UMLEntityAttributeDialog(0, newAttribute);
        button = dialog->exec();
        QString name = newAttribute->name();

        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (findChildObject(name) != 0) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
        delete dialog;
    }

    if (button != QDialog::Accepted) {
        delete newAttribute;
        return 0;
    }

    addEntityAttribute(newAttribute);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newAttribute);
    return newAttribute;
}

/**
 * Creates a Unique Constraint for this Entity.
 * @param name   an optional name
 * @return the UniqueConstraint created
 */
UMLUniqueConstraint* UMLEntity::createUniqueConstraint(const QString &name)
{
    Uml::ID::Type id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {

        /**
         *  @todo check parameter
         */
        currentName = uniqChildName(UMLObject::ot_UniqueConstraint);
    } else {
        currentName = name;
    }

    UMLUniqueConstraint* newUniqueConstraint = new UMLUniqueConstraint(this, currentName, id);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == QDialog::Accepted && !goodName && name.isNull()) {
        QPointer<UMLUniqueConstraintDialog> dialog = new UMLUniqueConstraintDialog(0, newUniqueConstraint);
        button = dialog->exec();
        QString name = newUniqueConstraint->name();

        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (findChildObject(name) != 0) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
        delete dialog;
    }

    if (button != QDialog::Accepted) {
        delete newUniqueConstraint;
        return 0;
    }

    addConstraint(newUniqueConstraint);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newUniqueConstraint);
    emitModified();
    return newUniqueConstraint;
}

/**
 * Creates a Foreign Key  Constraint for this Entity.
 * @param name   an optional name
 * @return the ForeignKeyConstraint created
 */
UMLForeignKeyConstraint* UMLEntity::createForeignKeyConstraint(const QString &name)
{
    Uml::ID::Type id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_ForeignKeyConstraint);
    } else {
        currentName = name;
    }

    UMLForeignKeyConstraint* newForeignKeyConstraint = new UMLForeignKeyConstraint(this, currentName, id);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == QDialog::Accepted && !goodName && name.isNull()) {
        QPointer<UMLForeignKeyConstraintDialog> dialog = new UMLForeignKeyConstraintDialog(0, newForeignKeyConstraint);
        button = dialog->exec();
        QString name = newForeignKeyConstraint->name();

        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (findChildObject(name) != 0) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
        delete dialog;
    }

    if (button != QDialog::Accepted) {
        return 0;
    }

    addConstraint(newForeignKeyConstraint);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newForeignKeyConstraint);
    emitModified();
    return newForeignKeyConstraint;
}

/**
 * Creates a Check  Constraint for this Entity.
 * @param name   an optional name
 * @return the CheckConstraint created
 */
UMLCheckConstraint* UMLEntity::createCheckConstraint(const QString &name)
{
    Uml::ID::Type id = UniqueID::gen();
    QString currentName;
    if (name.isNull())  {
        currentName = uniqChildName(UMLObject::ot_CheckConstraint);
    } else {
        currentName = name;
    }

    UMLCheckConstraint* newCheckConstraint = new UMLCheckConstraint(this, currentName, id);

    int button = QDialog::Accepted;
    bool goodName = false;

    //check for name.isNull() stops dialog being shown
    //when creating attribute via list view
    while (button == QDialog::Accepted && !goodName && name.isNull()) {
        QPointer<UMLCheckConstraintDialog> dialog = new UMLCheckConstraintDialog(0, newCheckConstraint);
        button = dialog->exec();
        QString name = newCheckConstraint->name();

        if (name.length() == 0) {
            KMessageBox::error(0, i18n("That is an invalid name."), i18n("Invalid Name"));
        } else if (findChildObject(name) != 0) {
            KMessageBox::error(0, i18n("That name is already being used."), i18n("Not a Unique Name"));
        } else {
            goodName = true;
        }
        delete dialog;
    }

    if (button != QDialog::Accepted) {
        return 0;
    }

    addConstraint(newCheckConstraint);

    UMLDoc *umldoc = UMLApp::app()->document();
    umldoc->signalUMLObjectCreated(newCheckConstraint);
    emitModified();
    return newCheckConstraint;
}

/**
 * Adds an entityAttribute.
 * The entityAttribute object must not belong to any other classifier.
 * @param name   name of the UMLEntityAttribute
 * @param id     id of the UMLEntityAttribute
 * @return  True if the entityAttribute was successfully added.
 */
UMLObject* UMLEntity::addEntityAttribute(const QString& name, Uml::ID::Type id)
{
    UMLEntityAttribute* literal = new UMLEntityAttribute(this, name, id);
    subordinates().append(literal);
    Q_EMIT entityAttributeAdded(literal);
    UMLObject::emitModified();
    connect(literal, SIGNAL(modified()), this, SIGNAL(modified()));
    return literal;
}

/**
 * Adds an already created entityAttribute.
 * The entityAttribute object must not belong to any other classifier.
 * @param att   Pointer to the UMLEntityAttribute.
 * @param log   Pointer to the IDChangeLog.
 * @return  True if the entityAttribute was successfully added.
 */
bool UMLEntity::addEntityAttribute(UMLEntityAttribute *att, IDChangeLog* log /* = nullptr*/)
{
    QString name = (QString)att->name();
    if (findChildObject(name) == 0) {
        att->setParent(this);
        subordinates().append(att);
        Q_EMIT entityAttributeAdded(att);
        UMLObject::emitModified();
        connect(att, SIGNAL(modified()), this, SIGNAL(modified()));
        return true;
    } else if (log) {
        log->removeChangeByNewID(att->id());
        delete att;
    }
    return false;
}

/**
 * Adds an entityAttribute to the entity, at the given position.
 * If position is negative or too large, the entityAttribute is added
 * to the end of the list.
 * TODO:  give default value -1 to position (append) - now it conflicts with the method above..
 * @param att       Pointer to the UMLEntityAttribute.
 * @param position  Position index for the insertion.
 * @return  True if the entityAttribute was successfully added.
 */
bool UMLEntity::addEntityAttribute(UMLEntityAttribute* att, int position)
{
    Q_ASSERT(att);
    QString name = (QString)att->name();
    if (findChildObject(name) == 0) {
        att->setParent(this);
        if (position >= 0 && position <= (int)subordinates().count())  {
            subordinates().insert(position, att);
        } else {
            subordinates().append(att);
        }
        Q_EMIT entityAttributeAdded(att);
        UMLObject::emitModified();
        connect(att, SIGNAL(modified()), this, SIGNAL(modified()));
        return true;
    }
    return false;
}

/**
 * Removes an entityAttribute from the class.
 * @param att   The entityAttribute to remove.
 * @return  Count of the remaining entityAttributes after removal.
 *          Returns -1 if the given entityAttribute was not found.
 */
int UMLEntity::removeEntityAttribute(UMLClassifierListItem* att)
{
    if (!subordinates().removeAll((UMLEntityAttribute*)att)) {
        logDebug0("UMLEntity::removeEntityAttribute cannot find att given in list");
        return -1;
    }
    Q_EMIT entityAttributeRemoved(att);
    UMLObject::emitModified();
    // If we are deleting the object, then we don't need to disconnect..this is done auto-magically
    // for us by QObject. -b.t.
    // disconnect(att, SIGNAL(modified()), this, SIGNAL(modified()));
    delete att;
    return subordinates().count();
}

/**
 * Returns the number of entityAttributes for the class.
 * @return  The number of entityAttributes for the class.
 */
int UMLEntity::entityAttributes() const
{
    UMLClassifierListItemList entityAttributes = getFilteredList(UMLObject::ot_EntityAttribute);
    return entityAttributes.count();
}

/**
 * Emit the entityAttributeRemoved signal.
 */
void UMLEntity::signalEntityAttributeRemoved(UMLClassifierListItem *eattr)
{
    Q_EMIT entityAttributeRemoved(eattr);
}

/**
 * Resolve the types referenced by our UMLEntityAttributes.
 * Reimplements the method from UMLClassifier.
 */
bool UMLEntity::resolveRef()
{
    bool success = UMLClassifier::resolveRef();
    for(UMLObject  *obj : subordinates()) {
        if (obj->resolveRef()) {
            UMLClassifierListItem *cli = obj->asUMLClassifierListItem();
            if (!cli)
                return success;
            switch (cli->baseType()) {
                case UMLObject::ot_EntityAttribute:
                    Q_EMIT entityAttributeAdded(cli);
                    break;
                case UMLObject::ot_UniqueConstraint:
                case UMLObject::ot_ForeignKeyConstraint:
                    Q_EMIT entityConstraintAdded(cli);
                    break;
                default:
                    break;
            }
        }
    }
    return success;
}

/**
 * Creates the <UML:Entity> element including its entityliterals.
 */
void UMLEntity::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("Entity"));

    // save entity attributes
    UMLClassifierListItemList entityAttributes = getFilteredList(UMLObject::ot_EntityAttribute);
    UMLClassifierListItem *pEntityAttribute = nullptr;
    for(pEntityAttribute: entityAttributes) {
        pEntityAttribute->saveToXMI(writer);
    }
    // save entity constraints
    UMLClassifierListItemList entityConstraints = getFilteredList(UMLObject::ot_EntityConstraint);
    for(UMLClassifierListItem *cli : entityConstraints) {
        cli->saveToXMI(writer);
    }

    UMLObject::save1end(writer);
}

/**
 * Loads the <UML:Entity> element including its entityAttributes.
 */
bool UMLEntity::load1(QDomElement& element)
{
    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QStringLiteral("ownedAttribute")) || UMLDoc::tagEq(tag, QStringLiteral("packagedElement"))) {
            tag = tempElement.attribute(QStringLiteral("xmi:type"));
        }
        if (UMLDoc::tagEq(tag, QStringLiteral("EntityAttribute"))) {   // for backward compatibility
            UMLEntityAttribute* pEntityAttribute = new UMLEntityAttribute(this);
            if(!pEntityAttribute->loadFromXMI(tempElement)) {
                return false;
            }
            subordinates().append(pEntityAttribute);
        } else if (UMLDoc::tagEq(tag, QStringLiteral("UniqueConstraint"))) {
            UMLUniqueConstraint* pUniqueConstraint = new UMLUniqueConstraint(this);
            if (!pUniqueConstraint->loadFromXMI(tempElement)) {
                return false;
            }
            addConstraint(pUniqueConstraint);
        } else if (UMLDoc::tagEq(tag, QStringLiteral("ForeignKeyConstraint"))) {
            UMLForeignKeyConstraint* pForeignKeyConstraint = new UMLForeignKeyConstraint(this);
            if (!pForeignKeyConstraint->loadFromXMI(tempElement)) {
                return false;
            }

            addConstraint(pForeignKeyConstraint);
        } else if (UMLDoc::tagEq(tag, QStringLiteral("CheckConstraint"))) {

            UMLCheckConstraint* pCheckConstraint = new UMLCheckConstraint(this);
            if (!pCheckConstraint->loadFromXMI(tempElement)) {
                return false;
            }

            addConstraint(pCheckConstraint);
        } else if (tag == QStringLiteral("stereotype")) {
            logDebug1("UMLEntity::load1 %1: losing old-format stereotype.", name());
        } else {
            logWarn1("UMLEntity::load1: unknown child type %1", tag);
        }
        node = node.nextSibling();
    }//end while
    return true;
}

/**
 * Sets the UniqueConstraint passed as the Primary Key of this Entity
 * If the UniqueConstraint exists, then it is made a primary key
 * Else the UniqueConstraint is added and set as PrimaryKey
 * @param uconstr The Unique Constraint that is  to be set as Primary Key
 * @return true if Primary key could be set successfully
 */
bool UMLEntity::setAsPrimaryKey(UMLUniqueConstraint* uconstr)
{
    if (uconstr == 0) {
        logWarn0("UMLEntity::setAsPrimaryKey: NULL value passed. To unset a Primary Key use "
                  "unsetPrimaryKey()");
        return false;
    }

    if (uconstr->umlParent()->asUMLEntity() != this) {

        logDebug1("UMLEntity::setAsPrimaryKey: Parent of %1 does not match with current entity",
                  uconstr->name());
        return false;
    }

    // check if this constraint already exists as a unique constraint for this entity
    UMLObject *o = findChildObjectById(uconstr->id());
    UMLUniqueConstraint* uuc = o ? o->asUMLUniqueConstraint() : 0;
    if (uuc == 0) {
        addConstraint(uconstr);
        uuc = uconstr;
    }

    UMLUniqueConstraint* oldPrimaryKey = m_PrimaryKey;

    m_PrimaryKey = uuc;

    if (oldPrimaryKey != 0)
        oldPrimaryKey->emitModified();

    uuc->emitModified();
    emitModified();
    return true;
}

/**
 * Unset a Primary Key Constraint if it exists, else does nothing
 * This function will make the primary key into just another UniqueConstraint
 * if it exists
 */
void UMLEntity::unsetPrimaryKey()
{
    m_PrimaryKey = 0;
}

/**
 * Checks if This UMLEntity has a primary key set.
 * @return true if a Primary Key Exists for this UMLEntity
 */
bool UMLEntity::hasPrimaryKey() const
{
    if (m_PrimaryKey) {
        return true;
    }

    return false;
}

/**
 * Adds a Constraint to this UMLEntity.
 * To set a UMLUniqueConstraint as Primary Key use setAsPrimaryKey.
 * @param constr The UMLEntityConstraint that is to be added
 * @return true if the constraint could be added successfully
 */
bool UMLEntity::addConstraint(UMLEntityConstraint* constr)
{
    if (findChildObjectById(constr->id()) != 0) {
        logDebug1("UMLEntity::addConstraint: Constraint with id %1 already exists",
                  Uml::ID::toString(constr->id()));
        return false;
    }

    subordinates().append(constr);

    Q_EMIT entityConstraintAdded(constr);
    UMLObject::emitModified();
    connect(constr, SIGNAL(modified()), this, SIGNAL(modified()));

    return true;
}

/**
 * Removes an existing constraint from this UMLEntity.
 * If the Constraint is a Primary Key, this Entity will no longer have a PrimaryKey.
 * @param constr   the constraint to be removed
 * @return true if the constraint could be removed successfully
 */
bool UMLEntity::removeConstraint(UMLEntityConstraint* constr)
{
     if (findChildObjectById(constr->id()) == 0) {
        logDebug1("UMLEntity::removeConstraint: Constraint with id %1 does not exist",
                  Uml::ID::toString(constr->id()));
        return false;
    }

    if (m_PrimaryKey == constr) {
        unsetPrimaryKey();
    }

    subordinates().removeAll(constr);

    Q_EMIT entityConstraintRemoved(constr);
    UMLObject::emitModified();

    delete constr;
    return true;
}

/**
 * Slot for entity attribute removed.
 */
void UMLEntity::slotEntityAttributeRemoved(UMLClassifierListItem* cli)
{
    // this function does some cleanjobs related to this entity when the attribute is
    // removed, like, removing the attribute from all constraints

    UMLEntityAttribute* entAtt = cli->asUMLEntityAttribute();
    if (cli) {
       UMLClassifierListItemList ual = this->getFilteredList(UMLObject::ot_UniqueConstraint);

       for(UMLClassifierListItem *ucli :  ual) {
           UMLUniqueConstraint* uuc = ucli->asUMLUniqueConstraint();
           if (uuc->hasEntityAttribute(entAtt)) {
               uuc->removeEntityAttribute(entAtt);
           }
       }
    }

}

/**
 * Reimplementation of getFilteredList to support ot=UMLObject::ot_EntityConstraint.
 */
UMLClassifierListItemList UMLEntity::getFilteredList(UMLObject::ObjectType ot) const
{
    if (ot == UMLObject::ot_EntityConstraint) {
        UMLClassifierListItemList ucList, fcList, ccList, rcList;
        ucList = UMLClassifier::getFilteredList(UMLObject::ot_UniqueConstraint);
        fcList = UMLClassifier::getFilteredList(UMLObject::ot_ForeignKeyConstraint);
        ccList = UMLClassifier::getFilteredList(UMLObject::ot_CheckConstraint);

        // append the lists to rcList
        // first the Unique Constraints
        for(UMLClassifierListItem *ucli : ucList) {
            rcList.append(ucli);
        }

        // then the Foreign Key Constraints
        for(UMLClassifierListItem *ucli : fcList) {
            rcList.append(ucli);
        }

        for(UMLClassifierListItem *ucli : ccList) {
            rcList.append(ucli);
        }

        return rcList;
    } else {
        return UMLClassifier::getFilteredList(ot);
    }
}

/**
 * Checks if a given Unique Constraint is primary key of this entity
 * @param uConstr   a Unique Constraint
 * @return bool true if passed parameter is a primary key of this entity
 */
bool UMLEntity::isPrimaryKey(const UMLUniqueConstraint* uConstr) const
{
    if (uConstr == m_PrimaryKey) {
        return true;
    }

    return false;
}

/**
 * Returns the Entity Attributes.
 * Same as getFilteredList(UMLObject::ot_EntityAttribute).
 */
UMLEntityAttributeList UMLEntity::getEntityAttributes() const
{
    UMLEntityAttributeList entityAttributeList;
    for(UMLObject  *listItem : subordinates()) {
        if (listItem->baseType() == UMLObject::ot_EntityAttribute) {
            entityAttributeList.append(listItem->asUMLEntityAttribute());
        }
    }
    return entityAttributeList;
}


/**
 * Create a new ClassifierListObject (entityattribute)
 * according to the given XMI tag.
 * Returns NULL if the string given does not contain one of the tags
 * <UML:EntityAttribute>
 * Used by the clipboard for paste operation.
 * Reimplemented from UMLClassifier for UMLEntity
 */
UMLClassifierListItem* UMLEntity::makeChildObject(const QString& xmiTag)
{
    UMLClassifierListItem *pObject = nullptr;
    if (UMLDoc::tagEq(xmiTag, QStringLiteral("EntityAttribute"))) {
        pObject = new UMLEntityAttribute(this);
    }
    return pObject;
}

