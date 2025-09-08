/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdsetvisibility.h"

// app includes
#include "umlapp.h"
#include "umldoc.h"
#include "umlobject.h"

#include <KLocalizedString>

namespace Uml
{

    CmdSetVisibility::CmdSetVisibility(UMLObject * obj, Visibility::Enum visibility)
      : CmdBaseObjectCommand(obj),
        m_visibility(visibility)
    {
        setText(i18n("Change visibility : %1", obj->name()));
        m_oldVisibility = obj->visibility();
    }

    CmdSetVisibility::~CmdSetVisibility()
    {
    }

    void CmdSetVisibility::redo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setVisibilityCmd(m_visibility);
    }

    void CmdSetVisibility::undo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setVisibilityCmd(m_oldVisibility);
    }

}
