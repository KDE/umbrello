/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "ownedcodeblock.h"

// local includes
#include "association.h"
#include "classifier.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"
#include "textblock.h"

// qt includes
#include <QXmlStreamWriter>

/**
 * Constructor
 */
OwnedCodeBlock::OwnedCodeBlock (UMLObject * parent)
    : QObject (parent)
{
    setObjectName(QLatin1String("anOwnedCodeBlock"));
    initFields(parent);
}

/**
 * Empty Destructor
 */
OwnedCodeBlock::~OwnedCodeBlock ()
{
    /*
        if (m_parentObject) {
                m_parentObject->disconnect(this);
        }
    */
}

/**
 * Causes the text block to release all of its connections
 * and any other text blocks that it 'owns'.
 * Needed to be called prior to deletion of the textblock.
 */
void OwnedCodeBlock::release ()
{
    if (m_parentObject) {
        m_parentObject->disconnect(this);
    }
    m_parentObject = 0;
}

/**
 * Get the value of m_parentObject.
 * @return the value of m_parentObject
 */
UMLObject * OwnedCodeBlock::getParentObject ()
{
    return m_parentObject;
}

/**
 * Set the class attributes from a passed object.
 */
void OwnedCodeBlock::setAttributesFromObject (TextBlock * obj)
{
    OwnedCodeBlock * oc = dynamic_cast<OwnedCodeBlock*>(obj);
    if (oc) {
        m_parentObject->disconnect(this);
        initFields(oc->getParentObject());
    }
}

void OwnedCodeBlock::setAttributesOnNode(QXmlStreamWriter& writer)
{
    // set local class attributes
    // setting ID's takes special treatment
    // as UMLRoles arent properly stored in the XMI right now.
    // (change would break the XMI format..save for big version change)
    UMLRole * role = m_parentObject->asUMLRole();
    if (role) {
        writer.writeAttribute(QLatin1String("parent_id"), Uml::ID::toString(role->parentAssociation()->id()));
        // CAUTION: role_id here is numerically inverted wrt Uml::Role_Type,
        //          i.e. role A is 1 and role B is 0.
        //          I'll resist the temptation to change this -
        //          in order to maintain backward compatibility.
        writer.writeAttribute(QLatin1String("role_id"), QString::number((role->role() == Uml::RoleType::A)));
    }
    else {
        writer.writeAttribute(QLatin1String("parent_id"), Uml::ID::toString(m_parentObject->id()));
        //elem.setAttribute(QLatin1String("role_id"),QLatin1String("-1"));
    }
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void OwnedCodeBlock::setAttributesFromNode (QDomElement & elem)
{
    // set local attributes, parent object first
    QString idStr = elem.attribute(QLatin1String("parent_id"), QLatin1String("-1"));
    Uml::ID::Type id = Uml::ID::fromString(idStr);

    // always disconnect from current parent
    getParentObject()->disconnect(this);

    // now, what is the new object we want to set?
    UMLObject * obj = UMLApp::app()->document()->findObjectById(id);
    if (obj) {
        // FIX..one day.
        // Ugh. This is UGLY, but we have to do it this way because UMLRoles
        // don't go into the document list of UMLobjects, and have the same
        // ID as their parent UMLAssociations. So..the drill is then special
        // for Associations..in that case we need to find out which role will
        // serve as the parametger here. The REAL fix, of course, would be to
        // treat UMLRoles on a more even footing, but im not sure how that change
        // might ripple throughout the code and cause problems. Thus, since the
        // change appears to be needed for only this part, I'll do this crappy
        // change instead. -b.t.
        UMLAssociation * assoc = obj->asUMLAssociation();
        if (assoc) {
            // In this case we init with indicated role child obj.
            UMLRole * role = 0;
            int role_id = elem.attribute(QLatin1String("role_id"), QLatin1String("-1")).toInt();
            // see comment on role_id at setAttributesOnNode()
            if (role_id == 1)
                role = assoc->getUMLRole(Uml::RoleType::A);
            else if (role_id == 0)
                role = assoc->getUMLRole(Uml::RoleType::B);
            else // this will cause a crash
                uError() << "corrupt save file? "
                         << "cant get proper UMLRole for ownedcodeblock uml id:"
                         << Uml::ID::toString(id) << " w/role_id:" << role_id;

            // init using UMLRole obj
            initFields (role);
        }
        else
            initFields (obj); // just the regular approach
    } 
    else
        uError() << "ERROR: cannot load ownedcodeblock: parentUMLObject w/id:"
                 << Uml::ID::toString(id) << " not found, corrupt save file?";
}

void OwnedCodeBlock::initFields(UMLObject * parent)
{
    m_parentObject = parent;

    // one reason for being: set up the connection between
    // this code block and the parent UMLObject..when the parent
    // signals a change has been made, we automatically update
    // ourselves
    connect(m_parentObject, SIGNAL(modified()), this, SLOT(syncToParent()));
}

void OwnedCodeBlock::syncToParent()
{
    updateContent();
}

