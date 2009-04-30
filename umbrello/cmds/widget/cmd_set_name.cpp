/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_set_name.h"

// app includes
#include "umlobject.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdSetName::CmdSetName(UMLObject * obj, const QString& name)
      : m_umlObject(obj), m_name (name)
    {
        m_oldname = obj->getName();
    }

    CmdSetName::~CmdSetName()
    {
    }

    void CmdSetName::redo()
    {
        m_umlObject->setNamecmd(m_name);
    }

    void CmdSetName::undo()
    {
        m_umlObject->setNamecmd(m_oldname);
    }

}
