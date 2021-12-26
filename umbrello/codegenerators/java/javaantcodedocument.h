/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVAANTCODEDOCUMENT_H
#define JAVAANTCODEDOCUMENT_H

#include "codedocument.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QXmlStreamWriter>

/**
 * class JavaANTCodeDocument
 * Represents
 */
class JavaANTCodeDocument : public CodeDocument
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    JavaANTCodeDocument ();

    /**
     * Empty Destructor
     */
    virtual ~JavaANTCodeDocument ();

    QString getPath () const;

    void updateContent();

    /**
     * Save the XMI representation of this object.
     */
    virtual void saveToXMI(QXmlStreamWriter& writer);

    /**
     * Load params from the appropriate XMI element node.
     */
    virtual void loadFromXMI (QDomElement & root);

    /**
     * Create a new HierarchicalCodeBlock object belonging to this CodeDocument.
     * @return      HierarchicalCodeBlock
     */
    virtual HierarchicalCodeBlock * newHierarchicalCodeBlock ();

    //**
    // * Create a new CodeBlockWithComments object belonging to this CodeDocument.
    // * @return      CodeBlockWithComments
    // */
    //  virtual CodeBlockWithComments * newCodeBlockWithComments ();

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

    /**
     * Need to overwrite this for java since we need to pick up the
     * xml declaration blocks.
     */
    virtual void loadChildTextBlocksFromNode (QDomElement & root);

private:

};

#endif // JAVAANTCODEDOCUMENT_H
