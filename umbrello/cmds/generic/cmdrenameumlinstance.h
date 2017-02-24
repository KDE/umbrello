/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2016                                              *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_RENAME_UMLINSTANCE_H
#define CMD_RENAME_UMLNSTANCE_H

#include <QUndoCommand>

class UMLInstance;

namespace Uml
{
    class CmdRenameUMLInstance : public QUndoCommand
    {
        public:
            CmdRenameUMLInstance(UMLInstance *o, const QString& name);
            ~CmdRenameUMLInstance();

            void redo();
            void undo();

        private:
            UMLInstance*  m_obj;
            QString     m_name;
            QString     m_oldname;
    };
}

#endif
