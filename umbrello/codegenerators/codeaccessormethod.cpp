/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "codeaccessormethod.h"

// qt/kde includes
#include <QXmlStreamWriter>

// local includes
#include "codeclassfield.h"

/**
 * Constructors
 */
CodeAccessorMethod::CodeAccessorMethod(CodeClassField * parentCF)
  : CodeMethodBlock (parentCF->getParentDocument(), parentCF->getParentObject())
{
    initFields(parentCF);
}

/**
 * Empty Destructor
 */
CodeAccessorMethod::~CodeAccessorMethod()
{
}

/**
 * Get the value of m_parentclassfield
 * @return the value of m_parentclassfield
 */
CodeClassField * CodeAccessorMethod::getParentClassField()
{
    return m_parentclassfield;
}

bool CodeAccessorMethod::parentIsAttribute()
{
    return getParentClassField()->parentIsAttribute();
}

/**
 * Utility method to get the value of the parent object of the parent classifield.
 * @return the value of the parent of the parent classfield
 */
/*
UMLObject * CodeAccessorMethod::getParentObject()
{
    return getParentClassField()->getParentObject();
}
*/

/**
 * Return the type of accessor method this is.
 */
CodeAccessorMethod::AccessorType CodeAccessorMethod::getType()
{
    return m_accessorType;
}

/**
 * Set the type of accessor method this is.
 */
void CodeAccessorMethod::setType(CodeAccessorMethod::AccessorType atype)
{
    m_accessorType = atype;
}

/**
 * This type of textblock is special
 * we DON'T release it when resetTextBlocks is
 * called because we re-use it over and over
 * until the codeclassfield is released.
 */
void CodeAccessorMethod::release()
{
    // do nothing
}

/**
 * A method so the parent code classfield can force code block to release.
 */
void CodeAccessorMethod::forceRelease()
{
    if (m_parentclassfield) {
        m_parentclassfield->disconnect(this);
    }
    CodeMethodBlock::release();
}

/**
 * Load params from the appropriate XMI element node.
 */
void CodeAccessorMethod::loadFromXMI(QDomElement & root)
{
    setAttributesFromNode(root);
}

/**
 * Save the XMI representation of this object.
 */
void CodeAccessorMethod::saveToXMI(QXmlStreamWriter& writer)
{
    writer.writeStartElement(QStringLiteral("codeaccessormethod"));

    setAttributesOnNode(writer);

    writer.writeEndElement();
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeAccessorMethod::setAttributesOnNode(QXmlStreamWriter& writer)
{
    // set super-class attributes
    CodeMethodBlock::setAttributesOnNode(writer);

    // set local class attributes
    writer.writeAttribute(QStringLiteral("accessType"), QString::number(getType()));
    writer.writeAttribute(QStringLiteral("classfield_id"), getParentClassField()->ID());
}

/**
 * Set the class attributes of this object from
 * the passed element node.
 */
void CodeAccessorMethod::setAttributesFromNode(QDomElement & root)
{
    // set attributes from the XMI
    CodeMethodBlock::setAttributesFromNode(root); // superclass load

    /*
        // I don't believe this is needed for a load from XMI. We never delete
        // accessor methods from the parent classfield.. they are essentially
        // in composition with the parent class and are arent meant to be out
        // on their own. Well, this is fine for now, but IF we start allowing
        // clipping and pasting of these methods between classes/ classfields
        // then we may have problems (ugh.. I cant imagine allowing this, but
        // perhaps someone will see a need to allow it. -b.t.)
        QString id = root.attribute("classfield_id","-1");
        CodeClassField * newCF = nullptr;
        ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(getParentDocument());
        if (cdoc)
                newCF = cdoc->findCodeClassFieldFromParentID (Uml::ID::fromString(id));

        m_parentclassfield->disconnect(this); // always disconnect
        if (newCF)
                initFields(newCF);
        else
                logError0("code accessor method cant load parent codeclassfield, corrupt file?");

    */
    // now load/set other local attributes
    setType((AccessorType)root.attribute(QStringLiteral("accessType"),QStringLiteral("0")).toInt());
}

/**
 * Set the class attributes from a passed object.
 */
void CodeAccessorMethod::setAttributesFromObject(TextBlock * obj)
{
    CodeMethodBlock::setAttributesFromObject(obj);

    CodeAccessorMethod * mb = dynamic_cast<CodeAccessorMethod*>(obj);
    if (mb) {
        m_parentclassfield->disconnect(this); // always disconnect

        initFields(mb->getParentClassField());

        setType(mb->getType());
    }
}

void CodeAccessorMethod::initFields(CodeClassField * parentClassField)
{
    m_parentclassfield = parentClassField;
    m_accessorType = GET;
    setCanDelete(false);  // we cant delete these with the codeeditor, delete the UML operation instead.

    connect(m_parentclassfield, SIGNAL(modified()), this, SLOT(syncToParent()));
}

