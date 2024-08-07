/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "codeclassfielddeclarationblock.h"

#include "codeclassfield.h"
#include "umlrole.h"

#include <QXmlStreamWriter>

/**
 * Constructor.
 */
CodeClassFieldDeclarationBlock::CodeClassFieldDeclarationBlock(CodeClassField * parentCF)
        : OwnedCodeBlock((UMLObject*) parentCF->getParentObject()),
          CodeBlockWithComments((CodeDocument*) parentCF->getParentDocument())
{
    init(parentCF);
}

/**
 * Empty Destructor
 */
CodeClassFieldDeclarationBlock::~CodeClassFieldDeclarationBlock()
{
    // Q: is this needed??
    //      m_parentclassfield->getParentObject()->disconnect(this);
}

/**
 * Get the value of m_parentclassfield.
 * @return the value of m_parentclassfield
 */
CodeClassField * CodeClassFieldDeclarationBlock::getParentClassField()
{
    return m_parentclassfield;
}

/**
 * A utility method to get the parent object of the parentCodeClassfield.
 */
UMLObject * CodeClassFieldDeclarationBlock::getParentObject()
{
    return m_parentclassfield->getParentObject();
}

// this type of textblock is special
// we DON'T release it when resetTextBlocks is
// called because we re-use it over and over
// until the codeclassfield is released.
void CodeClassFieldDeclarationBlock::release()
{
    // do nothing
}

/**
 * So parent can actually release this block.
 */
void CodeClassFieldDeclarationBlock::forceRelease()
{
    if (m_parentclassfield)
    {
        // m_parentclassfield->getParentObject()->disconnect(this);
        m_parentclassfield->disconnect(this);
    }
    m_parentclassfield = nullptr;
    OwnedCodeBlock::release();
    TextBlock::release();
}

/**
 * Save the XMI representation of this object.
 */
void CodeClassFieldDeclarationBlock::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("ccfdeclarationcodeblock"));
    setAttributesOnNode(writer);
    writer.writeEndElement();
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeClassFieldDeclarationBlock::loadFromXMI (QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeClassFieldDeclarationBlock::setAttributesOnNode (QXmlStreamWriter& writer)
{
    // set super-class attributes
    CodeBlockWithComments::setAttributesOnNode(writer);
    OwnedCodeBlock::setAttributesOnNode(writer);
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeClassFieldDeclarationBlock::setAttributesFromNode(QDomElement & root)
{
    // set attributes from the XMI
    CodeBlockWithComments::setAttributesFromNode(root); // superclass load
    OwnedCodeBlock::setAttributesFromNode(root); // superclass load

    syncToParent();
}

/**
 * Set the class attributes from a passed object.
 */
void CodeClassFieldDeclarationBlock::setAttributesFromObject (TextBlock * obj)
{
    CodeBlockWithComments::setAttributesFromObject(obj);

    CodeClassFieldDeclarationBlock * ccb = dynamic_cast<CodeClassFieldDeclarationBlock*>(obj);
    if (ccb)
    {
        m_parentclassfield->disconnect(this);
        init(ccb->getParentClassField());

        syncToParent();
    }
}

void CodeClassFieldDeclarationBlock::syncToParent ()
{
    // for role-based accessors, we DON'T write ourselves out when
    // the name of the role is not defined.
    if (!(getParentClassField()->parentIsAttribute()))
    {
        const UMLRole * parent = getParentObject()->asUMLRole();
        if (parent == nullptr)
            return;
        if (parent->name().isEmpty())
        {
            getComment()->setWriteOutText(false);
            setWriteOutText(false);
        } else {
            getComment()->setWriteOutText(true);
            setWriteOutText(true);
        }
    }

    // only update IF we are NOT AutoGenerated
    if (contentType() != AutoGenerated)
        return;

    updateContent();
}

void CodeClassFieldDeclarationBlock::init (CodeClassField * parentCF)
{
    m_parentclassfield = parentCF;
    setCanDelete(false);
    connect(m_parentclassfield, SIGNAL(modified()), this, SLOT(syncToParent()));
}

