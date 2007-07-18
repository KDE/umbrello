/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlrole.h"

// qt/kde includes
#include <kdebug.h>
#include <qregexp.h>

// local includes
#include "association.h"
#include "umldoc.h"
#include "uml.h"


// constructor
UMLRole::UMLRole(UMLAssociation * parent, UMLObject * parentObj, Uml::Role_Type role)
        : UMLObject(const_cast<UMLAssociation*>(parent))
{
    init(parent, parentObj, role);
}

UMLRole::~UMLRole() { }

bool UMLRole::operator==(UMLRole &rhs) {
    if (this == &rhs) {
        return true;
    }
    return ( UMLObject::operator==( rhs ) &&
             m_Changeability == rhs.m_Changeability &&
             m_Multi == rhs.m_Multi &&
             m_Name == rhs.m_Name
           );
}

UMLAssociation * UMLRole::getParentAssociation () {
    return m_pAssoc;
}

UMLObject* UMLRole::getObject() {
    return m_pSecondary;
}

Uml::Changeability_Type UMLRole::getChangeability() const {
    return m_Changeability;
}

QString UMLRole::getMultiplicity() const {
    return m_Multi;
}

void UMLRole::setObject (UMLObject *obj) {
    // because we will get the id of this role from the parent
    // object, we CANT allow UMLRoles to take other UMLRoles as
    // parent objects. In fact, there is probably good reason
    // to only take UMLClassifiers here, but I'll leave it more open
    // for the time being. -b.t.
    if (obj && dynamic_cast<UMLRole*>(obj)) {
        kError() << "UMLRole(" << ID2STR(m_nId) << ") cannot setObject() to another UMLRole("
            << ID2STR(obj->getID()) << ")" << endl;
        return;
    }

    m_pSecondary = obj;
    UMLObject::emitModified();
}

void UMLRole::setChangeability (Uml::Changeability_Type value) {
    m_Changeability = value;
    UMLObject::emitModified();
}

void UMLRole::setMultiplicity ( const QString &multi ) {
    m_Multi = multi;
    UMLObject::emitModified();
}

Uml::Role_Type UMLRole::getRole() {
    return m_role;
}

void UMLRole::init(UMLAssociation * parent, UMLObject * parentObj, Uml::Role_Type r) {
    m_BaseType = Uml::ot_Role;
    m_role = r;
    m_pAssoc = parent;
    m_pSecondary = parentObj;
    m_Multi = "";
    m_Name = "";
    m_Changeability = Uml::chg_Changeable;

    // connect this up to parent
    connect(this,SIGNAL(modified()),parent,SIGNAL(modified()));
}

void UMLRole::saveToXMI( QDomDocument & qDoc, QDomElement & qElement ) {
    QDomElement roleElement = UMLObject::save("UML:AssociationEnd", qDoc);
    if (m_pSecondary)
        roleElement.setAttribute( "type", ID2STR(m_pSecondary->getID()) );
    else
        kError() << "UMLRole::saveToXMI(id " << ID2STR(m_nId)
        << "): m_pSecondary is NULL" << endl;
    if (!m_Multi.isEmpty())
        roleElement.setAttribute("multiplicity", m_Multi);
    if (m_role == Uml::A) {  // role aggregation based on parent type
        // role A
        switch (m_pAssoc->getAssocType()) {
        case Uml::at_Composition:
            roleElement.setAttribute("aggregation", "composite");
            break;
        case Uml::at_Aggregation:
            roleElement.setAttribute("aggregation", "aggregate");
            break;
        default:
            roleElement.setAttribute("aggregation", "none");
            break;
        }
        if (m_pAssoc->getAssocType() == Uml::at_UniAssociation) {
            // Normally the isNavigable attribute is "true".
            // We set it to false on role A to indicate that
            // role B gets an explicit arrowhead.
            roleElement.setAttribute("isNavigable", "false");
        } else {
            roleElement.setAttribute("isNavigable", "true");
        }
    } else {
        roleElement.setAttribute("aggregation", "none");
        roleElement.setAttribute("isNavigable", "true");
        //FIXME obviously this isn't standard XMI
        if (m_pAssoc->getAssocType() == Uml::at_Relationship) {
            roleElement.setAttribute("relationship", "true");
        }
    }

    roleElement.setAttribute("visibility", getVisibility().toString(false));

    switch (m_Changeability) {
    case Uml::chg_Frozen:
        roleElement.setAttribute("changeability", "frozen");
        break;
    case Uml::chg_AddOnly:
        roleElement.setAttribute("changeability", "addOnly");
        break;
    case Uml::chg_Changeable:
        roleElement.setAttribute("changeability", "changeable");
        break;
    }
    qElement.appendChild( roleElement );
}

bool UMLRole::load( QDomElement & element ) {
    UMLDoc * doc = UMLApp::app()->getDocument();
    QString type = element.attribute("type", "");
    if (!type.isEmpty()) {
        if (!m_SecondaryId.isEmpty())
            kWarning() << "UMLRole::load: overwriting old m_SecondaryId \""
            << m_SecondaryId << " with new value \""
            << type << "\"" << endl;
        m_SecondaryId = type;
    }
    // Inspect child nodes - for multiplicity (and type if not set above.)
    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isComment())
            continue;
        QDomElement tempElement = node.toElement();
        QString tag = tempElement.tagName();
        if (Uml::tagEq(tag, "name")) {
            m_Name = tempElement.text();
        } else if (Uml::tagEq(tag, "AssociationEnd.multiplicity")) {
            /**
             * There are different ways in which the multiplicity might be given:
             *  - direct value in the <AssociationEnd.multiplicity> tag,
             *  - attributes "lower" and "upper" of a subordinate <MultiplicityRange>,
             *  - direct value in subordinate <MultiplicityRange.lower> and
             *    <MultiplicityRange.upper> tags
             */
            QDomNode n = tempElement.firstChild();
            if (node.isNull() || tempElement.isNull() || n.isNull() ||
                    n.toElement().isNull()) {
                m_Multi = tempElement.text().stripWhiteSpace();
                continue;
            }
            tempElement = n.toElement();
            tag = tempElement.tagName();
            if (!Uml::tagEq(tag, "Multiplicity")) {
                m_Multi = tempElement.text().stripWhiteSpace();
                continue;
            }
            n = tempElement.firstChild();
            tempElement = n.toElement();
            tag = tempElement.tagName();
            if (!Uml::tagEq(tag, "Multiplicity.range")) {
                m_Multi = tempElement.text().stripWhiteSpace();
                continue;
            }
            n = tempElement.firstChild();
            tempElement = n.toElement();
            tag = tempElement.tagName();
            if (!Uml::tagEq(tag, "MultiplicityRange")) {
                m_Multi = tempElement.text().stripWhiteSpace();
                continue;
            }
            QString multiUpper;
            if (tempElement.hasAttribute("lower")) {
                m_Multi = tempElement.attribute("lower", "");
                multiUpper = tempElement.attribute("upper", "");
                if (!multiUpper.isEmpty()) {
                    if (!m_Multi.isEmpty())
                        m_Multi.append("..");
                    m_Multi.append(multiUpper);
                }
                continue;
            }
            n = tempElement.firstChild();
            while (!n.isNull()) {
                tempElement = n.toElement();
                tag = tempElement.tagName();
                if (Uml::tagEq(tag, "MultiplicityRange.lower")) {
                    m_Multi = tempElement.text();
                } else if (Uml::tagEq(tag, "MultiplicityRange.upper")) {
                    multiUpper = tempElement.text();
                }
                n = n.nextSibling();
            }
            if (!multiUpper.isEmpty()) {
                if (!m_Multi.isEmpty())
                    m_Multi.append("..");
                m_Multi.append(multiUpper);
            }
        } else if (m_SecondaryId.isEmpty() &&
                   (Uml::tagEq(tag, "type") ||
                    Uml::tagEq(tag, "participant"))) {
            m_SecondaryId = tempElement.attribute("xmi.id", "");
            if (m_SecondaryId.isEmpty())
                m_SecondaryId = tempElement.attribute("xmi.idref", "");
            if (m_SecondaryId.isEmpty()) {
                QDomNode inner = tempElement.firstChild();
                QDomElement innerElem = inner.toElement();
                m_SecondaryId = innerElem.attribute("xmi.id", "");
                if (m_SecondaryId.isEmpty())
                    m_SecondaryId = innerElem.attribute("xmi.idref", "");
            }
        }
    }
    if (!m_Multi.isEmpty())
        kDebug() << "UMLRole::load(" << m_Name << "): m_Multi is " << m_Multi << endl;
    if (m_SecondaryId.isEmpty()) {
        kError() << "UMLRole::load(" << m_Name << "): type not given or illegal" << endl;
        return false;
    }
    UMLObject * obj;
    obj = doc->findObjectById(STR2ID(m_SecondaryId));
    if (obj) {
        m_pSecondary = obj;
        m_SecondaryId = "";
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
    // when (m_role == Uml::A) but some XMI writers (e.g. StarUML) place
    // the aggregation attribute at role B.
    // The role end with the aggregation unequal to "none" wins.
    QString aggregation = element.attribute("aggregation", "none");
    if (aggregation == "composite")
        m_pAssoc->setAssocType(Uml::at_Composition);
    else if (aggregation == "shared"       // UML1.3
          || aggregation == "aggregate")   // UML1.4
        m_pAssoc->setAssocType(Uml::at_Aggregation);

    if (!element.hasAttribute("isNavigable")) {
        /* Backward compatibility mode: In Umbrello version 1.3.x the
           logic for saving the isNavigable flag was wrong.
           May happen on loading role A.
         */
        m_pAssoc->setOldLoadMode(true);
    } else if (m_pAssoc->getOldLoadMode() == true) {
        /* Here is the original logic:
           " Role B:
             If isNavigable is not given, we make no change to the
             association type.
             If isNavigable is given, and is "true", then we assume that
             the association's other end (role A) is not navigable, and
             therefore we change the association type to UniAssociation.
             The case that isNavigable is given as "false" is ignored.
             Combined with the association type logic for role A, this
             allows us to support at_Association and at_UniAssociation. "
         */
        if (element.attribute("isNavigable") == "true")
            m_pAssoc->setAssocType(Uml::at_UniAssociation);
    } else if (element.attribute("isNavigable") == "false") {
        m_pAssoc->setAssocType(Uml::at_UniAssociation);
    }

    //FIXME not standard XMI
    if (element.hasAttribute("relationship")) {
        if (element.attribute("relationship") == "true") {
            m_pAssoc->setAssocType(Uml::at_Relationship);
        }
    }

    if (m_Multi.isEmpty())
        m_Multi = element.attribute("multiplicity", "");

    // Changeability defaults to Changeable if it cant set it here..
    m_Changeability = Uml::chg_Changeable;
    QString changeability = element.attribute("changeability", "");
    if (changeability.isEmpty())
        element.attribute("changeable", "");  // for backward compatibility
    if (changeability == "frozen")
        m_Changeability = Uml::chg_Frozen;
    else if (changeability == "addOnly")
        m_Changeability = Uml::chg_AddOnly;

    // finished config, now unblock
    blockSignals(false);
    return true;
}

#include "umlrole.moc"
