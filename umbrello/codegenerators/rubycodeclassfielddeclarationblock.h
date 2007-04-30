/***************************************************************************
                          rubycodeclassfielddeclarationblock.cpp
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

#ifndef RUBYCODECLASSFIELDDECLARATIONBLOCK_H
#define RUBYCODECLASSFIELDDECLARATIONBLOCK_H

#include <qstring.h>

#include "../codeclassfielddeclarationblock.h"

class RubyCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    // Constructors/Destructors
    //

    /**
     * Constructor
     */
    RubyCodeClassFieldDeclarationBlock ( CodeClassField * parent );

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeClassFieldDeclarationBlock ( );

protected:

    // this will be called by syncToParent whenever the parent object is "modified"
    void updateContent ( );

private:


};

#endif // RUBYCODECLASSFIELDDECLARATIONBLOCK_H
