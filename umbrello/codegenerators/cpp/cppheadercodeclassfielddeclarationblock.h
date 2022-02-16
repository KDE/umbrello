/*
    SPDX-License-Identifier: GPL-2.0-or-later

    SPDX-FileCopyrightText: 2003 Brian Thomas <thomas@mail630.gsfc.nasa.gov>
    SPDX-FileCopyrightText: 2004-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CPPHEADERCODECLASSFIELDDECLARATIONBLOCK_H
#define CPPHEADERCODECLASSFIELDDECLARATIONBLOCK_H

#include "codeclassfielddeclarationblock.h"

#include <QString>

class CPPHeaderCodeClassFieldDeclarationBlock : public CodeClassFieldDeclarationBlock
{
    Q_OBJECT
public:

    /**
     * Constructor
     */
    explicit CPPHeaderCodeClassFieldDeclarationBlock (CodeClassField * parent);

    /**
     * Empty Destructor
     */
    virtual ~CPPHeaderCodeClassFieldDeclarationBlock ();

    /**
     * This will be called by syncToParent whenever the parent object is "modified"
     */
    void updateContent ();

private:


};

#endif // CPPHEADERCODECLASSFIELDDECLARATIONBLOCK_H
