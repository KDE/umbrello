/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_setVisibility.h"

// app includes
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"

#include <klocale.h>

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
