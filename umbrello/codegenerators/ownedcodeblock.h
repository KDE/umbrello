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

#ifndef OWNEDCODEBLOCK_H
#define OWNEDCODEBLOCK_H

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtCore/QObject>

class TextBlock;
class UMLObject;

/**
 * Describes any codeblock which is 'owned' by a UMLobject of some sort and should
 * be in sync with that parent.
 */
class OwnedCodeBlock : virtual public QObject
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    OwnedCodeBlock ( UMLObject * parent );

    /**
     * Empty Destructor
     */
    virtual ~OwnedCodeBlock ( );

    /**
     * Get the value of m_parentObject.
     * @return the value of m_parentObject
     */
    UMLObject * getParentObject ( );

    /**
     * This is the method called from within syncToParent
     */
    virtual void updateContent ( ) = 0;

protected:

    /**
     * Causes the text block to release all of its connections
     * and any other text blocks that it 'owns'.
     * Needed to be called prior to deletion of the textblock.
     */
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
     * Set the class attributes from a passed object.
     */
    virtual void setAttributesFromObject (TextBlock * obj);

private:

    void initFields ( UMLObject * parent );

    UMLObject * m_parentObject;

public slots:

    virtual void syncToParent ( );

};

#endif // OWNEDCODEBLOCK_H
