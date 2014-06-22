/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_setStereotype.h"

// app includes
#include "umlobject.h"

#include <klocale.h>

namespace Uml
{

    CmdSetStereotype::CmdSetStereotype(UMLObject * obj, const QString& stereo)
      : CmdBaseObjectCommand(obj),
        m_stereo(stereo)
    {
        m_oldStereo = obj->stereotype();
        setText(i18n("Set stereotype : %1 to %2", m_oldStereo, stereo));
    }

    CmdSetStereotype::~CmdSetStereotype()
    {
    }

    void CmdSetStereotype::redo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setStereotypeCmd(m_stereo);
    }

    void CmdSetStereotype::undo()
    {
        UMLObject *umlObject = object();
        if (umlObject)
            umlObject->setStereotypeCmd(m_oldStereo);
    }

}
