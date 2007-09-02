/***************************************************************************
                          rubycodeaccessormethod.cpp
                          Derived from the Java code generator by thomas

    begin                : Thur Jul 21 2005
    author               : Richard Dale
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODEACCESSORMETHOD_H
#define RUBYCODEACCESSORMETHOD_H

#include <qstring.h>

#include "../codeaccessormethod.h"
#include "rubycodeclassfield.h"

class RubyCodeAccessorMethod : public CodeAccessorMethod
{
    Q_OBJECT
public:

    // Constructors/Destructors
    //

    /**
     * Constructor
     */
    RubyCodeAccessorMethod ( CodeClassField * field, CodeAccessorMethod::AccessorType type);

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeAccessorMethod ( );

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

private:

};

#endif // RUBYCODEACCESSORMETHOD_H
