/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdremoveumlobject.h"

// app includes
#include "uml.h"
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
