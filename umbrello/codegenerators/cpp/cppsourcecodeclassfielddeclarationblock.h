/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2003-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPSOURCECODECLASSFIELDDECLARATIONBLOCK_H
#define CPPSOURCECODECLASSFIELDDECLARATIONBLOCK_H

#include "codeclassfielddeclarationblock.h"

#include <QString>

class CPPSourceCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    explicit CPPSourceCodeClassFieldDeclarationBlock (CodeClassField * parent);

    /**
     * Empty Destructor
     */
    virtual ~CPPSourceCodeClassFieldDeclarationBlock ();

    /**
     * This will be called by syncToParent whenever the parent object is "modified"
     */
    void updateContent ();

private:

};

#endif // CPPSOURCECODECLASSFIELDDECLARATIONBLOCK_H
