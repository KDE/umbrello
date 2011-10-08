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
#include "codeaccessormethod.h"

// qt/kde includes

// local includes
#include "codeclassfield.h"

/**
 * Constructors
 */
CodeAccessorMethod::CodeAccessorMethod(CodeClassField * parentCF)
  : CodeMethodBlock ( parentCF->getParentDocument(), parentCF->getParentObject() )
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
void CodeAccessorMethod::saveToXMI(QDomDocument & doc, QDomElement & root)
{
    QDomElement docElement = doc.createElement( "codeaccessormethod" );

    setAttributesOnNode(doc, docElement);

    root.appendChild( docElement );
}

/**
 * Set attributes of the node that represents this class
 * in the XMI document.
 */
void CodeAccessorMethod::setAttributesOnNode(QDomDocument & doc, QDomElement & elem)
{
    // set super-class attributes
    CodeMethodBlock::setAttributesOnNode(doc, elem);

    // set local class attributes
    elem.setAttribute("accessType",getType());
    elem.setAttribute("classfield_id",getParentClassField()->getID());
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
        CodeClassField * newCF = 0;
        ClassifierCodeDocument * cdoc = dynamic_cast<ClassifierCodeDocument*>(getParentDocument());
        if (cdoc)
                newCF = cdoc->findCodeClassFieldFromParentID (STR2ID(id));

        m_parentclassfield->disconnect(this); // always disconnect
        if (newCF)
                initFields(newCF);
        else
                uError()<<"ERROR: code accessor method cant load parent codeclassfield, corrupt file?"<<endl;

    */
    // now load/set other local attributes
    setType((AccessorType)root.attribute("accessType","0").toInt());
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

    connect(m_parentclassfield,SIGNAL(modified()),this,SLOT(syncToParent()));
}

#include "codeaccessormethod.moc"
