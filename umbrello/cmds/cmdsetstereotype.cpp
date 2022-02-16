/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdsetstereotype.h"

// app includes
#include "umlobject.h"

#include <KLocalizedString>

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
