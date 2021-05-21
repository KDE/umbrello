/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODEACCESSORMETHOD_H
#define DCODEACCESSORMETHOD_H

#include "codeaccessormethod.h"

#include <QString>

class CodeClassField;

class DCodeAccessorMethod : public CodeAccessorMethod
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    DCodeAccessorMethod (CodeClassField * field, CodeAccessorMethod::AccessorType type);

    /**
     * Empty Destructor
     */
    virtual ~DCodeAccessorMethod ();

    /**
     * Must be called before this object is usable
     */
    void update();

    virtual void updateMethodDeclaration();
    virtual void updateContent();

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

};

#endif // DCODEACCESSORMETHOD_H
