/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdcreateumlobject.h"

// app includes
#include "debug_utils.h"
#include "package.h"
#include "uml.h"
#include "umldoc.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    /**
     * Constructor.
     */
    CmdCreateUMLObject::CmdCreateUMLObject(UMLObject* o)
      : QUndoCommand(),
        m_obj(o),
        m_skipSignal(true)
    {
        setText(i18n("Create UML object : %1", m_obj->fullyQualifiedName()));

        m_package = m_obj->umlPackage();
        m_type    = m_obj->baseType();
        m_name    = m_obj->name();
    }

    /**
     *  Destructor.
     */
    CmdCreateUMLObject::~CmdCreateUMLObject()
    {
    }

    /**
     * Create the UMLObject.
     */
    void CmdCreateUMLObject::redo()
    {
        // This object was removed from its package when it was deleted
        // so add it back to its package (if it belonged to one)
        if (m_package) {
            Q_ASSERT(m_package->baseType() != UMLObject::ot_Association);
            // add this object to its parent package
            m_package->addObject(m_obj);
        } else {
            logError1("CmdCreateUMLObject::redo: UMLPackage of %1 is not set", m_obj->name());
        }

        // The first call to redo, the object was created and signalled by the
        // caller (umlscene). On subsequent calls we use the "umlobject created"
        // signal to update Umbrello with the re-added object.
        if (m_skipSignal) {
            m_skipSignal = false;
        } else {
            UMLDoc *doc = UMLApp::app()->document();
            doc->signalUMLObjectCreated(m_obj);
        }
    }

    /**
     * Suppress the UMLObject.
     */
    void CmdCreateUMLObject::undo()
    {
        UMLDoc *doc = UMLApp::app()->document();
        doc->removeUMLObject(m_obj);
    }

}
