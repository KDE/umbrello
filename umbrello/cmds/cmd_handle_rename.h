/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_HANDLE_RENAME_H
#define CMD_HANDLE_RENAME_H

#include <QtGui/QUndoCommand>

class FloatingTextWidget;

namespace Uml
{
    class CmdHandleRename : public QUndoCommand
    {
        public:
            CmdHandleRename(FloatingTextWidget* ftw, QString& txt);
            ~CmdHandleRename();

            void redo();
            void undo();

        private:
            FloatingTextWidget*  m_ftw;
            QString     m_newstring;
            QString     m_oldstring;
    };
}

#endif
