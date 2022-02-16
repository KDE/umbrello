/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef JAVACODECLASSFIELDDECLARATIONBLOCK_H
#define JAVACODECLASSFIELDDECLARATIONBLOCK_H

#include "codeclassfielddeclarationblock.h"

#include <QString>

class JavaCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    explicit JavaCodeClassFieldDeclarationBlock(CodeClassField* parent);
    virtual ~JavaCodeClassFieldDeclarationBlock();

    void updateContent();

private:


};

#endif // JAVACODECLASSFIELDDECLARATIONBLOCK_H
