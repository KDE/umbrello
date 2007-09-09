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

#include "cmd_set_name.h"

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

    cmdSetName::cmdSetName(UMLObject * _UMLObj, const QString& _name):UMLObj(_UMLObj), name (_name)
    {
        oldname = _UMLObj->getName();
    }

    cmdSetName::~cmdSetName()
    {

    }

    void cmdSetName::redo()
    {
        UMLObj->setNamecmd(name);
    }

    void cmdSetName::undo()
    {
        UMLObj->setNamecmd(oldname);
    }

}
