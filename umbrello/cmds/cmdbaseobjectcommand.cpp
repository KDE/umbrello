/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2007-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdbaseobjectcommand.h"

// app includes
#include "umlapp.h"
#include "umldoc.h"
#include "umlobject.h"

// kde includes
#include <KLocalizedString>

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

        m_object = object;
        m_objectId = object->id();
    }

    UMLObject* CmdBaseObjectCommand::object()
    {
        UMLDoc *doc = UMLApp::app()->document();
        UMLObject *umlObject = doc->findObjectById(m_objectId);

        if (!umlObject)
            umlObject = m_object;

        return umlObject;
    }
}
