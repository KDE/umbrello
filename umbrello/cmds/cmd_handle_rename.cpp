/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_handle_rename.h"

#include "floatingtextwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdHandleRename::CmdHandleRename(FloatingTextWidget* ftw, QString& txt)
      : m_ftw(ftw), m_newstring(txt)
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
