/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2020 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

 /* Created by Bouchikhi Mohamed-Amine */

#include "cmdchangemultiplicity.h"

// app includes
#include "umlrole.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{
    CmdChangeMultiplicity::CmdChangeMultiplicity(UMLRole *role, const QString &multi)
      : m_umlRole(role), m_newMulti(multi)
    {
        setText(i18n("Change multiplicity : %1 to %2", role->name(), multi));
        m_oldMulti = m_umlRole->multiplicity();
    }

    void CmdChangeMultiplicity::undo()
    {
        if (!m_oldMulti.isEmpty()) {
            m_umlRole->setMultiplicity(m_oldMulti);
        }
        else {
            m_umlRole->setMultiplicity(QString());
        }
    }

    void CmdChangeMultiplicity::redo()
    {
        m_umlRole->setMultiplicity(m_newMulti);
    }
}

/* line to add the commands in the undo/redo list :
UMLApp::app()->executeCommand(new CmdChangeMulti(UMLRole role, QString newMulti));
*/
