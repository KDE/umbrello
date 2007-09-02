
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007 Jari-Matti Mäkelä <jmjm@iki.fi>                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef DCODEACCESSORMETHOD_H
#define DCODEACCESSORMETHOD_H

#include <qstring.h>

#include "../codeaccessormethod.h"

class CodeClassField;

class DCodeAccessorMethod : public CodeAccessorMethod
{
    Q_OBJECT
public:

    // Constructors/Destructors
    //

    /**
     * Constructor
     */
    DCodeAccessorMethod ( CodeClassField * field, CodeAccessorMethod::AccessorType type);

    /**
     * Empty Destructor
     */
    virtual ~DCodeAccessorMethod ( );

    /**
     * Must be called before this object is usable
     */
    void update();

    virtual void updateMethodDeclaration();
    virtual void updateContent();

protected:

    /** set attributes of the node that represents this class
     * in the XMI document.
     */
    virtual void setAttributesOnNode ( QDomDocument & doc, QDomElement & blockElement);

    /** set the class attributes of this object from
     * the passed element node.
     */
    virtual void setAttributesFromNode ( QDomElement & element);

};

#endif // DCODEACCESSORMETHOD_H
