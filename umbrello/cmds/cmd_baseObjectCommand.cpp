/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_baseObjectCommand.h"

// app includes
#include "uml.h"
#include "umldoc.h"
#include "umlobject.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdBaseObjectCommand::CmdBaseObjectCommand(UMLObject* object)
    {
        setObject(object);
    }

    CmdBaseObjectCommand::~CmdBaseObjectCommand()
    {
    }

    void CmdBaseObjectCommand::setObject(UMLObject* object)
    {
        Q_ASSERT(object);

        m_objectId = object->id();
    }

    UMLObject* CmdBaseObjectCommand::object()
    {
        UMLDoc *doc = UMLApp::app()->document();
        UMLObject *umlObject = doc->findObjectById(m_objectId);

        //Q_ASSERT(umlObject);

        return umlObject;
    }
}
