/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef OWNEDHIERARCHICALCODEBLOCK_H
#define OWNEDHIERARCHICALCODEBLOCK_H

#include "hierarchicalcodeblock.h"

class UMLObject;
class CodeDocument;

/**
 * class OwnedHierarchicalCodeBlock
 * A "chunk" of code within the code document that is "owned" by some
 * umlobject. This is an abstract class that should be inherited.
 * Note: keep the inheritance sequence: QObject needs to be first in inheritance list.
 */
class OwnedHierarchicalCodeBlock : public OwnedCodeBlock, public HierarchicalCodeBlock
{
    Q_OBJECT
public:

    /**
     * Constructor with QString so we can create & populate it in
     * one step.
     */
    OwnedHierarchicalCodeBlock (UMLObject * parent, CodeDocument * parentDoc, const QString &start = QString(), const QString &end = QString(), const QString &comment = QString());

    /**
     * Empty Destructor
     */
    virtual ~OwnedHierarchicalCodeBlock ();

    CodeDocument * getParentDocument();

    virtual void updateContent () = 0;

    // these next 2 methods should be supplied by inheriting classes

    /**
     * Save the XMI representation of this object
     */
    virtual void saveToXMI1(QXmlStreamWriter& writer) = 0;

    /**
     * load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI1 (QDomElement & root) = 0;

protected:

    virtual void release ();

    virtual void setAttributesOnNode (QXmlStreamWriter& writer);

    virtual void setAttributesFromNode (QDomElement & element);

    virtual void setAttributesFromObject (TextBlock * obj);

private:

public slots:

    void syncToParent ();

};

#endif // OWNEDHIERARCHICALCODEBLOCK_H
