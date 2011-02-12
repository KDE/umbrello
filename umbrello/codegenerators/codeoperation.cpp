/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "codeoperation.h"

// local includes
#include "classifiercodedocument.h"
#include "debug_utils.h"
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"

CodeOperation::CodeOperation ( ClassifierCodeDocument * doc , UMLOperation * parentOp, const QString & body, const QString & comment)
        : CodeMethodBlock ( doc, parentOp, body, comment)
{
    init(parentOp);
}

CodeOperation::~CodeOperation ( )
{
}

/**
 * Add a Parameter object to the m_parameterVector List
 */
/*
void CodeOperation::addParameter ( CodeParameter * add_object )
{
    m_parameterVector.append(add_object);
}
*/

/**
 * Remove a Parameter object from m_parameterVector List
 */
/*
void CodeOperation::removeParameter ( CodeParameter * remove_object )
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
QList<CodeParameter*> CodeOperation::getParameterList ( )
{
    return m_parameterVector;
}
*/

UMLOperation * CodeOperation::getParentOperation( )
{
    return dynamic_cast<UMLOperation*>(getParentObject());
}

/**
 * Save the XMI representation of this object.
 */
void CodeOperation::saveToXMI ( QDomDocument & doc, QDomElement & root )
{
    QDomElement blockElement = doc.createElement( "codeoperation" );
    // set attributes
    setAttributesOnNode(doc, blockElement);
    root.appendChild( blockElement );
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeOperation::loadFromXMI ( QDomElement & root )
{
    setAttributesFromNode(root);
}

/**
 * Find the value of the tag that this operation would have.
 */
QString CodeOperation::findTag (UMLOperation * op)
{
    return QString("operation_" + ID2STR(op->id()));
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeOperation::setAttributesOnNode ( QDomDocument & doc, QDomElement & elem)
{
    CodeMethodBlock::setAttributesOnNode(doc,elem); // superclass
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeOperation::setAttributesFromNode ( QDomElement & element)
{
    CodeMethodBlock::setAttributesFromNode(element); // superclass

    // now set local attributes

    // oops..this is done in the parent class "ownedcodeblock".
    // we simply need to record the parent operation here
    // m_parentOperation->disconnect(this); // always disconnect from current parent

    QString idStr = element.attribute("parent_id","-1");
    Uml::IDType id = STR2ID(idStr);
    UMLObject * obj = UMLApp::app()->document()->findObjectById(id);
    UMLOperation * op = dynamic_cast<UMLOperation*>(obj);

    if (op)
        init(op);
    else
        uError() << "ERROR: could not load code operation because of missing UMLoperation, corrupt savefile?";
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
    //  connect(parentOp,SIGNAL(modified()),this,SLOT(syncToParent()));
}

void CodeOperation::updateContent()
{
    // Empty. Unlike codeaccessor methods for most (all?) languages
    // we don't auto-generate content for operations
}


#include "codeoperation.moc"
