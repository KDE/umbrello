/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

 /* Created by Bouchikhi Mohamed-Amine */
#include "cmd_changeMulti.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"
#include "umlrole.h"


#include <klocale.h>

namespace Uml
{
    cmdChangeMulti::cmdChangeMulti(UMLRole *role, const QString &multi):UMLr(role),newMulti(multi)
    {
        setText(i18n("Change Multiplicity"));
        oldMulti=UMLr->getMultiplicity();
    }
    void cmdChangeMulti::undo()
    {
        if (!oldMulti.isEmpty())
        {
            UMLr->setMultiplicity(oldMulti);
        }
        else
        {
            UMLr->setMultiplicity("");
        }
    }
    void cmdChangeMulti::redo()
    {
        UMLr->setMultiplicity(newMulti);
    }
}

/* line to add the commande in the undo/redo list :
UMLApp::app()->executeCommand(new cmdChangeMulti(UMLRole role, QString newMulti));
*/
