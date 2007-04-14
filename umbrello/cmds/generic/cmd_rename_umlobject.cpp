/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_rename_umlobject.h"

#include "uml.h"
#include "umldoc.h"

#include <klocale.h>

namespace Uml
{

    cmdRenameUMLObject::cmdRenameUMLObject(UMLObject* o, const QString& name):m_obj(o),m_name(name)
    {
        setText(i18n("Rename object"));
        m_oldname = o->getName();
    }

    cmdRenameUMLObject::~cmdRenameUMLObject()
    {
    }

    void cmdRenameUMLObject::redo()
    {
        m_obj->setNamecmd(m_name);
    }

    void cmdRenameUMLObject::undo()
    {
        m_obj->setNamecmd(m_oldname);
    }

}
