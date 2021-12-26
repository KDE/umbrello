/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2003-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "association.h"

// app includes
#include "debug_utils.h"
#include "classifier.h"
#include "classpropertiesdialog.h"
#include "folder.h"
#include "uml.h"
#include "umldoc.h"
#include "umlrole.h"
#include "uniqueid.h"
#include "model_utils.h"
#include "cmds.h"

// kde includes
#include <KLocalizedString>

// qt includes
#include <QPointer>
#include <QRegExp>

using namespace Uml;

DEBUG_REGISTER(UMLAssociation)

/**
 * Sets up an association.
 * A new unique ID is assigned internally.
 * @param type    The AssociationType::Enum to construct.
 * @param roleA   Pointer to the UMLObject in role A.
 * @param roleB   Pointer to the UMLObject in role B.
 */
UMLAssociation::UMLAssociation(Uml::AssociationType::Enum type,
                                UMLObject * roleA, UMLObject * roleB)
  : UMLObject(QString())
{
    init(type, roleA, roleB);

    m_pRole[RoleType::A]->setID(UniqueID::gen());
    m_pRole[RoleType::B]->setID(UniqueID::gen());
}

/**
 * Constructs an association - for loading only.
 * This constructor should not normally be used as it constructs
 * an incomplete association (i.e. the role objects are missing.)
 * @param type   The AssociationType::Enum to construct.
 *               Default: Unknown.
 */
UMLAssociation::UMLAssociation(Uml::AssociationType::Enum type)
  : UMLObject(QString(), Uml::ID::Reserved)
{
    init(type, 0, 0);
}

/**
 * Standard destructor.
 */
UMLAssociation::~UMLAssociation()
{
    if (m_pRole[RoleType::A] == 0) {
        uError() << "UMLAssociation destructor: m_pRole[A] is NULL already";
    } else {
        delete m_pRole[RoleType::A];
        m_pRole[RoleType::A] = 0;
    }
    if (m_pRole[RoleType::B] == 0) {
        uError() << "UMLAssociation destructor: m_pRole[B] is NULL already";
    } else {
        delete m_pRole[RoleType::B];
        m_pRole[RoleType::B] = 0;
    }
}

/**
 * Overloaded '==' operator
 */
bool UMLAssociation::operator==(const UMLAssociation &rhs) const
{
    if (this == &rhs) {
        return true;
    }
    return (UMLObject::operator== (rhs) &&
             m_AssocType == rhs.m_AssocType &&
             m_Name == rhs.m_Name &&
             m_pRole[RoleType::A] == rhs.m_pRole[RoleType::A] &&
             m_pRole[RoleType::B] == rhs.m_pRole[RoleType::B]);
}

/**
 * Returns the AssociationType::Enum of the UMLAssociation.
 * @return  The AssociationType::Enum of the UMLAssociation.
 */
Uml::AssociationType::Enum UMLAssociation::getAssocType() const
{
    return m_AssocType;
}

/**
 * Returns a String representation of this UMLAssociation.
 */
QString UMLAssociation::toString() const
{
    QString string = m_pRole[RoleType::A]->toString();
    string += QLatin1Char(' ') + Uml::AssociationType::toStringI18n(m_AssocType) + QLatin1Char(' ');
    string += m_pRole[RoleType::B]->toString();
    return string;
}

/**
 * Resolve types. Required when dealing with foreign XMI files.
 * Needs to be called after all UML objects are loaded from file.
 * Overrides the method from UMLObject.
 * Calls resolveRef() for each role.
 * @return  True for success.
 */
bool UMLAssociation::resolveRef()
{
    bool successA = getUMLRole(RoleType::A)->resolveRef();
    bool successB = getUMLRole(RoleType::B)->resolveRef();
    if (successA && successB) {
        UMLObject *objA = getUMLRole(RoleType::A)->object();
        UMLObject *objB = getUMLRole(RoleType::B)->object();
        // Check if need to change the assoc type to Realization
        if (isRealization(objA, objB)) {
            m_AssocType = Uml::AssociationType::Realization;
        }
        umlPackage()->addAssocToConcepts(this);
        return true;
    }
    return false;
}

/**
 * Creates the <UML:Generalization> or <UML:Association> XMI element
 * including its role objects.
 */
void UMLAssociation::saveToXMI1(QXmlStreamWriter& writer)
{
    if (m_AssocType == Uml::AssociationType::Generalization) {
        UMLObject::save1(QLatin1String("UML:Generalization"), writer);
        writer.writeAttribute(QLatin1String("child"), Uml::ID::toString(getObjectId(RoleType::A)));
        writer.writeAttribute(QLatin1String("parent"), Uml::ID::toString(getObjectId(RoleType::B)));
        writer.writeEndElement();
        return;
    }
    if (m_AssocType == Uml::AssociationType::Realization) {
        UMLObject::save1(QLatin1String("UML:Abstraction"), writer);
        writer.writeAttribute(QLatin1String("client"), Uml::ID::toString(getObjectId(RoleType::A)));
        writer.writeAttribute(QLatin1String("supplier"), Uml::ID::toString(getObjectId(RoleType::B)));
        writer.writeEndElement();
        return;
    }
    if (m_AssocType == Uml::AssociationType::Dependency) {
        UMLObject::save1(QLatin1String("UML:Dependency"), writer);
        writer.writeAttribute(QLatin1String("client"), Uml::ID::toString(getObjectId(RoleType::A)));
        writer.writeAttribute(QLatin1String("supplier"), Uml::ID::toString(getObjectId(RoleType::B)));
        writer.writeEndElement();
        return;
    }
    if (m_AssocType == Uml::AssociationType::Child2Category) {
        UMLObject::save1(QLatin1String("UML:Child2Category"), writer);
        writer.writeAttribute(QLatin1String("client"), Uml::ID::toString(getObjectId(RoleType::A)));
        writer.writeAttribute(QLatin1String("supplier"), Uml::ID::toString(getObjectId(RoleType::B)));
        writer.writeEndElement();
        return;
    }
    if (m_AssocType == Uml::AssociationType::Category2Parent) {
        UMLObject::save1(QLatin1String("UML:Category2Parent"), writer);
        writer.writeAttribute(QLatin1String("client"), Uml::ID::toString(getObjectId(RoleType::A)));
        writer.writeAttribute(QLatin1String("supplier"), Uml::ID::toString(getObjectId(RoleType::B)));
        writer.writeEndElement();
        return;
    }

    UMLObject::save1(QLatin1String("UML:Association"), writer);
    writer.writeStartElement(QLatin1String("UML:Association.connection"));
    getUMLRole(RoleType::A)->saveToXMI1 (writer);
    getUMLRole(RoleType::B)->saveToXMI1 (writer);
    writer.writeEndElement();            // UML:Association.connection
    writer.writeEndElement();  // UML:Association
}

bool UMLAssociation::showPropertiesDialog(QWidget *parent)
{
    QPointer<ClassPropertiesDialog> dlg = new ClassPropertiesDialog(parent, this, true);
    bool modified = dlg->exec();
    delete dlg;
    return modified;
}

/**
 * Loads the <UML:Generalization> or <UML:Association> XMI element
 * including its role objects.
 */
bool UMLAssociation::load1(QDomElement & element)
{
    if (id() == Uml::ID::None)
        return false; // old style XMI file. No real info in this association.

    UMLDoc * doc = UMLApp::app()->document();
    UMLObject * obj[2] = { 0, NULL };
    if (m_AssocType == Uml::AssociationType::Generalization ||
        m_AssocType == Uml::AssociationType::Realization    ||
        m_AssocType == Uml::AssociationType::Dependency     ||
        m_AssocType == Uml::AssociationType::Child2Category ||
        m_AssocType == Uml::AssociationType::Category2Parent) {
        QString general = element.attribute(QLatin1String("general"));
        if (!general.isEmpty()) {
            UMLClassifier *owningClassifier = umlParent()->asUMLClassifier();
            if (owningClassifier == 0){
                uWarning() << "Cannot load UML2 generalization: m_pUMLPackage is expected "
                           << "to be the owning classifier (=client)";
                return false;
            }
            m_pRole[RoleType::A]->setObject(owningClassifier);
            m_pRole[RoleType::B]->setSecondaryId(general);     // defer resolution to resolveRef()
            owningClassifier->addAssociationEnd(this);
            setUMLPackage(umlPackage()->umlPackage());       // reparent
            umlPackage()->addObject(this);
            return true;
        }
        for (unsigned r = RoleType::A; r <= RoleType::B; ++r) {
            const QString fetch = (m_AssocType == Uml::AssociationType::Generalization ?
                                   r == RoleType::A ? QLatin1String("child") : QLatin1String("parent")
                       : r == RoleType::A ? QLatin1String("client") : QLatin1String("supplier"));
            QString roleIdStr = element.attribute(fetch);
            if (roleIdStr.isEmpty()) {
                // Might be given as a child node instead - see below.
                continue;
            }

            // set umlobject of role if possible (else defer resolution)
            obj[r] = doc->findObjectById(Uml::ID::fromString(roleIdStr));
            Uml::RoleType::Enum role = Uml::RoleType::fromInt(r);
            if (obj[r] == 0) {
                m_pRole[role]->setSecondaryId(roleIdStr);  // defer to resolveRef()
            } else {
                m_pRole[role]->setObject(obj[r]);
                if (umlPackage() == 0) {
                    Uml::ModelType::Enum mt = Model_Utils::convert_OT_MT(obj[r]->baseType());
                    setUMLPackage(doc->rootFolder(mt));
                    DEBUG(DBG_SRC) << "assoctype " << m_AssocType
                        << ": setting model type " << Uml::ModelType::toString(mt);
                }
            }
        }
        if (obj[RoleType::A] == 0 || obj[RoleType::B] == 0) {
            for (QDomNode node = element.firstChild(); !node.isNull();
                    node = node.nextSibling()) {
                if (node.isComment())
                    continue;
                QDomElement tempElement = node.toElement();
                QString tag = tempElement.tagName();
                if (Model_Utils::isCommonXMI1Attribute(tag))
                    continue;
                // Permitted tag names:
                //  roleA: "child" "subtype" "client"
                //  roleB: "parent" "supertype" "supplier"
                QString idStr = Model_Utils::getXmiId(tempElement);
                if (idStr.isEmpty())
                    idStr = tempElement.attribute(QLatin1String("xmi.idref"));
                if (idStr.isEmpty()) {
                    QDomNode inner = node.firstChild();
                    QDomElement tmpElem = inner.toElement();
                    idStr = Model_Utils::getXmiId(tmpElem);
                    if (idStr.isEmpty())
                        idStr = tmpElem.attribute(QLatin1String("xmi.idref"));
                }
                if (idStr.isEmpty()) {
                    uError() << "type " << m_AssocType
                        << ", id " << Uml::ID::toString(id()) << ": "
                        << "xmi id not given for " << tag;
                    continue;
                }
                // Since we know for sure that we're dealing with a non
                // umbrello file, use deferred resolution unconditionally.
                if (UMLDoc::tagEq(tag, QLatin1String("child")) ||
                        UMLDoc::tagEq(tag, QLatin1String("subtype")) ||
                        UMLDoc::tagEq(tag, QLatin1String("client"))) {
                    getUMLRole(RoleType::A)->setSecondaryId(idStr);
                } else {
                    getUMLRole(RoleType::B)->setSecondaryId(idStr);
                }
            }
        }

        // it is a realization if either endpoint is an interface
        if (isRealization(obj[RoleType::A], obj[RoleType::B])) {
            m_AssocType = Uml::AssociationType::Realization;
        }
        return true;
    }

    for (QDomNode node = element.firstChild(); !node.isNull();
            node = node.nextSibling()) {
        // uml13.dtd compliant format (new style)
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (Model_Utils::isCommonXMI1Attribute(tag))
            continue;
        QDomNode nodeA = node;
        if (UMLDoc::tagEq(tag, QLatin1String("Association.connection")) ||
                UMLDoc::tagEq(tag, QLatin1String("Association.end")) ||  // Embarcadero's Describe
                UMLDoc::tagEq(tag, QLatin1String("Namespace.ownedElement")) ||
                UMLDoc::tagEq(tag, QLatin1String("Namespace.contents"))) {
            nodeA = tempElement.firstChild();
        }
        // Load role A.
        while (nodeA.isComment())
            nodeA = nodeA.nextSibling();
        tempElement = nodeA.toElement();
        if (tempElement.isNull()) {
            uWarning() << "UML:Association : element (A) is Null";
            return false;
        }
        tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QLatin1String("NavigableEnd"))) {  // Embarcadero's Describe
            m_AssocType = Uml::AssociationType::UniAssociation;
        } else if (!UMLDoc::tagEq(tag, QLatin1String("ownedEnd")) &&
                   !UMLDoc::tagEq(tag, QLatin1String("AssociationEnd")) &&
                   !UMLDoc::tagEq(tag, QLatin1String("AssociationEndRole"))) {
            uWarning() << "unknown child (A) tag " << tag;
            return false;
        }
        if (! getUMLRole(RoleType::A)->loadFromXMI1(tempElement))
            return false;
        // Load role B.
        QDomNode nodeB = nodeA.nextSibling();
        while (nodeB.isComment())
            nodeB = nodeB.nextSibling();
        tempElement = nodeB.toElement();
        if (tempElement.isNull()) {
            uWarning() << "UML:Association : element (B) is Null";
            return false;
        }
        tag = tempElement.tagName();
        if (UMLDoc::tagEq(tag, QLatin1String("NavigableEnd"))) {  // Embarcadero's Describe
            m_AssocType = Uml::AssociationType::UniAssociation;
        } else if (!UMLDoc::tagEq(tag, QLatin1String("ownedEnd")) &&
                   !UMLDoc::tagEq(tag, QLatin1String("AssociationEnd")) &&
                   !UMLDoc::tagEq(tag, QLatin1String("AssociationEndRole"))) {
            uWarning() << "unknown child (B) tag " << tag;
            return false;
        }
        if (! getUMLRole(RoleType::B)->loadFromXMI1(tempElement))
            return false;

        if (umlPackage() == 0) {
            Uml::ModelType::Enum mt = Model_Utils::convert_OT_MT(getObject(RoleType::B)->baseType());
            setUMLPackage(doc->rootFolder(mt));
            DEBUG(DBG_SRC) << "setting model type " << Uml::ModelType::toString(mt);
        }

        // setting the association type:
        //
        // In the old days, we could just record this on the association,
        // and be done with it. But that's not how the UML13.dtd does things.
        // As a result, we are checking roleA for information about the
        // parent association (!) which by this point in the parse, should
        // be set. However, the information that the roles are allowed to have
        // is not complete, so we need to finish the analysis here.

        // find self-associations
        if (m_AssocType == Uml::AssociationType::Association && getObjectId(RoleType::A) == getObjectId(RoleType::B))
            m_AssocType = Uml::AssociationType::Association_Self;

        // fall-back default type
        if (m_AssocType == Uml::AssociationType::Unknown) {
            m_AssocType = Uml::AssociationType::Association;
        }

        return true;
    }

    // From here on it's old-style stuff.
    QString assocTypeStr = element.attribute(QLatin1String("assoctype"), QLatin1String("-1"));
    Uml::AssociationType::Enum assocType = Uml::AssociationType::Unknown;
    if (assocTypeStr[0] >= QLatin1Char('a') && assocTypeStr[0] <= QLatin1Char('z')) {
        // In an earlier version, the natural assoctype names were saved.
        const char *assocTypeString[] = {
                    "generalization",   // Uml::AssociationType::Generalization
                    "aggregation",      // Uml::AssociationType::Aggregation
                    "dependency",       // Uml::AssociationType::Dependency
                    "association",      // Uml::AssociationType::Association
                    "associationself",  // Uml::AssociationType::Association_Self
                    "collmessage",      // Uml::AssociationType::Coll_Message
                    "seqmessage",       // Uml::AssociationType::Seq_Message
                    "collmessageself",  // Uml::AssociationType::Coll_Mesg_Self
                    "seqmessageself",   // Uml::AssociationType::Seq_Message_Self
                    "implementation",   // Uml::AssociationType::Implementation
                    "composition",      // Uml::AssociationType::Composition
                    "realization",      // Uml::AssociationType::Realization
                    "uniassociation",   // Uml::AssociationType::UniAssociation
                    "anchor",           // Uml::AssociationType::Anchor
                    "state",            // Uml::AssociationType::State
                    "activity",         // Uml::AssociationType::Activity
                    "exception",        // Uml::AssociationType::Exception
                    "category2parent"   // Uml::AssociationType::Category2Parent
                    "child2category"    // Uml::AssociationType::Child2Category
                    "relationship"      // Uml::AssociationType::Relationship
        };
        const int arraySize = sizeof(assocTypeString) / sizeof(char*);
        DEBUG(DBG_SRC) << "AssociationType string array size = " << arraySize;

        int index;
        for (index = 0; index < arraySize; ++index)
            if (assocTypeStr == QString::fromLatin1(assocTypeString[index]))
                break;
        if (index < arraySize)
            assocType = Uml::AssociationType::fromInt(index);
    } else {
        int assocTypeNum = assocTypeStr.toInt();
        if (assocTypeNum < (int)Uml::AssociationType::Generalization ||   // first enum
            assocTypeNum >= (int)Uml::AssociationType::Reserved) {     // last enum
            uWarning() << "bad assoctype of UML:AssociationType::Enum " << Uml::ID::toString(id());
            return false;
        }
        assocType = Uml::AssociationType::fromInt(assocTypeNum);
    }
    setAssociationType(assocType);

    Uml::ID::Type roleAObjID = Uml::ID::fromString(element.attribute(QLatin1String("rolea"), QLatin1String("-1")));
    Uml::ID::Type roleBObjID = Uml::ID::fromString(element.attribute(QLatin1String("roleb"), QLatin1String("-1")));
    if (assocType == Uml::AssociationType::Aggregation ||
        assocType == Uml::AssociationType::Composition) {
        // Flip roles to compensate for changed diamond logic in AssociationLine.
        // For further explanations see AssociationWidget::loadFromXMI.
        Uml::ID::Type tmp = roleAObjID;
        roleAObjID = roleBObjID;
        roleBObjID = tmp;
    }

    UMLObject * objA = doc->findObjectById(roleAObjID);
    UMLObject * objB = doc->findObjectById(roleBObjID);

    if(objA)
        getUMLRole(RoleType::A)->setObject(objA);
    else
        return false;

    if(objB)
        getUMLRole(RoleType::B)->setObject(objB);
    else
        return false;

    setMultiplicity(element.attribute(QLatin1String("multia")), RoleType::A);
    setMultiplicity(element.attribute(QLatin1String("multib")), RoleType::B);

    setRoleName(element.attribute(QLatin1String("namea")), RoleType::A);
    setRoleName(element.attribute(QLatin1String("nameb")), RoleType::B);

    setRoleDoc(element.attribute(QLatin1String("doca")), RoleType::A);
    setRoleDoc(element.attribute(QLatin1String("docb")), RoleType::B);

    // Visibility defaults to Public if it cant set it here..
    QString visibilityA = element.attribute(QLatin1String("visibilitya"), QLatin1String("0"));
    QString visibilityB = element.attribute(QLatin1String("visibilityb"), QLatin1String("0"));
    int vis = visibilityA.toInt();
    if (vis >= 200)  // bkwd compat.
        vis -= 200;
    setVisibility((Uml::Visibility::Enum)vis, RoleType::A);
    vis = visibilityB.toInt();
    if (vis >= 200)  // bkwd compat.
        vis -= 200;
    setVisibility((Uml::Visibility::Enum)vis, RoleType::B);

    // Changeability defaults to Changeable if it cant set it here..
    QString changeabilityA = element.attribute(QLatin1String("changeabilitya"), QLatin1String("0"));
    QString changeabilityB = element.attribute(QLatin1String("changeabilityb"), QLatin1String("0"));
    if (changeabilityA.toInt() > 0)
        setChangeability(Uml::Changeability::fromInt(changeabilityA.toInt()), RoleType::A);
    if (changeabilityB.toInt() > 0)
        setChangeability(Uml::Changeability::fromInt(changeabilityB.toInt()), RoleType::B);

    return true;
}

/**
 * Returns the UMLObject assigned to the given role.
 * @return  Pointer to the UMLObject in the given role.
 */
UMLObject* UMLAssociation::getObject(Uml::RoleType::Enum role) const
{
    if (m_pRole[role] == 0)
        return 0;
    return m_pRole[role]->object();
}

/**
 * Returns the ID of the UMLObject assigned to the given role.
 * Shorthand for getObject(role)->ID().
 * @return  ID of the UMLObject in the given role.
 */
Uml::ID::Type UMLAssociation::getObjectId(Uml::RoleType::Enum role) const
{
    UMLRole *roleObj = m_pRole[role];
    if (roleObj == 0)
        return Uml::ID::None;
    UMLObject *o = roleObj->object();
    if (o == 0) {
        QString auxID = roleObj->secondaryId();
        if (auxID.isEmpty()) {
            uError() << "role " << role << ": getObject returns NULL";
            return Uml::ID::None;
        } else {
            DEBUG(DBG_SRC) << "role " << role << ": using secondary ID " << auxID;
            return Uml::ID::fromString(auxID);
        }
    }
    return o->id();
}

/**
 * Returns the ID of the UMLObject assigned to the given role.
 * CURRENTLY UNUSED.
 * @return  ID of the UMLObject of the given role.
 */
Uml::ID::Type UMLAssociation::getRoleId(RoleType::Enum role) const
{
    return m_pRole[role]->id();
}

/**
 * Returns the changeability.
 */
Uml::Changeability::Enum UMLAssociation::changeability(Uml::RoleType::Enum role) const
{
    return m_pRole[role]->changeability();
}

/**
 * Returns the Visibility of the given role.
 * @return  Visibility of the given role.
 */
Uml::Visibility::Enum UMLAssociation::visibility(Uml::RoleType::Enum role) const
{
    return m_pRole[role]->visibility();
}

/**
 * Returns the multiplicity assigned to the given role.
 * @return  The multiplicity assigned to the given role.
 */
QString UMLAssociation::getMultiplicity(Uml::RoleType::Enum role) const
{
    return m_pRole[role]->multiplicity();
}

/**
 * Returns the name assigned to the role A.
 * @return  The name assigned to the given role.
 */
QString UMLAssociation::getRoleName(Uml::RoleType::Enum role) const
{
    return m_pRole[role]->name();
}

/**
 * Returns the documentation assigned to the given role.
 * @return  Documentation text of given role.
 */
QString UMLAssociation::getRoleDoc(Uml::RoleType::Enum role) const
{
    return m_pRole[role]->doc();
}

/**
 * Get the underlying UMLRole object for the given role.
 * @return  Pointer to the UMLRole object for the given role.
 */
UMLRole * UMLAssociation::getUMLRole(Uml::RoleType::Enum role) const
{
    return m_pRole[role];
}

/**
 * Set the attribute m_bOldLoadMode.
 * @param value   the new value to set
 */
void UMLAssociation::setOldLoadMode(bool value /* = true */)
{
    m_bOldLoadMode = value;
}

/**
 * Return the backward compatibility flag for loading files.
 */
bool UMLAssociation::getOldLoadMode() const
{
    return m_bOldLoadMode;
}

/**
 * Sets the assocType of the UMLAssociation.
 * @param assocType The AssociationType::Enum of the UMLAssociation.
 */
void UMLAssociation::setAssociationType(Uml::AssociationType::Enum assocType)
{
    m_AssocType = assocType;
    if (m_AssocType == Uml::AssociationType::UniAssociation)
    {
        // In this case we need to auto-set the multiplicity/rolenames
        // of the roles
#ifdef VERBOSE_DEBUGGING
        DEBUG(DBG_SRC) << " A new uni-association has been created.";
#endif
    }
    UMLObject::emitModified();
}

/**
 * Sets the UMLObject playing the given role in the association.
 * @param obj  Pointer to the UMLObject of the given role.
 * @param role The Uml::RoleType::Enum played by the association
 */
void UMLAssociation::setObject(UMLObject *obj, Uml::RoleType::Enum role)
{
    m_pRole[role]->setObject(obj);
}

/**
 * Sets the visibility of the given role of the UMLAssociation.
 * @param value  Visibility of role.
 * @param role   The Uml::RoleType::Enum to which the visibility is being applied
 */
void UMLAssociation::setVisibility(Visibility::Enum value, Uml::RoleType::Enum role)
{
    m_pRole[role]->setVisibility(value);
}

/**
 * Sets the changeability of the given role of the UMLAssociation.
 * @param value     Changeability_Type of the given role.
 * @param role      The Uml::RoleType::Enum to which the changeability is being set
 */
void UMLAssociation::setChangeability(Uml::Changeability::Enum value, Uml::RoleType::Enum role)
{
    m_pRole[role]->setChangeability(value);
}

/**
 * Sets the multiplicity of the given role of the UMLAssociation.
 * @param multi    The multiplicity of the given role.
 * @param role     The Uml::RoleType::Enum to which the multiplicity is being applied
 */
void UMLAssociation::setMultiplicity(const QString &multi, Uml::RoleType::Enum role)
{
    if (m_pRole[role]->multiplicity() != multi) {
        UMLApp::app()->executeCommand(new CmdChangeMultiplicity(m_pRole[role], multi));
    }
}

/**
 * Sets the name of the given role of the UMLAssociation.
 * @param roleName  The name to set for the given role.
 * @param role      The Uml::RoleType::Enum for which to set the name.
 */
void UMLAssociation::setRoleName(const QString &roleName, Uml::RoleType::Enum role)
{
    m_pRole[role]->setName(roleName);
}

/**
 * Sets the documentation on the given role in the association.
 * @param doc      The string with the documentation.
 * @param role     The Uml::RoleType::Enum to which the documentation is being applied
 */
void UMLAssociation::setRoleDoc(const QString &doc, Uml::RoleType::Enum role)
{
    m_pRole[role]->setDoc(doc);
}

/**
 * When the association type is "Generalization" and at least one of the
 * given objects an interface, then it is a "Realization".
 * @param objA   UML object as role A
 * @param objB   UML object as role B
 * @return flag whether association is a realization
 */
bool UMLAssociation::isRealization(UMLObject* objA, UMLObject* objB) const
{
    bool aIsInterface = false;
    if (objA && (objA->baseType() == UMLObject::ot_Interface)) {
        aIsInterface = true;
    }
    bool bIsInterface = false;
    if (objB && (objB->baseType() == UMLObject::ot_Interface)) {
        bIsInterface = true;
    }
    return (m_AssocType == Uml::AssociationType::Generalization) &&
           (aIsInterface || bIsInterface);
}

/**
 * Common initializations at construction time.
 * @param type      The AssociationType::Enum to represent.
 * @param roleAObj  Pointer to the role A UMLObject.
 * @param roleBObj  Pointer to the role B UMLObject.
 */
void UMLAssociation::init(Uml::AssociationType::Enum type, UMLObject *roleAObj, UMLObject *roleBObj)
{
    m_AssocType = type;
    m_BaseType = ot_Association;
    m_Name = QString();
    m_bOldLoadMode = false;
    nrof_parent_widgets = -1;
    if (!UMLApp::app()->document()->loading()) {
        setUMLPackage(UMLApp::app()->document()->currentRoot());
    }
    m_pRole[RoleType::A] = new UMLRole (this, roleAObj, RoleType::A);
    m_pRole[RoleType::B] = new UMLRole (this, roleBObj, RoleType::B);
}

