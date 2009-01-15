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

#include "cmd_set_txt.h"

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
#include <kdebug.h>

namespace Uml
{

    CmdSetTxt::CmdSetTxt(FloatingTextWidget* _ftw, const QString& txt):ftw(_ftw),newstring(txt)
    {

        oldstring = _ftw->getText();
        uDebug() << "oldstring: "<< oldstring << ", newstring: "<< newstring;
    }

    CmdSetTxt::~CmdSetTxt()
    {

    }

    void CmdSetTxt::redo()
    {

        ftw->setTextcmd(newstring);
        uDebug() << "string after redo" << ftw->getText()<< ", oldstring: "<< oldstring << ", newstring: "<< newstring;
    }

    void CmdSetTxt::undo()
    {
        ftw->setName("balbalbalbalbla");
        ftw->setTextcmd(oldstring);
        uDebug() << "string after undo: " << ftw->getText()<< "oldstring: "<< oldstring << "newstring: "<< newstring;
    }

}
