/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef RUBYCODECLASSFIELDDECLARATIONBLOCK_H
#define RUBYCODECLASSFIELDDECLARATIONBLOCK_H

#include "codeclassfielddeclarationblock.h"

#include <QtCore/QString>

class RubyCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    RubyCodeClassFieldDeclarationBlock ( CodeClassField * parent );

    /**
     * Empty Destructor
     */
    virtual ~RubyCodeClassFieldDeclarationBlock ( );

    /**
     * This will be called by syncToParent whenever the parent object is "modified"
     */
    void updateContent ( );

private:

};

#endif // RUBYCODECLASSFIELDDECLARATIONBLOCK_H
