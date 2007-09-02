
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

#ifndef DCODECLASSFIELDDECLARATIONBLOCK_H
#define DCODECLASSFIELDDECLARATIONBLOCK_H

#include <qstring.h>

#include "../codeclassfielddeclarationblock.h"

class DCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    // Constructors/Destructors
    //

    /**
     * Constructor
     */
    DCodeClassFieldDeclarationBlock ( CodeClassField * parent );

    /**
     * Empty Destructor
     */
    virtual ~DCodeClassFieldDeclarationBlock ( );

    /**
     * This will be called by syncToParent whenever the parent object is "modified"
     */
    void updateContent ( );

private:


};

#endif // DCODECLASSFIELDDECLARATIONBLOCK_H
