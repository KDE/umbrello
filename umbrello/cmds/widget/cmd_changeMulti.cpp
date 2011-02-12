/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

 /* Created by Bouchikhi Mohamed-Amine */

#include "cmd_changeMulti.h"

// app includes
#include "umlrole.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeMulti::CmdChangeMulti(UMLRole *role, const QString &multi)
      : m_umlRole(role), m_newMulti(multi)
    {
        setText(i18n("Change multiplicity : %1 to %2", role->name(), multi));
        m_oldMulti = m_umlRole->multiplicity();
    }

    void CmdChangeMulti::undo()
    {
        if (!m_oldMulti.isEmpty()) {
            m_umlRole->setMultiplicity(m_oldMulti);
        }
        else {
            m_umlRole->setMultiplicity("");
        }
    }

    void CmdChangeMulti::redo()
    {
        m_umlRole->setMultiplicity(m_newMulti);
    }
}

/* line to add the commande in the undo/redo list :
UMLApp::app()->executeCommand(new CmdChangeMulti(UMLRole role, QString newMulti));
*/
