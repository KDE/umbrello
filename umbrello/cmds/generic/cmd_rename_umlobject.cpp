/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_rename_umlobject.h"

// app includes
#include "umlobject.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdRenameUMLObject::CmdRenameUMLObject(UMLObject* o, const QString& name)
      : m_obj(o), m_name(name)
    {
        setText(i18n("Rename object"));
        m_oldname = o->getName();
    }

    CmdRenameUMLObject::~CmdRenameUMLObject()
    {
    }

    void CmdRenameUMLObject::redo()
    {
        m_obj->setNamecmd(m_name);
    }

    void CmdRenameUMLObject::undo()
    {
        m_obj->setNamecmd(m_oldname);
    }

}
