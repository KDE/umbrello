/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codeoperation.h"

// local includes
#include "classifiercodedocument.h"
#include "debug_utils.h"
#include "umlapp.h"
#include "umldoc.h"
#include "umlobject.h"

// qt/kde includes
#include <QXmlStreamWriter>

CodeOperation::CodeOperation (ClassifierCodeDocument * doc, UMLOperation * parentOp, const QString & body, const QString & comment)
        : CodeMethodBlock (doc, parentOp, body, comment)
{
    init(parentOp);
}

CodeOperation::~CodeOperation ()
{
}

/**
 * Add a Parameter object to the m_parameterVector List
 */
/*
void CodeOperation::addParameter (CodeParameter * add_object)
{
    m_parameterVector.append(add_object);
}
*/

/**
 * Remove a Parameter object from m_parameterVector List
 */
/*
void CodeOperation::removeParameter (CodeParameter * remove_object)
{
    m_parameterVector.remove(remove_object);
}
*/

/**
 * Get the list of Parameter objects held by m_parameterVector
 * @return QList<CodeParameter*> list of Parameter objects held by
 * m_parameterVector
 */
/*
QList<CodeParameter*> CodeOperation::getParameterList ()
{
    return m_parameterVector;
}
*/

UMLOperation * CodeOperation::getParentOperation()
{
    return getParentObject()->asUMLOperation();
}

/**
 * Save the XMI representation of this object.
 */
void CodeOperation::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("codeoperation"));
    // set attributes
    setAttributesOnNode(writer);
    writer.writeEndElement();
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeOperation::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * Find the value of the tag that this operation would have.
 */
QString CodeOperation::findTag (UMLOperation * op)
{
    return QString(QStringLiteral("operation_") + Uml::ID::toString(op->id()));
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeOperation::setAttributesOnNode (QXmlStreamWriter& writer)
{
    CodeMethodBlock::setAttributesOnNode(writer); // superclass
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeOperation::setAttributesFromNode (QDomElement & element)
{
    CodeMethodBlock::setAttributesFromNode(element); // superclass

    // now set local attributes

    // oops..this is done in the parent class "ownedcodeblock".
    // we simply need to record the parent operation here
    // m_parentOperation->disconnect(this); // always disconnect from current parent

    QString idStr = element.attribute(QStringLiteral("parent_id"), QStringLiteral("-1"));
    Uml::ID::Type id = Uml::ID::fromString(idStr);
    UMLObject * obj = UMLApp::app()->document()->findObjectById(id);
    UMLOperation * op = obj->asUMLOperation();

    if (op)
        init(op);
    else
        logError0("could not load code operation because of missing UMLoperation, corrupt savefile?");
}

/**
 * Set the class attributes from a passed object.
 */
void CodeOperation::setAttributesFromObject(TextBlock * obj)
{
    CodeMethodBlock::setAttributesFromObject(obj);

    CodeOperation * op = dynamic_cast<CodeOperation*>(obj);
    if (op)
        init((UMLOperation*) op->getParentObject());
}

void CodeOperation::init (UMLOperation * parentOp)
{
    setCanDelete(false); // we cant delete these with the codeeditor, delete the UML operation instead.
    setTag(CodeOperation::findTag(parentOp));

    // not needed.. done by parent "ownedcodeblock" class
    //  connect(parentOp, SIGNAL(modified()), this, SLOT(syncToParent()));
}

void CodeOperation::updateContent()
{
    // Empty. Unlike codeaccessor methods for most (all?) languages
    // we don't auto-generate content for operations
}


