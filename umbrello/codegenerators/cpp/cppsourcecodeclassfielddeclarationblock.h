/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2003      Brian Thomas <thomas@mail630.gsfc.nasa.gov>   *
 *   copyright (C) 2003-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CPPSOURCECODECLASSFIELDDECLARATIONBLOCK_H
#define CPPSOURCECODECLASSFIELDDECLARATIONBLOCK_H

#include "codeclassfielddeclarationblock.h"

#include <QtCore/QString>

class CPPSourceCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    CPPSourceCodeClassFieldDeclarationBlock ( CodeClassField * parent );

    /**
     * Empty Destructor
     */
    virtual ~CPPSourceCodeClassFieldDeclarationBlock ( );

    /**
     * This will be called by syncToParent whenever the parent object is "modified"
     */
    void updateContent ( );

private:

};

#endif // CPPSOURCECODECLASSFIELDDECLARATIONBLOCK_H
