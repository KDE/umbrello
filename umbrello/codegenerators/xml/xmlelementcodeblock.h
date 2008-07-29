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

#ifndef XMLELEMENTCODEBLOCK_H
#define XMLELEMENTCODEBLOCK_H

#include "hierarchicalcodeblock.h"
#include "umlattributelist.h"

#include <QtCore/QString>

class CodeDocument;
class UMLAttribute;

class XMLElementCodeBlock : public HierarchicalCodeBlock
{
public:

    /**
     * Empty Constructor
     */
    XMLElementCodeBlock ( CodeDocument * parentDoc, const QString & nodeName, const QString & comment = "");

    /**
     * Empty Destructor
     */
    virtual ~XMLElementCodeBlock ( );

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI ( QDomDocument & doc, QDomElement & root );

    /**
     * Load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI ( QDomElement & root );

    virtual UMLAttributeList * getAttributeList();

    virtual void setNodeName (const QString &name);
    virtual QString getNodeName ();

    void addAttribute (UMLAttribute * at);

    /**
     * Update the start/end text of this codeblock.
     */
    void updateContent ( );

protected:

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

private:

    UMLAttributeList m_attList;
    QString m_nodeName;
    void init (CodeDocument * parent, const QString &nodeName, const QString &comment);

};

#endif // XMLELEMENTCODEBLOCK_H
