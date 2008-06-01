/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2008                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#include "cmd_set_visibility.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "umlnamespace.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "classifierwidget.h"
#include "associationwidget.h"
#include "messagewidget.h"

#include <klocale.h>

namespace Uml
{

    CmdSetVisibility::CmdSetVisibility(UMLObject * _UMLObj, Uml::Visibility _visib)
      : visib(_visib), UMLObj(_UMLObj)
    {
        setText(i18n("Change visibility : ") + _UMLObj->getName());
        oldvisibility = _UMLObj->getVisibility();
    }

    CmdSetVisibility::~CmdSetVisibility()
    {

    }

    void CmdSetVisibility::redo()
    {
        UMLObj->setVisibilitycmd(visib);
    }

    void CmdSetVisibility::undo()
    {
        UMLObj->setVisibilitycmd(oldvisibility);
    }

}
