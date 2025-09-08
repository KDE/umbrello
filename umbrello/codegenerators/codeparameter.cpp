/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codeparameter.h"

// local includes
#include "umlassociation.h"
#include "attribute.h"
#include "classifiercodedocument.h"
#include "debug_utils.h"
#include "umldoc.h"
#include "umlobject.h"
#include "umlrole.h"
#include "uml.h"
#include "codegenfactory.h"

// qt/kde includes
#include <QXmlStreamWriter>

/**
 * Constructor.
 */
CodeParameter::CodeParameter(ClassifierCodeDocument * parentDoc, UMLObject * parentObject)
        : QObject(parentObject)
{
    setObjectName(QStringLiteral("ACodeParam"));
    initFields(parentDoc, parentObject);
}

/**
 * Destructor.
 */
CodeParameter::~CodeParameter()
{
}

/**
 * Utility method to get the value of parent object abstract value
 * @return the value of parent object abstrtact
 */
bool CodeParameter::getAbstract()
{
    return m_parentObject->isAbstract();
}

/**
 * Utility method to get the value of parent object static
 * Whether or not this is static.
 * @return the value of static
 */
bool CodeParameter::getStatic()
{
    return m_parentObject->isStatic();
}

/**
 * Utility method to get the value of parent object name
 * The name of this code parameter.
 * @return the value
 */
QString CodeParameter::getName() const
{
    return m_parentObject->name();
}

/**
 * Utility method to get the value of parent object type.
 * the typeName of this parameters (e.g. boolean, int, etc or perhaps Class name of
 * an object)
 * @return the value of type
 */
QString CodeParameter::getTypeName()
{
    UMLAttribute * at = (UMLAttribute*) m_parentObject;
    return at->getTypeName();
}

/**
 * Utility method to get the value of parent object scope.
 * The visibility of this code parameter.
 * @return the value of parent object scope
 */
Uml::Visibility::Enum CodeParameter::getVisibility() const
{
    return m_parentObject->visibility();
}

/**
 * Set the value of m_initialValue.
 * The initial value of this code parameter.
 * @param new_var the new value of m_initialValue
 */
void CodeParameter::setInitialValue(const QString &new_var)
{
    m_initialValue = new_var;
}

/**
 * Get the value of m_initialValue
 * The initial value of this code parameter
 * @return the value of m_initialValue
 */
QString CodeParameter::getInitialValue()
{
    return m_initialValue;
}

/**
 * Set a Comment object.
 */
void CodeParameter::setComment(CodeComment * object)
{
    m_comment = object;
}

/**
 * Get the Comment on this object.
 */
CodeComment * CodeParameter::getComment()
{
    return m_comment;
}

/**
 * Get the parent Code Document
 */
ClassifierCodeDocument * CodeParameter::getParentDocument()
{
    return m_parentDocument;
}

/**
 * Get the ParentObject object.
 */
UMLObject * CodeParameter::getParentObject()
{
    return m_parentObject;
}

// need to get the ID of the parent object
// this is kind of broken for UMLRoles.
QString CodeParameter::ID() const
{
    const UMLRole * role = m_parentObject->asUMLRole();
    if (role)
    {
        // cant use Role "ID" as that is used to distinquish if its
        // role "A" or "B"
        const UMLAssociation *assoc = role->parentAssociation();
        return Uml::ID::toString(assoc->id());
    } else
        return Uml::ID::toString(m_parentObject->id());

}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeParameter::setAttributesOnNode(QXmlStreamWriter& writer)
{
    // set local attributes
    writer.writeAttribute(QStringLiteral("parent_id"), ID());

    // setting ID's takes special treatment
    // as UMLRoles arent properly stored in the XMI right now.
    // (change would break the XMI format..save for big version change)
    const UMLRole * role = m_parentObject->asUMLRole();
    if (role)
        writer.writeAttribute(QStringLiteral("role_id"), QString::number(role->role()));
    else
        writer.writeAttribute(QStringLiteral("role_id"), QStringLiteral("-1"));

    writer.writeAttribute(QStringLiteral("initialValue"), getInitialValue());

    // a comment which we will store in its own separate child node block
    writer.writeStartElement(QStringLiteral("header"));
    getComment()->saveToXMI(writer); // comment
    writer.writeEndElement();
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeParameter::setAttributesFromNode(QDomElement & root)
{
    // set local attributes, parent object first
    QString idStr = root.attribute(QStringLiteral("parent_id"), QStringLiteral("-1"));
    Uml::ID::Type id = Uml::ID::fromString(idStr);

    // always disconnect
    m_parentObject->disconnect(this);

    // now, what is the new object we want to set?
    UMLObject * obj = UMLApp::app()->document()->findObjectById(id);
    if (obj)
    {
        // FIX..one day.
        // Ugh. This is UGLY, but we have to do it this way because UMLRoles
        // don't go into the document list of UMLobjects, and have the same
        // ID as their parent UMLAssociations. So..the drill is then special
        // for Associations..in that case we need to find out which role will
        // serve as the parameter here. The REAL fix, of course, would be to
        // treat UMLRoles on a more even footing, but im not sure how that change
        // might ripple throughout the code and cause problems. Thus, since the
        // change appears to be needed for only this part, I'll do this crappy
        // change instead. -b.t.
        UMLAssociation * assoc = obj->asUMLAssociation();
        if (assoc) {
            // In this case we init with indicated role child obj.
            UMLRole  *role = nullptr;
            int role_id = root.attribute(QStringLiteral("role_id"), QStringLiteral("-1")).toInt();
            if (role_id == 1)
                role = assoc->getUMLRole(Uml::RoleType::A);
            else if (role_id == 0)
                role = assoc->getUMLRole(Uml::RoleType::B);
            else
                logError2("CodeParameter::setAttributesFromNode: corrupt save file? "
                          "cant get proper UMLRole for codeparameter uml id: %1 w/role_id: %2",
                          Uml::ID::toString(id), role_id);

            // init using UMLRole obj
            if (role)
                initFields (m_parentDocument, role);

        } else
            initFields (m_parentDocument, obj); // just the regular approach

    } else
        logError1("CodeParameter::setAttributesFromNode: Cant load CodeParam: parentUMLObject w/id: %1 not found",
                  Uml::ID::toString(id));

    // other attribs now
    setInitialValue(root.attribute(QStringLiteral("initialValue")));

    // load comment now
    // by looking for our particular child element
    QDomNode node = root.firstChild();
    QDomElement element = node.toElement();
    bool gotComment = false;
    while (!element.isNull()) {
        QString tag = element.tagName();
        if (tag == QStringLiteral("header")) {
            QDomNode cnode = element.firstChild();
            QDomElement celem = cnode.toElement();
            getComment()->loadFromXMI(celem);
            gotComment = true;
            break;
        }
        node = element.nextSibling();
        element = node.toElement();
    }

    if (!gotComment)
        logWarn0("CodeParameter::setAttributesFromNode: unable to initialize CodeComment in codeparam");
}

/**
 * Create the string representation of this code parameter.
 * @return      QString
 */
void CodeParameter::syncToParent()
{
    getComment()->setText(getParentObject()->doc());

    updateContent();
}

void CodeParameter::initFields(ClassifierCodeDocument * doc, UMLObject * obj)
{
    m_parentObject = obj;

    m_parentDocument = doc;
    m_initialValue.clear();

    m_comment = CodeGenFactory::newCodeComment(m_parentDocument);
    m_comment->setText(getParentObject()->doc());

    connect(m_parentObject, SIGNAL(modified()), this, SLOT(syncToParent()));
}

