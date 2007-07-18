/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "association.h"
// qt/kde includes
#include <kdebug.h>
#include <klocale.h>
#include <qregexp.h>
// app includes
#include "classifier.h"
#include "folder.h"
#include "uml.h"
#include "umldoc.h"
#include "umlrole.h"
#include "uniqueid.h"
#include "model_utils.h"

using namespace Uml;

// static members
const Uml::Association_Type UMLAssociation::atypeFirst = Uml::at_Generalization;
const Uml::Association_Type UMLAssociation::atypeLast = Uml::at_Relationship;
const unsigned UMLAssociation::nAssocTypes = (unsigned)atypeLast -
        (unsigned)atypeFirst + 1;

// constructor
UMLAssociation::UMLAssociation( Uml::Association_Type type,
                                UMLObject * roleA, UMLObject * roleB )
        : UMLObject("")
{
    init(type, roleA, roleB);

    m_pRole[Uml::A]->setID( UniqueID::gen() );
    m_pRole[Uml::B]->setID( UniqueID::gen() );
}

UMLAssociation::UMLAssociation( Uml::Association_Type type /* = Uml::at_Unknown */)
        : UMLObject("", Uml::id_Reserved)
{
    init(type, NULL, NULL);
}

// destructor
UMLAssociation::~UMLAssociation( ) {
    if (m_pRole[A] == NULL) {
        kError() << "UMLAssociation destructor: m_pRole[A] is NULL already"
        << endl;
    } else {
        delete m_pRole[A];
        m_pRole[A] = NULL;
    }
    if (m_pRole[B] == NULL) {
        kError() << "UMLAssociation destructor: m_pRole[B] is NULL already"
        << endl;
    } else {
        delete m_pRole[B];
        m_pRole[B] = NULL;
    }
}

bool UMLAssociation::operator==(UMLAssociation &rhs) {
    if (this == &rhs) {
        return true;
    }
    return ( UMLObject::operator== ( rhs ) &&
             m_AssocType == rhs.m_AssocType &&
             m_Name == rhs.m_Name &&
             m_pRole[A] == rhs.m_pRole[A] &&
             m_pRole[B] == rhs.m_pRole[B] );
}

const QString UMLAssociation::assocTypeStr[UMLAssociation::nAssocTypes] = {
            /* The elements must be listed in the same order as in the
               Uml::Association_Type.  */
            i18n("Generalization"),             // at_Generalization
            i18n("Aggregation"),                // at_Aggregation
            i18n("Dependency"),                 // at_Dependency
            i18n("Association"),                // at_Association
            i18n("Self Association"),           // at_Association_Self
            i18n("Collaboration Message"),      // at_Coll_Message
            i18n("Sequence Message"),           // at_Seq_Message
            i18n("Collaboration Self Message"), // at_Coll_Message_Self
            i18n("Sequence Self Message"),      // at_Seq_Message_Self
            i18n("Containment"),                // at_Containment
            i18n("Composition"),                // at_Composition
            i18n("Realization"),                // at_Realization
            i18n("Uni Association"),            // at_UniAssociation
            i18n("Anchor"),                     // at_Anchor
            i18n("State Transition"),           // at_State
            i18n("Activity"),                   // at_Activity
        };

Uml::Association_Type UMLAssociation::getAssocType() const {
    return m_AssocType;
}

QString UMLAssociation::toString ( ) const
{
    QString string;
    if(m_pRole[A])
    {
        string += m_pRole[A]->getObject()->getName();
        string += ':';
        string += m_pRole[A]->getName();
    }
    string += ':' + typeAsString(m_AssocType) + ':';
    if(m_pRole[B])
    {
        string += m_pRole[B]->getObject( )->getName();
        string += ':';
        string += m_pRole[B]->getName();
    }
    return string;
}

QString UMLAssociation::typeAsString (Uml::Association_Type atype)
{
    if (atype < atypeFirst || atype > atypeLast)
        return "";
    return assocTypeStr[(unsigned)atype - (unsigned)atypeFirst];
}

bool UMLAssociation::assocTypeHasUMLRepresentation(Uml::Association_Type atype)
{
    return (atype == Uml::at_Generalization ||
            atype == Uml::at_Realization ||
            atype == Uml::at_Association ||
            atype == Uml::at_Association_Self ||
            atype == Uml::at_UniAssociation ||
            atype == Uml::at_Aggregation ||
            atype == Uml::at_Relationship ||
            atype == Uml::at_Composition ||
            atype == Uml::at_Dependency);
}

bool UMLAssociation::resolveRef() {
    bool successA = getUMLRole(A)->resolveRef();
    bool successB = getUMLRole(B)->resolveRef();
    if (successA && successB) {
        UMLObject *objA = getUMLRole(A)->getObject();
        UMLObject *objB = getUMLRole(B)->getObject();
        // Check if need to change the assoc type to Realization
        if (m_AssocType == Uml::at_Generalization &&
                (objA && objA->getBaseType() == Uml::ot_Interface ||
                 objB && objB->getBaseType() == Uml::ot_Interface))
            m_AssocType = Uml::at_Realization;
        m_pUMLPackage->addAssocToConcepts(this);
        return true;
    }
    return false;
}

void UMLAssociation::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    if (m_AssocType == Uml::at_Generalization) {
        QDomElement assocElement = UMLObject::save("UML:Generalization", qDoc);
        assocElement.setAttribute( "discriminator", "" );
        assocElement.setAttribute( "child", ID2STR(getObjectId(A)) );
        assocElement.setAttribute( "parent", ID2STR(getObjectId(B)) );
        qElement.appendChild( assocElement );
        return;
    }
    if (m_AssocType == Uml::at_Realization) {
        QDomElement assocElement = UMLObject::save("UML:Abstraction", qDoc);
        assocElement.setAttribute( "client", ID2STR(getObjectId(A)) );
        assocElement.setAttribute( "supplier", ID2STR(getObjectId(B)) );
        qElement.appendChild( assocElement );
        return;
    }
    if (m_AssocType == Uml::at_Dependency) {
        QDomElement assocElement = UMLObject::save("UML:Dependency", qDoc);
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

bool UMLAssociation::load( QDomElement & element ) {
    if (getID() == Uml::id_None)
        return false; // old style XMI file. No real info in this association.

    UMLDoc * doc = UMLApp::app()->getDocument();
    UMLObject * obj[2] = { NULL, NULL };
    if (m_AssocType == Uml::at_Generalization ||
        m_AssocType == Uml::at_Realization ||
        m_AssocType == Uml::at_Dependency) {
        for (unsigned r = Uml::A; r <= Uml::B; r++) {
            const QString fetch = (m_AssocType == Uml::at_Generalization ?
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
                    Uml::Model_Type mt = Model_Utils::convert_OT_MT(obj[r]->getBaseType());
                    m_pUMLPackage = doc->getRootFolder(mt);
                    kDebug() << "UMLAssociation::load(assoctype " << m_AssocType
                        << "): setting model type " << mt << endl;
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
                    kError() << "UMLAssociation::load (type " << m_AssocType
                        << ", id " << ID2STR(getID()) << "): "
                        << "xmi id not given for " << tag << endl;
                    continue;
                }
                // Since we know for sure that we're dealing with a non
                // umbrello file, use deferred resolution unconditionally.
                if (tagEq(tag, "child") || tagEq(tag, "subtype") || tagEq(tag, "client")) {
                    getUMLRole(A)->setSecondaryId(idStr);
                } else {
                    getUMLRole(B)->setSecondaryId(idStr);
                }
            }
        }

        // its a realization if either endpoint is an interface
        if (m_AssocType == Uml::at_Generalization &&
                (obj[A] && obj[A]->getBaseType() == Uml::ot_Interface ||
                 obj[B] && obj[B]->getBaseType() == Uml::ot_Interface))
            m_AssocType = Uml::at_Realization;

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
        if (!tagEq(tag, "Association.connection") &&
                !tagEq(tag, "Namespace.ownedElement") &&
                !tagEq(tag, "Namespace.contents")) {
            kWarning() << "UMLAssociation::load: "
            << "unknown child node " << tag << endl;
            continue;
        }
        // Load role A.
        node = tempElement.firstChild();
        while (node.isComment())
            node = node.nextSibling();
        tempElement = node.toElement();
        if (tempElement.isNull()) {
            kWarning() << "UML:Association : element (A) is Null" << endl;
            return false;
        }
        tag = tempElement.tagName();
        if (!tagEq(tag, "AssociationEndRole") &&
                !tagEq(tag, "AssociationEnd")) {
            kWarning() << "UMLAssociation::load: "
            << "unknown child (A) tag " << tag << endl;
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
            kWarning() << "UML:Association : element (B) is Null" << endl;
            return false;
        }
        tag = tempElement.tagName();
        if (!tagEq(tag, "AssociationEndRole") &&
                !tagEq(tag, "AssociationEnd")) {
            kWarning() << "UMLAssociation::load: "
            << "unknown child (B) tag " << tag << endl;
            return false;
        }
        if (! getUMLRole(B)->loadFromXMI(tempElement))
            return false;

        if (m_pUMLPackage == NULL) {
            Uml::Model_Type mt = Model_Utils::convert_OT_MT(getObject(B)->getBaseType());
            m_pUMLPackage = doc->getRootFolder(mt);
            kDebug() << "UMLAssociation::load: setting model type " << mt << endl;
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
        if (m_AssocType == Uml::at_Association && getObjectId(A) == getObjectId(B))
            m_AssocType = Uml::at_Association_Self;

        // fall-back default type
        if (m_AssocType == Uml::at_Unknown) {
            m_AssocType = Uml::at_Association;
        }

        return true;
    }

    // From here on it's old-style stuff.
    QString assocTypeStr = element.attribute( "assoctype", "-1" );
    Uml::Association_Type assocType = Uml::at_Unknown;
    if (assocTypeStr[0] >= 'a' && assocTypeStr[0] <= 'z') {
        // In an earlier version, the natural assoctype names were saved.
        const QString assocTypeString[nAssocTypes] = {
                    "generalization",   // at_Generalization
                    "aggregation",      // at_Aggregation
                    "dependency",       // at_Dependency
                    "association",      // at_Association
                    "associationself",  // at_Association_Self
                    "collmessage",      // at_Coll_Message
                    "seqmessage",       // at_Seq_Message
                    "collmessageself",  // at_Coll_Message_Self
                    "seqmessageself",   // at_Seq_Message_Self
                    "implementation",   // at_Implementation
                    "composition",      // at_Composition
                    "realization",      // at_Realization
                    "uniassociation",   // at_UniAssociation
                    "anchor",           // at_Anchor
                    "state",            // at_State
                    "activity",         // at_Activity
                    "relationship"      // at_Relationship
                };

        unsigned index;
        for (index = 0; index < nAssocTypes; index++)
            if (assocTypeStr == assocTypeString[index])
                break;
        if (index < nAssocTypes)
            assocType = (Uml::Association_Type)index;
    } else {
        int assocTypeNum = assocTypeStr.toInt();
        if (assocTypeNum < (int)atypeFirst || assocTypeNum > (int)atypeLast) {
            kWarning() << "bad assoctype of UML:Association "
            << ID2STR(getID()) << endl;
            return false;
        }
        assocType = (Uml::Association_Type)assocTypeNum;
    }
    setAssocType( assocType );

    Uml::IDType roleAObjID = STR2ID(element.attribute( "rolea", "-1" ));
    Uml::IDType roleBObjID = STR2ID(element.attribute( "roleb", "-1" ));
    if (assocType == at_Aggregation || assocType == at_Composition) {
        // Flip roles to compensate for changed diamond logic in LinePath.
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
    if (visibilityA.toInt() > 0)
        setVisibility((Uml::Visibility::Value)visibilityA.toInt(), A);
    if (visibilityB.toInt() > 0)
        setVisibility((Uml::Visibility::Value)visibilityB.toInt(), B);

    // Changeability defaults to Changeable if it cant set it here..
    QString changeabilityA = element.attribute( "changeabilitya", "0");
    QString changeabilityB = element.attribute( "changeabilityb", "0");
    if (changeabilityA.toInt() > 0)
        setChangeability ( (Uml::Changeability_Type) changeabilityA.toInt(), A);
    if (changeabilityB.toInt() > 0)
        setChangeability ( (Uml::Changeability_Type) changeabilityB.toInt(), B);

    return true;
}

UMLObject* UMLAssociation::getObject(Uml::Role_Type role) {
    return m_pRole[role]->getObject();
}

Uml::IDType UMLAssociation::getObjectId(Uml::Role_Type role) {
    UMLRole *roleObj = m_pRole[role];
    UMLObject *o = roleObj->getObject();
    if (o == NULL) {
        QString auxID = roleObj->getSecondaryId();
        if (auxID.isEmpty()) {
            kError() << "UMLAssociation::getObjectId(" << role
            << "): getObject returns NULL" << endl;
            return Uml::id_None;
        } else {
            kDebug() << "UMLAssociation::getObjectId(" << role
            << "): using secondary ID " << auxID << endl;
            return STR2ID(auxID);
        }
    }
    return o->getID();
}

/* CURRENTLY UNUSED
Uml::IDType UMLAssociation::getRoleId(Role_Type role) const {
    return m_pRole[role]->getID();
}
 */

Uml::Changeability_Type UMLAssociation::getChangeability(Uml::Role_Type role) const {
    return m_pRole[role]->getChangeability();
}

Uml::Visibility UMLAssociation::getVisibility(Uml::Role_Type role) const {
    return m_pRole[role]->getVisibility();
}

QString UMLAssociation::getMulti(Uml::Role_Type role) const {
    return m_pRole[role]->getMultiplicity();
}

QString UMLAssociation::getRoleName(Uml::Role_Type role) const {
    return m_pRole[role]->getName();
}

QString UMLAssociation::getRoleDoc(Uml::Role_Type role) const {
    return m_pRole[role]->getDoc();
}

UMLRole * UMLAssociation::getUMLRole(Uml::Role_Type role) {
    return m_pRole[role];
}

void UMLAssociation::setOldLoadMode(bool value /* = true */) {
    m_bOldLoadMode = value;
}

bool UMLAssociation::getOldLoadMode() const {
    return m_bOldLoadMode;
}

void UMLAssociation::setAssocType(Uml::Association_Type assocType) {
    m_AssocType = assocType;
    if(m_AssocType == at_UniAssociation)
    {
        // In this case we need to auto-set the multiplicity/rolenames
        // of the roles
#ifdef VERBOSE_DEBUGGING
        kDebug() << " A new uni-association has been created." << endl;
#endif
    }
    UMLDoc *umldoc = UMLApp::app()->getDocument();
    if (! umldoc->loading())
        emit modified();
}

void UMLAssociation::setObject(UMLObject *obj, Uml::Role_Type role) {
    m_pRole[role]->setObject(obj);
}

void UMLAssociation::setVisibility(Uml::Visibility value, Uml::Role_Type role) {
    m_pRole[role]->setVisibility(value);
}

void UMLAssociation::setChangeability(Uml::Changeability_Type value, Uml::Role_Type role) {
    m_pRole[role]->setChangeability(value);
}

void UMLAssociation::setMulti(const QString &value, Uml::Role_Type role) {
    m_pRole[role]->setMultiplicity(value);
}

void UMLAssociation::setRoleName(const QString &value, Uml::Role_Type role) {
    m_pRole[role]->setName(value);
}

void UMLAssociation::setRoleDoc(const QString &doc, Uml::Role_Type role) {
    m_pRole[role]->setDoc(doc);
}

QString UMLAssociation::ChangeabilityToString(Uml::Changeability_Type type) {
    switch (type) {
    case Uml::chg_Frozen:
        return "frozen";
        break;
    case Uml::chg_AddOnly:
        return "addOnly";
        break;
    case Uml::chg_Changeable:
    default:
        return "changeable";
        break;
    }
}

void UMLAssociation::init(Uml::Association_Type type, UMLObject *roleAObj, UMLObject *roleBObj) {
    m_AssocType = type;
    m_BaseType = ot_Association;
    m_Name = "";
    m_bOldLoadMode = false;
    nrof_parent_widgets = -1;
    if (!UMLApp::app()->getDocument()->loading())
        m_pUMLPackage = UMLApp::app()->getDocument()->currentRoot();
    m_pRole[Uml::A] = new UMLRole (this, roleAObj, Uml::A);
    m_pRole[Uml::B] = new UMLRole (this, roleBObj, Uml::B);
}


#include "association.moc"
