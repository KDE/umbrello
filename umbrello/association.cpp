/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "association.h"

// app includes
#include "debug_utils.h"
#include "classifier.h"
#include "folder.h"
#include "uml.h"
#include "umldoc.h"
#include "umlrole.h"
#include "uniqueid.h"
#include "model_utils.h"
#include "cmds.h"

// kde includes
#include <klocale.h>

// qt includes
#include <QtCore/QRegExp>

using namespace Uml;

/**
 * Sets up an association.
 * A new unique ID is assigned internally.
 * @param type    The AssociationType to construct.
 * @param roleA   Pointer to the UMLObject in role A.
 * @param roleB   Pointer to the UMLObject in role B.
 */
UMLAssociation::UMLAssociation( Uml::AssociationType type,
                                UMLObject * roleA, UMLObject * roleB )
  : UMLObject("")
{
    init(type, roleA, roleB);

    m_pRole[Uml::A]->setID( UniqueID::gen() );
    m_pRole[Uml::B]->setID( UniqueID::gen() );
}

/**
 * Constructs an association - for loading only.
 * This constructor should not normally be used as it constructs
 * an incomplete association (i.e. the role objects are missing.)
 * @param type   The AssociationType to construct.
 *               Default: Unknown.
 */
UMLAssociation::UMLAssociation( Uml::AssociationType type)
  : UMLObject("", Uml::id_Reserved)
{
    init(type, NULL, NULL);
}

/**
 * Standard destructor.
 */
UMLAssociation::~UMLAssociation( )
{
    if (m_pRole[A] == NULL) {
        uError() << "UMLAssociation destructor: m_pRole[A] is NULL already";
    } else {
        delete m_pRole[A];
        m_pRole[A] = NULL;
    }
    if (m_pRole[B] == NULL) {
        uError() << "UMLAssociation destructor: m_pRole[B] is NULL already";
    } else {
        delete m_pRole[B];
        m_pRole[B] = NULL;
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
    return ( UMLObject::operator== ( rhs ) &&
             m_AssocType == rhs.m_AssocType &&
             m_Name == rhs.m_Name &&
             m_pRole[A] == rhs.m_pRole[A] &&
             m_pRole[B] == rhs.m_pRole[B] );
}

/**
 * Returns the AssociationType of the UMLAssociation.
 * @return  The AssociationType of the UMLAssociation.
 */
Uml::AssociationType UMLAssociation::getAssocType() const
{
    return m_AssocType;
}

/**
 * Returns a String representation of this UMLAssociation.
 */
QString UMLAssociation::toString() const
{
    QString string;
    if(m_pRole[A])
    {
        string += m_pRole[A]->object()->name();
        string += ':';
        string += m_pRole[A]->name();
    }
    string += ':' + m_AssocType.toStringI18n() + ':';
    if(m_pRole[B])
    {
        string += m_pRole[B]->object( )->name();
        string += ':';
        string += m_pRole[B]->name();
    }
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
    bool successA = getUMLRole(A)->resolveRef();
    bool successB = getUMLRole(B)->resolveRef();
    if (successA && successB) {
        UMLObject *objA = getUMLRole(A)->object();
        UMLObject *objB = getUMLRole(B)->object();
        // Check if need to change the assoc type to Realization
        if (isRealization(objA, objB)) {
            m_AssocType = Uml::AssociationType::Realization;
        }
        m_pUMLPackage->addAssocToConcepts(this);
        return true;
    }
    return false;
}

/**
 * Creates the <UML:Generalization> or <UML:Association> XMI element
 * including its role objects.
 */
void UMLAssociation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement )
{
    if (m_AssocType == Uml::AssociationType::Generalization) {
        QDomElement assocElement = UMLObject::save("UML:Generalization", qDoc);
        assocElement.setAttribute( "discriminator", "" );
        assocElement.setAttribute( "child", ID2STR(getObjectId(A)) );
        assocElement.setAttribute( "parent", ID2STR(getObjectId(B)) );
        qElement.appendChild( assocElement );
        return;
    }
    if (m_AssocType == Uml::AssociationType::Realization) {
        QDomElement assocElement = UMLObject::save("UML:Abstraction", qDoc);
        assocElement.setAttribute( "client", ID2STR(getObjectId(A)) );
        assocElement.setAttribute( "supplier", ID2STR(getObjectId(B)) );
        qElement.appendChild( assocElement );
        return;
    }
    if (m_AssocType == Uml::AssociationType::Dependency) {
        QDomElement assocElement = UMLObject::save("UML:Dependency", qDoc);
        assocElement.setAttribute( "client", ID2STR(getObjectId(A)) );
        assocElement.setAttribute( "supplier", ID2STR(getObjectId(B)) );
        qElement.appendChild( assocElement );
        return;
    }
    if (m_AssocType == Uml::AssociationType::Child2Category ) {
        QDomElement assocElement = UMLObject::save("UML:Child2Category", qDoc);
        assocElement.setAttribute( "client", ID2STR(getObjectId(A)) );
        assocElement.setAttribute( "supplier", ID2STR(getObjectId(B)) );
        qElement.appendChild( assocElement );
        return;
    }
    if (m_AssocType == Uml::AssociationType::Category2Parent ) {
        QDomElement assocElement = UMLObject::save("UML:Category2Parent", qDoc);
        assocElement.setAttribute( "client", ID2STR(getObjectId(A)) );
        assocElement.setAttribute( "supplier", ID2STR(getObjectId(B)) );
        qElement.appendChild( assocElement );
        return;
    }

    QDomElement associationElement = UMLObject::save("UML:Association", qDoc);
    QDomElement connElement = qDoc.createElement("UML:Association.connection");
    getUMLRole(A)->saveToXMI (qDoc, connElement);
    getUMLRole(B)->saveToXMI (qDoc, connElement);
    associationElement.appendChild (connElement);
    qElement.appendChild( associationElement );
}

/**
 * Creates the <UML:Generalization> or <UML:Association> XMI element
 * including its role objects.
 */
bool UMLAssociation::load( QDomElement & element )
{
    if (id() == Uml::id_None)
        return false; // old style XMI file. No real info in this association.

    UMLDoc * doc = UMLApp::app()->document();
    UMLObject * obj[2] = { NULL, NULL };
    if (m_AssocType == Uml::AssociationType::Generalization ||
        m_AssocType == Uml::AssociationType::Realization    ||
        m_AssocType == Uml::AssociationType::Dependency     ||
        m_AssocType == Uml::AssociationType::Child2Category ||
        m_AssocType == Uml::AssociationType::Category2Parent
        ) {
        for (unsigned r = Uml::A; r <= Uml::B; ++r) {
            const QString fetch = (m_AssocType == Uml::AssociationType::Generalization ?
                                   r == Uml::A ? "child" : "parent"
                       : r == Uml::A ? "client" : "supplier");
            QString roleIdStr = element.attribute(fetch, "");
            if (roleIdStr.isEmpty()) {
                // Might be given as a child node instead - see below.
                continue;
            }

            // set umlobject of role if possible (else defer resolution)
            obj[r] = doc->findObjectById(STR2ID(roleIdStr));
            Uml::Role_Type role = (Uml::Role_Type)r;
            if (obj[r] == NULL) {
                m_pRole[role]->setSecondaryId(roleIdStr);  // defer to resolveRef()
            } else {
                m_pRole[role]->setObject(obj[r]);
                if (m_pUMLPackage == NULL) {
                    Uml::ModelType mt = Model_Utils::convert_OT_MT(obj[r]->baseType());
                    m_pUMLPackage = doc->rootFolder(mt);
                    uDebug() << "assoctype " << m_AssocType
                        << ": setting model type " << mt;
                }
            }
        }
        if (obj[A] == NULL || obj[B] == NULL) {
            for (QDomNode node = element.firstChild(); !node.isNull();
                    node = node.nextSibling()) {
                if (node.isComment())
                    continue;
                QDomElement tempElement = node.toElement();
                QString tag = tempElement.tagName();
                if (Model_Utils::isCommonXMIAttribute(tag))
                    continue;
                // Permitted tag names:
                //  roleA: "child" "subtype" "client"
                //  roleB: "parent" "supertype" "supplier"
                QString idStr = tempElement.attribute( "xmi.id", "" );
                if (idStr.isEmpty())
                    idStr = tempElement.attribute( "xmi.idref", "" );
                if (idStr.isEmpty()) {
                    QDomNode inner = node.firstChild();
                    QDomElement tmpElem = inner.toElement();
                    idStr = tmpElem.attribute( "xmi.id", "" );
                    if (idStr.isEmpty())
                        idStr = tmpElem.attribute( "xmi.idref", "" );
                }
                if (idStr.isEmpty()) {
                    uError() << "type " << m_AssocType
                        << ", id " << ID2STR(id()) << ": "
                        << "xmi id not given for " << tag;
                    continue;
                }
                // Since we know for sure that we're dealing with a non
                // umbrello file, use deferred resolution unconditionally.
                if (UMLDoc::tagEq(tag, "child") || UMLDoc::tagEq(tag, "subtype") || UMLDoc::tagEq(tag, "client")) {
                    getUMLRole(A)->setSecondaryId(idStr);
                } else {
                    getUMLRole(B)->setSecondaryId(idStr);
                }
            }
        }

        // it is a realization if either endpoint is an interface
        if (isRealization(obj[A], obj[B])) {
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
        if (Model_Utils::isCommonXMIAttribute(tag))
            continue;
        if (!UMLDoc::tagEq(tag, "Association.connection") &&
                !UMLDoc::tagEq(tag, "Namespace.ownedElement") &&
                !UMLDoc::tagEq(tag, "Namespace.contents")) {
            uWarning() << "unknown child node " << tag;
            continue;
        }
        // Load role A.
        node = tempElement.firstChild();
        while (node.isComment())
            node = node.nextSibling();
        tempElement = node.toElement();
        if (tempElement.isNull()) {
            uWarning() << "UML:Association : element (A) is Null";
            return false;
        }
        tag = tempElement.tagName();
        if (!UMLDoc::tagEq(tag, "AssociationEndRole") &&
                !UMLDoc::tagEq(tag, "AssociationEnd")) {
            uWarning() << "unknown child (A) tag " << tag;
            return false;
        }
        if (! getUMLRole(A)->loadFromXMI(tempElement))
            return false;
        // Load role B.
        node = node.nextSibling();
        while (node.isComment())
            node = node.nextSibling();
        tempElement = node.toElement();
        if (tempElement.isNull()) {
            uWarning() << "UML:Association : element (B) is Null";
            return false;
        }
        tag = tempElement.tagName();
        if (!UMLDoc::tagEq(tag, "AssociationEndRole") &&
                !UMLDoc::tagEq(tag, "AssociationEnd")) {
            uWarning() << "unknown child (B) tag " << tag;
            return false;
        }
        if (! getUMLRole(B)->loadFromXMI(tempElement))
            return false;

        if (m_pUMLPackage == NULL) {
            Uml::ModelType mt = Model_Utils::convert_OT_MT(getObject(B)->baseType());
            m_pUMLPackage = doc->rootFolder(mt);
            uDebug() << "setting model type " << mt;
        }

        // setting the association type:
        //
        // In the old days, we could just record this on the association,
        // and be done with it. But thats not how the UML13.dtd does things.
        // As a result, we are checking roleA for information about the
        // parent association (!) which by this point in the parse, should
        // be set. However, the information that the roles are allowed to have
        // is not complete, so we need to finish the analysis here.

        // find self-associations
        if (m_AssocType == Uml::AssociationType::Association && getObjectId(A) == getObjectId(B))
            m_AssocType = Uml::AssociationType::Association_Self;

        // fall-back default type
        if (m_AssocType == Uml::AssociationType::Unknown) {
            m_AssocType = Uml::AssociationType::Association;
        }

        return true;
    }

    // From here on it's old-style stuff.
    QString assocTypeStr = element.attribute( "assoctype", "-1" );
    Uml::AssociationType assocType = Uml::AssociationType::Unknown;
    if (assocTypeStr[0] >= 'a' && assocTypeStr[0] <= 'z') {
        // In an earlier version, the natural assoctype names were saved.
        const QString assocTypeString[] = {
                    "generalization",   // Uml::AssociationType::Generalization
                    "aggregation",      // Uml::AssociationType::Aggregation
                    "dependency",       // Uml::AssociationType::Dependency
                    "association",      // Uml::AssociationType::Association
                    "associationself",  // Uml::AssociationType::Association_Self
                    "collmessage",      // Uml::AssociationType::Coll_Message
                    "seqmessage",       // Uml::AssociationType::Seq_Message
                    "collmessageself",  // Uml::AssociationType::Coll_Message_Self
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
        const int arraySize = sizeof(assocTypeString)/sizeof(QString);
        uDebug() << "AssociationType string array size = " << arraySize;

        int index;
        for (index = 0; index < arraySize; ++index)
            if (assocTypeStr == assocTypeString[index])
                break;
        if (index < arraySize)
            assocType = Uml::AssociationType::Value(index);
    } else {
        int assocTypeNum = assocTypeStr.toInt();
        if (assocTypeNum < (int)Uml::AssociationType::Generalization ||   // first enum
            assocTypeNum > (int)Uml::AssociationType::Relationship) {     // last enum
            uWarning() << "bad assoctype of UML:AssociationType " << ID2STR(id());
            return false;
        }
        assocType = Uml::AssociationType::Value(assocTypeNum);
    }
    setAssociationType( assocType );

    Uml::IDType roleAObjID = STR2ID(element.attribute( "rolea", "-1" ));
    Uml::IDType roleBObjID = STR2ID(element.attribute( "roleb", "-1" ));
    if (assocType == Uml::AssociationType::Aggregation ||
        assocType == Uml::AssociationType::Composition) {
        // Flip roles to compensate for changed diamond logic in AssociationLine.
        // For further explanations see AssociationWidget::loadFromXMI.
        Uml::IDType tmp = roleAObjID;
        roleAObjID = roleBObjID;
        roleBObjID = tmp;
    }

    UMLObject * objA = doc->findObjectById(roleAObjID);
    UMLObject * objB = doc->findObjectById(roleBObjID);

    if(objA)
        getUMLRole(A)->setObject(objA);
    else
        return false;

    if(objB)
        getUMLRole(B)->setObject(objB);
    else
        return false;

    setMulti(element.attribute( "multia", "" ), A);
    setMulti(element.attribute( "multib", "" ), B);

    setRoleName(element.attribute( "namea", "" ), A);
    setRoleName(element.attribute( "nameb", "" ), B);

    setRoleDoc(element.attribute( "doca", "" ), A);
    setRoleDoc(element.attribute( "docb", "" ), B);

    // Visibility defaults to Public if it cant set it here..
    QString visibilityA = element.attribute( "visibilitya", "0");
    QString visibilityB = element.attribute( "visibilityb", "0");
    int vis = visibilityA.toInt();
    if (vis >= 200)  // bkwd compat.
        vis -= 200;
    setVisibility((Uml::Visibility::Value)vis, A);
    vis = visibilityB.toInt();
    if (vis >= 200)  // bkwd compat.
        vis -= 200;
    setVisibility((Uml::Visibility::Value)vis, B);

    // Changeability defaults to Changeable if it cant set it here..
    QString changeabilityA = element.attribute( "changeabilitya", "0");
    QString changeabilityB = element.attribute( "changeabilityb", "0");
    if (changeabilityA.toInt() > 0)
        setChangeability(Uml::Changeability(Uml::Changeability::Value(changeabilityA.toInt())), A);
    if (changeabilityB.toInt() > 0)
        setChangeability(Uml::Changeability(Uml::Changeability::Value(changeabilityB.toInt())), B);

    return true;
}

/**
 * Returns the UMLObject assigned to the given role.
 * @return  Pointer to the UMLObject in the given role.
 */
UMLObject* UMLAssociation::getObject(Uml::Role_Type role) const
{
    if (m_pRole[role] == NULL)
        return NULL;
    return m_pRole[role]->object();
}

/**
 * Returns the ID of the UMLObject assigned to the given role.
 * Shorthand for getObject(role)->getID().
 * @return  ID of the UMLObject in the given role.
 */
Uml::IDType UMLAssociation::getObjectId(Uml::Role_Type role) const
{
    UMLRole *roleObj = m_pRole[role];
    UMLObject *o = roleObj->object();
    if (o == NULL) {
        QString auxID = roleObj->secondaryId();
        if (auxID.isEmpty()) {
            uError() << "role " << role << ": getObject returns NULL";
            return Uml::id_None;
        } else {
            uDebug() << "role " << role << ": using secondary ID " << auxID;
            return STR2ID(auxID);
        }
    }
    return o->id();
}

/**
 * Returns the ID of the UMLObject assigned to the given role.
 * CURRENTLY UNUSED.
 * @return  ID of the UMLObject of the given role.
 */
Uml::IDType UMLAssociation::getRoleId(Role_Type role) const
{
    return m_pRole[role]->id();
}

/**
 * Returns the changeability.
 */
Uml::Changeability UMLAssociation::changeability(Uml::Role_Type role) const
{
    return m_pRole[role]->changeability();
}

/**
 * Returns the Visibility of the given role.
 * @return  Visibility of the given role.
 */
Uml::Visibility UMLAssociation::getVisibility(Uml::Role_Type role) const
{
    return m_pRole[role]->visibility();
}

/**
 * Returns the multiplicity assigned to the given role.
 * @return  The multiplicity assigned to the given role.
 */
QString UMLAssociation::getMulti(Uml::Role_Type role) const
{
    return m_pRole[role]->multiplicity();
}

/**
 * Returns the name assigned to the role A.
 * @return  The name assigned to the given role.
 */
QString UMLAssociation::getRoleName(Uml::Role_Type role) const
{
    return m_pRole[role]->name();
}

/**
 * Returns the documentation assigned to the given role.
 * @return  Documentation text of given role.
 */
QString UMLAssociation::getRoleDoc(Uml::Role_Type role) const
{
    return m_pRole[role]->doc();
}

/**
 * Get the underlying UMLRole object for the given role.
 * @return  Pointer to the UMLRole object for the given role.
 */
UMLRole * UMLAssociation::getUMLRole(Uml::Role_Type role) const
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
 * @param assocType The AssociationType of the UMLAssociation.
 */
void UMLAssociation::setAssociationType(Uml::AssociationType assocType)
{
    m_AssocType = assocType;
    if (m_AssocType == Uml::AssociationType::UniAssociation)
    {
        // In this case we need to auto-set the multiplicity/rolenames
        // of the roles
#ifdef VERBOSE_DEBUGGING
        uDebug() << " A new uni-association has been created.";
#endif
    }
    UMLObject::emitModified();
}

/**
 * Sets the UMLObject playing the given role in the association.
 * @param obj  Pointer to the UMLObject of the given role.
 * @param role The Uml::Role_Type played by the association
 */
void UMLAssociation::setObject(UMLObject *obj, Uml::Role_Type role)
{
    m_pRole[role]->setObject(obj);
}

/**
 * Sets the visibility of the given role of the UMLAssociation.
 * @param value  Visibility of role.
 * @param role   The Uml::Role_Type to which the visibility is being applied
 */
void UMLAssociation::setVisibility(Uml::Visibility value, Uml::Role_Type role)
{
    m_pRole[role]->setVisibility(value);
}

/**
 * Sets the changeability of the given role of the UMLAssociation.
 * @param value     Changeability_Type of the given role.
 * @param role      The Uml::Role_Type to which the changeability is being set
 */
void UMLAssociation::setChangeability(Uml::Changeability value, Uml::Role_Type role)
{
    m_pRole[role]->setChangeability(value);
}

/**
 * Sets the multiplicity of the given role of the UMLAssociation.
 * @param multi    The multiplicity of the given role.
 * @param role     The Uml::Role_Type to which the multiplicity is being applied
 */
void UMLAssociation::setMulti(const QString &multi, Uml::Role_Type role)
{
    UMLApp::app()->executeCommand(new CmdChangeMulti(m_pRole[role], multi));
    //m_pRole[role]->setMultiplicity(multi);
}

/**
 * Sets the name of the given role of the UMLAssociation.
 * @param roleName  The name to set for the given role.
 * @param role      The Uml::Role_Type for which to set the name.
 */
void UMLAssociation::setRoleName(const QString &roleName, Uml::Role_Type role)
{
    m_pRole[role]->setName(roleName);
}

/**
 * Sets the documentation on the given role in the association.
 * @param doc      The string with the documentation.
 * @param role     The Uml::Role_Type to which the documentation is being applied
 */
void UMLAssociation::setRoleDoc(const QString &doc, Uml::Role_Type role)
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
 * @param type      The AssociationType to represent.
 * @param roleAObj  Pointer to the role A UMLObject.
 * @param roleBObj  Pointer to the role B UMLObject.
 */
void UMLAssociation::init(Uml::AssociationType type, UMLObject *roleAObj, UMLObject *roleBObj)
{
    m_AssocType = type;
    m_BaseType = ot_Association;
    m_Name = "";
    m_bOldLoadMode = false;
    nrof_parent_widgets = -1;
    if (!UMLApp::app()->document()->loading()) {
        m_pUMLPackage = UMLApp::app()->document()->currentRoot();
    }
    m_pRole[Uml::A] = new UMLRole (this, roleAObj, Uml::A);
    m_pRole[Uml::B] = new UMLRole (this, roleBObj, Uml::B);
}

#include "association.moc"
