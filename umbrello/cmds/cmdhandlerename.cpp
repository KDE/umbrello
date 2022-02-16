/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdhandlerename.h"

#include "floatingtextwidget.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    CmdHandleRename::CmdHandleRename(FloatingTextWidget* ftw, QString& txt)
      : QUndoCommand(),
        m_ftw(ftw),
        m_newstring(txt)
    {
        m_oldstring = ftw->text();
        setText(i18n("Change text : %1 to %2", m_oldstring, txt));
    }

    CmdHandleRename::~CmdHandleRename()
    {
    }

    void CmdHandleRename::redo()
    {
        m_ftw->changeName(m_newstring);
    }

    void CmdHandleRename::undo()
    {
        m_ftw->changeName(m_oldstring);
    }

}
