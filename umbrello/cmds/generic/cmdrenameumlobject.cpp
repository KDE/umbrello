/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdrenameumlobject.h"

// app includes
#include "umlobject.h"

// kde includes
#include <KLocalizedString>

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
