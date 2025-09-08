/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdremoveumlobject.h"

// app includes
#include "umlapp.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    /**
     * Constructor.
     */
    CmdRemoveUMLObject::CmdRemoveUMLObject(UMLObject* o)
      : QUndoCommand(),
        m_obj(o)
    {
        setText(i18n("Remove UML object : %1", m_obj->fullyQualifiedName()));
    }

    /**
     *  Destructor.
     */
    CmdRemoveUMLObject::~CmdRemoveUMLObject()
    {
    }

    /**
     * Remove the UMLObject.
     */
    void CmdRemoveUMLObject::redo()
    {
        UMLDoc *doc = UMLApp::app()->document();
        doc->removeUMLObject(m_obj);
    }

    /**
     * Suppress the UMLObject.
     */
    void CmdRemoveUMLObject::undo()
    {
        UMLDoc *doc = UMLApp::app()->document();
        doc->addUMLObject(m_obj);
        doc->signalUMLObjectCreated(m_obj);
    }

}
