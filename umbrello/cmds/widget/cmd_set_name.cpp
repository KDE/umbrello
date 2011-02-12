/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
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
      : m_umlObject(obj), m_name(name)
    {
        setText(i18n("Set name : %1 to %2", obj->name(), name));
        m_oldname = obj->name();
    }

    CmdSetName::~CmdSetName()
    {
    }

    void CmdSetName::redo()
    {
        m_umlObject->setNameCmd(m_name);
    }

    void CmdSetName::undo()
    {
        m_umlObject->setNameCmd(m_oldname);
    }

}
