/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2005 Richard Dale <Richard_Dale@tipitina.demon.co.uk>
    SPDX-FileCopyrightText: 2006-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
