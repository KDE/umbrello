/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_handle_rename.h"

// kde includes
#include <klocale.h>
#include <kdebug.h>

namespace Uml
{

    CmdHandleRename::CmdHandleRename(FloatingTextWidget* _ftw, QString& txt)
      : ftw(_ftw), newstring(txt)
    {
        setText(i18n("Change text"));
        oldstring = _ftw->text();
    }

    CmdHandleRename::~CmdHandleRename()
    {
    }

    void CmdHandleRename::redo()
    {
        ftw->changeName(newstring);
    }

    void CmdHandleRename::undo()
    {
        ftw->changeName(oldstring);
    }

}
