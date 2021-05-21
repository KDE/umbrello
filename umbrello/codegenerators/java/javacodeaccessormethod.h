/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODEACCESSORMETHOD_H
#define JAVACODEACCESSORMETHOD_H

#include "codeaccessormethod.h"

#include <QString>

class CodeClassField;

class JavaCodeAccessorMethod : public CodeAccessorMethod
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    JavaCodeAccessorMethod (CodeClassField * field, CodeAccessorMethod::AccessorType type);

    /**
     * Empty Destructor
     */
    virtual ~JavaCodeAccessorMethod ();

    /**
     * Must be called before this object is usable.
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

#endif // JAVACODEACCESSORMETHOD_H
