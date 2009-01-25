/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#include "cmd_changeFillColor.h"

// app includes
#include "uml.h"
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"

#include <klocale.h>

namespace Uml
{
    CmdChangeFillColor::CmdChangeFillColor(UMLWidget *w, const QColor& col):UMLw(w),color(col)
    {
        setText(i18n("Change Fill Color")+ w->getName());
        oldColor= w -> getFillColor() ;
    }

    CmdChangeFillColor::~CmdChangeFillColor()
    {
    }

    void CmdChangeFillColor::redo()
    {
        UMLw -> setFillColourcmd( color );
    }

    void CmdChangeFillColor::undo()
    {

        UMLw -> setFillColourcmd( oldColor );
    }
}
