/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDHANDLERENAME_H
#define CMDHANDLERENAME_H

#include <QUndoCommand>

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
