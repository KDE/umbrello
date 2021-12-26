/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef XMLELEMENTCODEBLOCK_H
#define XMLELEMENTCODEBLOCK_H

#include "hierarchicalcodeblock.h"
#include "umlattributelist.h"

#include <QString>

class CodeDocument;
class UMLAttribute;

class XMLElementCodeBlock : public HierarchicalCodeBlock
{
public:

    /**
     * Empty Constructor
     */
    XMLElementCodeBlock (CodeDocument * parentDoc, const QString & nodeName, const QString & comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~XMLElementCodeBlock ();

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI(QXmlStreamWriter& writer);

    /**
     * Load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI (QDomElement & root);

    virtual UMLAttributeList * getAttributeList();

    virtual void setNodeName (const QString &name);
    virtual QString getNodeName ();

    void addAttribute (UMLAttribute * at);

    /**
     * Update the start/end text of this codeblock.
     */
    void updateContent ();

protected:

    /**
     * Set attributes of the node that represents this class
     * in the XMI document.
     */
    virtual void setAttributesOnNode (QXmlStreamWriter& writer);

    /**
     * Set the class attributes of this object from
     * the passed element node.
     */
    virtual void setAttributesFromNode (QDomElement & element);

private:

    UMLAttributeList m_attList;
    QString m_nodeName;
    void init (CodeDocument * parent, const QString &nodeName, const QString &comment);

};

#endif // XMLELEMENTCODEBLOCK_H
