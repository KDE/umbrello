/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2005                                                    *
 *   Richard Dale  <Richard_Dale@tipitina.demon.co.uk>                     *
 *   copyright (C) 2006-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef RUBYCODECLASSFIELDDECLARATIONBLOCK_H
#define RUBYCODECLASSFIELDDECLARATIONBLOCK_H

#include "codeclassfielddeclarationblock.h"

#include <QString>

class RubyCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    explicit RubyCodeClassFieldDeclarationBlock(CodeClassField * parent);
    virtual ~RubyCodeClassFieldDeclarationBlock();

    void updateContent();

private:

};

#endif // RUBYCODECLASSFIELDDECLARATIONBLOCK_H
