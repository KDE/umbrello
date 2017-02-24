/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2016                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdrenameumlobjectinstance.h"

// app includes
#include "instance.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    CmdRenameUMLObjectInstance::CmdRenameUMLObjectInstance(UMLInstance* o, const QString& name)
      : m_obj(o), m_name(name)
    {
        setText(i18n("Rename object : %1 to %2", o->instanceName(), name));
        m_oldname = o->instanceName();
    }

    CmdRenameUMLObjectInstance::~CmdRenameUMLObjectInstance()
    {
    }

    void CmdRenameUMLObjectInstance::redo()
    {
        m_obj->setInstanceNameCmd(m_name);
    }

    void CmdRenameUMLObjectInstance::undo()
    {
        m_obj->setInstanceName(m_oldname);
    }

}
