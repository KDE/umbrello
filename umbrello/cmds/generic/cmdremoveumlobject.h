/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
