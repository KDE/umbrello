/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDRENAMEUMLOBJECT_H
#define CMDRENAMEUMLOBJECT_H

#include <QUndoCommand>

class UMLObject;

namespace Uml
{
    class CmdRenameUMLObject : public QUndoCommand
    {
        public:
            CmdRenameUMLObject(UMLObject* o, const QString& name);
            ~CmdRenameUMLObject();

            void redo();
            void undo();

        private:
            UMLObject*  m_obj;
            QString     m_name;
            QString     m_oldname;
    };
}

#endif
