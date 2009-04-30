/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_set_stereotype.h"

// app includes
#include "umlobject.h"

#include <klocale.h>

namespace Uml
{

    CmdSetStereotype::CmdSetStereotype(UMLObject * obj, const QString& stereo)
        : m_stereo(stereo), m_umlObject(obj)
    {
        m_oldStereo = obj->getStereotype();
    }

    CmdSetStereotype::~CmdSetStereotype()
    {
    }

    void CmdSetStereotype::redo()
    {
        m_umlObject->setStereotypecmd(m_stereo);
    }

    void CmdSetStereotype::undo()
    {
        m_umlObject->setStereotypecmd(m_oldStereo);
    }

}
