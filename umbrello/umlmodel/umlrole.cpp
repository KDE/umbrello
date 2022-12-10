/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "umlrole.h"

// local includes
#include "association.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "optionstate.h"
#include "umldoc.h"
#include "umlroledialog.h"
#include "uml.h"

// qt includes
#include <QPointer>
#include <QRegExp>

DEBUG_REGISTER(UMLRole)

/**
 * Sets up an association.
 *
 * @param parent     The parent (association) of this UMLRole.
 * @param parentObj  The Parent UML Object of this UMLRole
 * @param role       The Uml::RoleType::Enum of this UMLRole
 */
UMLRole::UMLRole(UMLAssociation * parent, UMLObject * parentObj, Uml::RoleType::Enum role)
  : UMLObject(const_cast<UMLAssociation*>(parent)),
    m_pAssoc(parent),
    m_role(role),
    m_Multi(QString()),
    m_Changeability(Uml::Changeability::Changeable)
{
    m_BaseType = UMLObject::ot_Role;
    m_name.clear();
    m_pSecondary = parentObj;

    // connect this up to parent
    connect(this, SIGNAL(modified()), parent, SIGNAL(modified()));
}

/**
 * Standard destructor.
 */
UMLRole::~UMLRole()
{
}

/**
 * Overloaded '==' operator.
 */
bool UMLRole::operator==(const UMLRole &rhs) const
{
    if (this == &rhs) {
        return true;
    }
    return (UMLObject::operator==(rhs) &&
             m_Changeability == rhs.m_Changeability &&
             m_Multi == rhs.m_Multi &&
             m_name == rhs.m_name
          );
}

/**
 * Returns a String representation of this UMLRole instance.
 */
QString UMLRole::toString() const
{
    QString result;
    if (object()) {
        result = object()->name();
        result += QLatin1Char(':');
        result += name();
    } else
        result = QStringLiteral("null");
    return result;
}

UMLAssociation * UMLRole::parentAssociation() const
{
    return m_pAssoc;
}

/**
 * Returns the UMLObject assigned to the role.
 * @return  Pointer to the UMLObject in role.
 */
UMLObject* UMLRole::object() const
{
    return m_pSecondary;
}

/**
 * Returns the Changeablity of the role.
 *
 * @return  Changeability of role.
 */
Uml::Changeability::Enum UMLRole::changeability() const
{
    return m_Changeability;
}

/**
 * Returns the multiplicity assigned to the role.
 *
 * @return  The multiplicity assigned to the role.
 */
QString UMLRole::multiplicity() const
{
    return m_Multi;
}

/**
 * Sets the UMLObject playing the role in the association.
 *
 * @param obj   Pointer to the UMLObject of role.
 */
void UMLRole::setObject(UMLObject *obj)
{
    // because we will get the id of this role from the parent
    // object, we CANT allow UMLRoles to take other UMLRoles as
    // parent objects. In fact, there is probably good reason
    // to only take UMLClassifiers here, but I'll leave it more open
    // for the time being. -b.t.
    if (obj && obj->asUMLRole()) {
        logError2("UMLRole(%1) cannot setObject() to another UMLRole(%2)",
                  Uml::ID::toString(m_nId), Uml::ID::toString(obj->id()));
        return;
    }

    m_pSecondary = obj;
    UMLObject::emitModified();
}

/**
 * Sets the changeability of the role.
 *
 * @param value   Changeability::Enum of role.
 */
void UMLRole::setChangeability(Uml::Changeability::Enum value)
{
    m_Changeability = value;
    UMLObject::emitModified();
}

/**
 * Sets the multiplicity of the role.
 *
 * @param multi   The multiplicity of role.
 */
void UMLRole::setMultiplicity(const QString &multi)
{
    m_Multi = multi;
    UMLObject::emitModified();
}

/**
 * Get the 'id' of the role (NOT the parent object). This could be
 * either Uml::RoleType::A or Uml::RoleType::B. Yes, it would be better if we
 * could get along without this, but we need it to distinguish saved
 * umlrole objects in the XMI for 'self' associations where both roles
 * will point to the same underlying UMLObject.
 */
Uml::RoleType::Enum UMLRole::role() const
{
    return m_role;
}

/**
 * Creates the <UML:AssociationEnd> XMI element.
 */
void UMLRole::saveToXMI(QXmlStreamWriter& writer)
{
    UMLObject::save1(writer, QStringLiteral("AssociationEnd"), QStringLiteral("ownedEnd"));
    if (m_pSecondary)
        writer.writeAttribute(QStringLiteral("type"), Uml::ID::toString(m_pSecondary->id()));
    else
        logError1("UMLRole::saveToXMI(id %1) : m_pSecondary is null", Uml::ID::toString(m_nId));
    if (!m_Multi.isEmpty())
        writer.writeAttribute(QStringLiteral("multiplicity"), m_Multi);
    if (m_role == Uml::RoleType::A) {  // role aggregation based on parent type
        // role A
        switch (m_pAssoc->getAssocType()) {
        case Uml::AssociationType::Composition:
            writer.writeAttribute(QStringLiteral("aggregation"), QStringLiteral("composite"));
            break;
        case Uml::AssociationType::Aggregation:
            writer.writeAttribute(QStringLiteral("aggregation"), QStringLiteral("aggregate"));
            break;
        default:
            writer.writeAttribute(QStringLiteral("aggregation"), QStringLiteral("none"));
            break;
        }
        if (m_pAssoc->getAssocType() == Uml::AssociationType::UniAssociation) {
            // Normally the isNavigable attribute is "true".
            // We set it to false on role A to indicate that
            // role B gets an explicit arrowhead.
            writer.writeAttribute(QStringLiteral("isNavigable"), QStringLiteral("false"));
        } else {
            writer.writeAttribute(QStringLiteral("isNavigable"), QStringLiteral("true"));
        }
    } else {
        writer.writeAttribute(QStringLiteral("aggregation"), QStringLiteral("none"));
        writer.writeAttribute(QStringLiteral("isNavigable"), QStringLiteral("true"));
        //FIXME obviously this isn't standard XMI
        if (m_pAssoc->getAssocType() == Uml::AssociationType::Relationship) {
            writer.writeAttribute(QStringLiteral("relationship"), QStringLiteral("true"));
        }
    }

    switch (m_Changeability) {
        case Uml::Changeability::Frozen:
            writer.writeAttribute(QStringLiteral("changeability"), QStringLiteral("frozen"));
            break;
        case Uml::Changeability::AddOnly:
            writer.writeAttribute(QStringLiteral("changeability"), QStringLiteral("addOnly"));
            break;
        case Uml::Changeability::Changeable:
            writer.writeAttribute(QStringLiteral("changeability"), QStringLiteral("changeable"));
            break;
    }
    writer.writeEndElement();
}

/**
 * Display the properties configuration dialog for the object.
 *
 * @param parent    The parent widget.
 * @return  True for success of this operation.
 */
bool UMLRole::showPropertiesDialog(QWidget *parent)
{
    QPointer<UMLRoleDialog> dlg = new UMLRoleDialog(parent, this);
    bool modified = dlg->exec() == QDialog::Accepted;
    delete dlg;
    return modified;
}

/**
 * Loads the <UML:AssociationEnd> XMI element.
 * Auxiliary to UMLObject::loadFromXMI.
 */
bool UMLRole::load1(QDomElement & element)
{
    UMLDoc * doc = UMLApp::app()->document();
    QString type = element.attribute(QStringLiteral("type"));
    if (!type.isEmpty()) {
        if (!m_SecondaryId.isEmpty())
            logWarn2("UMLRole::load1 overwriting old m_SecondaryId %1 with new value %2",
                     m_SecondaryId, type);
        m_SecondaryId = type;
    }
    // Inspect child nodes - for multiplicity (and type if not set above.)
    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QStringLiteral("name"))) {
            m_name = tempElement.text();
        } else if (UMLDoc::tagEq(tag, QStringLiteral("AssociationEnd.multiplicity"))) {
            /*
             * There are different ways in which the multiplicity might be given:
             *  - direct value in the <AssociationEnd.multiplicity> tag,
             *  - attributes "lower" and "upper" of a subordinate <MultiplicityRange>,
             *  - direct value in subordinate <MultiplicityRange.lower> and
             *    <MultiplicityRange.upper> tags
             */
            QDomNode n = tempElement.firstChild();
            if (node.isNull() || tempElement.isNull() || n.isNull() ||
                    n.toElement().isNull()) {
                m_Multi = tempElement.text().trimmed();
                continue;
            }
            tempElement = n.toElement();
            tag = tempElement.tagName();
            if (!UMLDoc::tagEq(tag, QStringLiteral("Multiplicity"))) {
                m_Multi = tempElement.text().trimmed();
                continue;
            }
            n = tempElement.firstChild();
            tempElement = n.toElement();
            tag = tempElement.tagName();
            if (!UMLDoc::tagEq(tag, QStringLiteral("Multiplicity.range"))) {
                m_Multi = tempElement.text().trimmed();
                continue;
            }
            n = tempElement.firstChild();
            tempElement = n.toElement();
            tag = tempElement.tagName();
            if (!UMLDoc::tagEq(tag, QStringLiteral("MultiplicityRange"))) {
                m_Multi = tempElement.text().trimmed();
                continue;
            }
            QString multiUpper;
            if (tempElement.hasAttribute(QStringLiteral("lower"))) {
                m_Multi = tempElement.attribute(QStringLiteral("lower"));
                multiUpper = tempElement.attribute(QStringLiteral("upper"));
                if (!multiUpper.isEmpty()) {
                    if (!m_Multi.isEmpty())
                        m_Multi.append(QStringLiteral(".."));
                    m_Multi.append(multiUpper);
                }
                continue;
            }
            n = tempElement.firstChild();
            while (!n.isNull()) {
                tempElement = n.toElement();
                tag = tempElement.tagName();
                if (UMLDoc::tagEq(tag, QStringLiteral("MultiplicityRange.lower"))) {
                    m_Multi = tempElement.text();
                } else if (UMLDoc::tagEq(tag, QStringLiteral("MultiplicityRange.upper"))) {
                    multiUpper = tempElement.text();
                }
                n = n.nextSibling();
            }
            if (!multiUpper.isEmpty()) {
                if (!m_Multi.isEmpty())
                    m_Multi.append(QStringLiteral(".."));
                m_Multi.append(multiUpper);
            }
        } else if (m_SecondaryId.isEmpty() &&
                   (UMLDoc::tagEq(tag, QStringLiteral("type")) ||
                    UMLDoc::tagEq(tag, QStringLiteral("participant")))) {
            m_SecondaryId = Model_Utils::getXmiId(tempElement);
            if (m_SecondaryId.isEmpty())
                m_SecondaryId = tempElement.attribute(QStringLiteral("xmi.idref"));
            if (m_SecondaryId.isEmpty()) {
                QDomNode inner = tempElement.firstChild();
                QDomElement innerElem = inner.toElement();
                m_SecondaryId = Model_Utils::getXmiId(innerElem);
                if (m_SecondaryId.isEmpty())
                    m_SecondaryId = innerElem.attribute(QStringLiteral("xmi.idref"));
            }
        }
    }
    if (!m_Multi.isEmpty())
        logDebug2("UMLRole::load1 %1: m_Multi is %2", name(), m_Multi);
    if (m_SecondaryId.isEmpty()) {
        logError1("UMLRole::load1(%1) : type not given or illegal", name());
        return false;
    }
    UMLObject * obj;
    obj = doc->findObjectById(Uml::ID::fromString(m_SecondaryId));
    if (obj) {
        m_pSecondary = obj;
        m_SecondaryId = QString();
    }

    // block signals to prevent needless updating
    blockSignals(true);
    // Here comes the handling of the association type.
    // This is open for discussion - I'm pretty sure there are better ways..

    // Yeah, for one, setting the *parent* object parameters from here is sucky
    // as hell. Why are we using roleA to store what is essentially a parent (association)
    // parameter, eh? The UML13.dtd is pretty silly, but since that is what
    // is driving us to that point, we have to go with it. Some analysis of
    // the component roles/linked items needs to be done in order to get things
    // right. *sigh* -b.t.

    // Setting association type from the role (A)
    // Determination of the "aggregation" attribute used to be done only
    // when (m_role == Uml::RoleType::A) but some XMI writers (e.g. StarUML) place
    // the aggregation attribute at role B.
    // The role end with the aggregation unequal to "none" wins.
    QString aggregation = element.attribute(QStringLiteral("aggregation"), QStringLiteral("none"));
    if (aggregation == QStringLiteral("composite"))
        m_pAssoc->setAssociationType(Uml::AssociationType::Composition);
    else if (aggregation == QStringLiteral("shared")       // UML1.3
          || aggregation == QStringLiteral("aggregate"))   // UML1.4
        m_pAssoc->setAssociationType(Uml::AssociationType::Aggregation);

    if (!element.hasAttribute(QStringLiteral("isNavigable"))) {
        // Backward compatibility mode: In Umbrello version 1.3.x the
        // logic for saving the isNavigable flag was wrong.
        // May happen on loading role A.
        m_pAssoc->setOldLoadMode(true);
    } else if (m_pAssoc->getOldLoadMode() == true) {
        // Here is the original logic:
        // "Role B:
        //  If isNavigable is not given, we make no change to the
        //  association type.
        //  If isNavigable is given, and is "true", then we assume that
        //  the association's other end (role A) is not navigable, and
        //  therefore we change the association type to UniAssociation.
        //  The case that isNavigable is given as "false" is ignored.
        //  Combined with the association type logic for role A, this
        //  allows us to support at_Association and at_UniAssociation."
        if (element.attribute(QStringLiteral("isNavigable")) == QStringLiteral("true"))
            m_pAssoc->setAssociationType(Uml::AssociationType::UniAssociation);
    } else if (element.attribute(QStringLiteral("isNavigable")) == QStringLiteral("false")) {
        m_pAssoc->setAssociationType(Uml::AssociationType::UniAssociation);
    }

    //FIXME not standard XMI
    if (element.hasAttribute(QStringLiteral("relationship"))) {
        if (element.attribute(QStringLiteral("relationship")) == QStringLiteral("true")) {
            m_pAssoc->setAssociationType(Uml::AssociationType::Relationship);
        }
    }

    if (m_Multi.isEmpty())
        m_Multi = element.attribute(QStringLiteral("multiplicity"));

    // Changeability defaults to Changeable if it cant set it here..
    m_Changeability = Uml::Changeability::Changeable;
    QString changeability = element.attribute(QStringLiteral("changeability"));
    if (changeability.isEmpty())
        element.attribute(QStringLiteral("changeable"));  // for backward compatibility
    if (changeability == QStringLiteral("frozen"))
        m_Changeability = Uml::Changeability::Frozen;
    else if (changeability == QStringLiteral("addOnly"))
        m_Changeability = Uml::Changeability::AddOnly;

    // finished config, now unblock
    blockSignals(false);
    return true;
}

