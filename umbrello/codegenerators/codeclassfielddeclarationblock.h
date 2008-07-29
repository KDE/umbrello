/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2004-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CODECLASSFIELDDECLARATIONBLOCK_H
#define CODECLASSFIELDDECLARATIONBLOCK_H

class UMLObject;
class CodeClassField;

#include "codeblockwithcomments.h"
#include "ownedcodeblock.h"

/**
 * Used to declare classifier fields (e.g. either class attributes or classifier
 * associations) in the code document for any given code classfield. This is a
 * special CodeBlockWithComments which is "sync'd" to the parent CodeClassField.
 * Note: keep the inheritance sequence: QObject needs to be first in inheritance list.
 */
class CodeClassFieldDeclarationBlock : public OwnedCodeBlock, public CodeBlockWithComments
{
    friend class CodeClassField;
    Q_OBJECT
public:

    /**
     * Constructor
     */
    CodeClassFieldDeclarationBlock (CodeClassField * parent);

    /**
     * Empty Destructor
     */
    virtual ~CodeClassFieldDeclarationBlock ( );

    /**
     * Get the value of m_parentclassfield.
     * @return the value of m_parentclassfield
     */
    CodeClassField * getParentClassField ( );

    /**
     * A utility method to get the parent object of the parentCodeClassfield.
     */
    UMLObject * getParentObject ( );

    /**
     * Set the class attributes from a passed object.
     */
    virtual void setAttributesFromObject (TextBlock * obj);

    /**
     * This is called by syncToParent.
     */
    virtual void updateContent() = 0;

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    /**
     * Load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI ( QDomElement & root );

protected:

    virtual void release ();

    /**
     * Set attributes of the node that represents this class
     * in the XMI document.
     */
    virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

    /**
     * Set the class attributes of this object from
     * the passed element node.
     */
    virtual void setAttributesFromNode ( QDomElement & element);

    /**
     * So parent can actually release this block.
     */
    void forceRelease ();

private:

    CodeClassField * m_parentclassfield;
    void init(CodeClassField * parent);

public slots:

    virtual void syncToParent();

};

#endif // CODECLASSFIELDDECLARATIONBLOCK_H
