/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_create_umlobject.h"

// app includes
#include "uml.h"
#include "umldoc.h"
#include "package.h"
#include "umlobject.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdCreateUMLObject::CmdCreateUMLObject(UMLObject* o)
      : m_obj(o)
    {
        setText(i18n("Create uml object : %1", o->fullyQualifiedName()));
    }

    CmdCreateUMLObject::~CmdCreateUMLObject()
    {
    }

    // Create the UMLObject
    void CmdCreateUMLObject::redo()
    {
        UMLDoc *doc = UMLApp::app()->document();

        // This object was removed from it's package when it was deleted
        // so add it back to it's package ( if it belonged to one )
        UMLPackage *pkg = m_obj->umlPackage();
        if (pkg == NULL) {
          // object does not belong to any package

        } else {
          // add this object to its parent package
          pkg->addObject(m_obj);
        }

        doc->signalUMLObjectCreated(m_obj);
    }

    // Suppress the UMLObject
    void CmdCreateUMLObject::undo()
    {
        UMLDoc *doc = UMLApp::app()->document();
        doc->removeUMLObject(m_obj);
    }

}
