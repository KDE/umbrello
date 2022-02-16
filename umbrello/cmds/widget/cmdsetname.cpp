/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdsetname.h"

// app includes
#include "umlobject.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    CmdSetName::CmdSetName(UMLObject * obj, const QString& name)
      : CmdBaseObjectCommand(obj),
        m_name(name)
    {
        setText(i18n("Set name : %1 to %2", obj->name(), name));
        m_oldname = obj->name();
    }

    CmdSetName::~CmdSetName()
    {
    }

    void CmdSetName::redo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setNameCmd(m_name);
    }

    void CmdSetName::undo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setNameCmd(m_oldname);
    }

}
