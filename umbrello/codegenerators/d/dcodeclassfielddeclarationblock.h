/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2007 Jari-Matti Mäkelä <jmjm@iki.fi>
    SPDX-FileCopyrightText: 2008-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef DCODECLASSFIELDDECLARATIONBLOCK_H
#define DCODECLASSFIELDDECLARATIONBLOCK_H

#include "codeclassfielddeclarationblock.h"

#include <QString>

class DCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    explicit DCodeClassFieldDeclarationBlock(CodeClassField * parent);
    virtual ~DCodeClassFieldDeclarationBlock();

    void updateContent();

};

#endif // DCODECLASSFIELDDECLARATIONBLOCK_H
