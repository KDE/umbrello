/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDREMOVEUMLOBJECT_H
#define CMDREMOVEUMLOBJECT_H

#include "umlobject.h"
#include <QUndoCommand>

namespace Uml
{
    class CmdRemoveUMLObject : public QUndoCommand
    {
    public:
        explicit CmdRemoveUMLObject(UMLObject* o);
        ~CmdRemoveUMLObject();

        void redo();
        void undo();

    private:
        UMLObject             *m_obj;
    };
}

#endif
