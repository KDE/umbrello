/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
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
        setText(i18n("Rename object : %1 to %2", o->name(), name));
        m_oldname = o->name();
    }

    CmdRenameUMLObject::~CmdRenameUMLObject()
    {
    }

    void CmdRenameUMLObject::redo()
    {
        m_obj->setNameCmd(m_name);
    }

    void CmdRenameUMLObject::undo()
    {
        m_obj->setNameCmd(m_oldname);
    }

}
