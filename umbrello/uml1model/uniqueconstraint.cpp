/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

//own header
#include "uniqueconstraint.h"

// app includes
#include "debug_utils.h"
#include "entity.h"
#include "entityattribute.h"
#include "umldoc.h"
#include "uml.h"
#include "umlattributedialog.h"
#include "umluniqueconstraintdialog.h"
#include "object_factory.h"

/**
 * Sets up a constraint.
 *
 * @param parent    The parent of this UMLUniqueConstraint.
 * @param name      The name of this UMLUniqueConstraint.
 * @param id        The unique id given to this UMLUniqueConstraint.
 */
UMLUniqueConstraint::UMLUniqueConstraint(UMLObject *parent, const QString& name, Uml::ID::Type id)
  : UMLEntityConstraint(parent, name, id)
{
    init();
}

/**
 * Sets up a constraint.
 *
 * @param parent    The parent of this UMLUniqueConstraint.
 */
UMLUniqueConstraint::UMLUniqueConstraint(UMLObject *parent)
  : UMLEntityConstraint(parent)
{
    init();
}

/**
 * Overloaded '==' operator
 */
bool UMLUniqueConstraint::operator==(const  UMLUniqueConstraint &rhs) const
{
    if(this == &rhs)
        return true;

    if(!UMLObject::operator==(rhs))
        return false;

    return true;
}

/**
 * Destructor.
 */
UMLUniqueConstraint::~UMLUniqueConstraint()
{
}

/**
 * Copy the internal presentation of this object into the UMLUniqueConstraint
 * object.
 */
void UMLUniqueConstraint::copyInto(UMLObject *lhs) const
{
    UMLUniqueConstraint *target = lhs->asUMLUniqueConstraint();

    // call the parent first.
    UMLEntityConstraint::copyInto(target);

    // Copy all datamembers
    target->m_EntityAttributeList.clear();
    bool valid = true;
    foreach(UMLEntityAttribute* attr, m_EntityAttributeList) {
       if (!valid)
           break;
       valid = target->addEntityAttribute(attr);
    }

    if (!valid) {
        target->m_EntityAttributeList.clear();
        uDebug() <<"Copying Attributes Failed : Target list cleared instead";
    }
}

/**
 * Make a clone of the UMLUniqueConstraint.
 */
UMLObject* UMLUniqueConstraint::clone() const
{
    //FIXME: The new attribute should be slaved to the NEW parent not the old.
    UMLUniqueConstraint *clone = new UMLUniqueConstraint(umlParent());
    copyInto(clone);
    return clone;
}

/**
 * Returns a string representation of the UMLUniqueConstraint.
 *
 * @param sig  If true will show the attribute type and initial value.
 * @return  Returns a string representation of the UMLAttribute.
 */
QString UMLUniqueConstraint::toString(Uml::SignatureType::Enum sig,
                                      bool withStereotype) const
{
    Q_UNUSED(withStereotype);
    QString s;

    if (sig == Uml::SignatureType::ShowSig || sig == Uml::SignatureType::SigNoVis) {
        s = name() + QLatin1Char(':');

        const UMLEntity *e = umlParent()->asUMLEntity();
        if (e && e->isPrimaryKey(this)) {
           s += QLatin1String("Primary Key (");
        } else {
           s += QLatin1String("Unique (");
        }

        bool first = true;
        foreach(UMLEntityAttribute* att, m_EntityAttributeList) {
            if (first) {
               first = false;
            } else
                s += QLatin1Char(',');
            s += att->name();
        }
        s +=  QLatin1Char(')') ;
    }

    return s;
}

QString UMLUniqueConstraint::getFullyQualifiedName(const QString& separator,
                                                   bool includeRoot) const
{
    Q_UNUSED(separator); Q_UNUSED(includeRoot);
    return this->name();
}

/**
 * Creates the <UML:UniqueConstraint> XMI element.
 */
void UMLUniqueConstraint::saveToXMI1(QXmlStreamWriter& writer)
{
    UMLObject::save1(QLatin1String("UML:UniqueConstraint"), writer);

    const UMLEntity* parentEnt = umlParent()->asUMLEntity();
    if (parentEnt && parentEnt->isPrimaryKey(this)) {
        writer.writeAttribute(QLatin1String("isPrimary"), QLatin1String("1"));
    } else {
        writer.writeAttribute(QLatin1String("isPrimary"), QLatin1String("0"));
    }

    foreach(UMLEntityAttribute* att, m_EntityAttributeList) {
        att->saveToXMI1(writer);
    }

    writer.writeEndElement();
}

/**
 * Display the properties configuration dialog for the attribute.
 */
bool UMLUniqueConstraint::showPropertiesDialog(QWidget* parent)
{
    UMLUniqueConstraintDialog dialog(parent, this);
    return dialog.exec();
}

/**
 * Loads the <UML:UniqueConstraint> XMI element.
 */
bool UMLUniqueConstraint::load1(QDomElement & element)
{
    int isPrimary = element.attribute(QLatin1String("isPrimary"), QLatin1String("0")).toInt();
    UMLEntity* parentEnt = umlParent()->asUMLEntity();

    if (isPrimary == 1) {
        parentEnt->setAsPrimaryKey(this);
    }

    QDomNode node = element.firstChild();
    while (!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QLatin1String("EntityAttribute"))) {

            QString attName = tempElement.attribute(QLatin1String("name"));
            UMLObject* obj = parentEnt->findChildObject(attName);

            UMLEntityAttribute* entAtt = obj->asUMLEntityAttribute();
            if (entAtt == 0)
                continue;

            m_EntityAttributeList.append(entAtt);

        } else {
            uWarning() << "unknown child type in UMLUniqueConstraint::load";
        }

        node = node.nextSibling();
    }

    return true;
}


/**
 * Check if a entity attribute is present in m_entityAttributeList
 *
 * @param attr The Entity Attribute to check for existence in list
 * @return true if it exists in the list, else false
 */
bool UMLUniqueConstraint::hasEntityAttribute(UMLEntityAttribute* attr) const
{
    if (m_EntityAttributeList.indexOf(attr) == -1) {
        //not present
        return false;
    }

    // else present
    return true;
}

/**
 * Adds a UMLEntityAttribute to the list.
 * The UMLEntityAttribute should already exist and should
 * belong to the parent UMLEntity.
 *
 * @param attr The UMLEntityAttribute to add
 * @return false if it failed to add, else true
 */
bool UMLUniqueConstraint::addEntityAttribute(UMLEntityAttribute* attr)
{
    const UMLEntity *owningParent = umlParent()->asUMLEntity();

    if (hasEntityAttribute(attr)) {
        uDebug() << "Unique Constraint already contains" << attr->name();
        return false;

    }
    if (owningParent == 0) {
        uError() << name() << ": parent is not a UMLEntity";
        return false;
    }

    if (owningParent->findChildObjectById(attr->id()) == 0) {
        uError()
            << " parent " << owningParent->name()
            << " does not contain attribute " << attr->name();
        return false;
    }

    //else add the attribute to the Entity Attribute List
    m_EntityAttributeList.append(attr);

    return true;
}

/**
 * Removes a UMLEntityAttribute from the list
 *
 * @param attr The UMLEntityAttribute to remove from list
 * @return false if it failed to remove the attribute from the list
 */
bool UMLUniqueConstraint::removeEntityAttribute(UMLEntityAttribute* attr)
{
    const UMLEntity *owningParent = umlParent()->asUMLEntity();

    if (owningParent == 0) {
        uError() << name() << ": parent is not a UMLEntity";
        return false;
    }

    /*
     * The attribute may already be removed from the Entity when this function
     * is called. So checking this is not right
     *
     * if (owningParent->findChildObjectById(attr->ID()) == 0) {
     *    uError()
     *        << " parent " << owningParent->getName()
     *        << " does not contain attribute " << attr->getName();
     *    return false;
     * }
     */

    //else remove the attribute from the Entity Attribute List
    if (m_EntityAttributeList.removeAll(attr)) {
        return true;
    }

    return false;
}

/**
 * Get the Entity Attributes List.
 */
UMLEntityAttributeList UMLUniqueConstraint::getEntityAttributeList() const
{
    return m_EntityAttributeList;
}

void UMLUniqueConstraint::init()
{
    m_BaseType = UMLObject::ot_UniqueConstraint;
}

/**
 * Clear the list of attributes contained in this UniqueConstraint
 */
void UMLUniqueConstraint::clearAttributeList()
{
    m_EntityAttributeList.clear();
}

