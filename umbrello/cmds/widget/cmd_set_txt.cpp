/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_set_txt.h"

// app includes
#include "debug_utils.h"
#include "floatingtextwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdSetTxt::CmdSetTxt(FloatingTextWidget* ftw, const QString& txt)
      : m_ftw(ftw), m_newstring(txt)
    {
        setText(i18n("Set text : %1 to %2", ftw->name(), txt));
        m_oldstring = ftw->text();
        uDebug() << "oldstring: "<< m_oldstring << ", newstring: "<< m_newstring;
    }

    CmdSetTxt::~CmdSetTxt()
    {
    }

    void CmdSetTxt::redo()
    {
        m_ftw->setTextcmd(m_newstring);
        uDebug() << "string after redo" << m_ftw->text()
            << ", oldstring: "<< m_oldstring << ", newstring: " << m_newstring;
    }

    void CmdSetTxt::undo()
    {
        m_ftw->setName("balbalbalbalbla");
        m_ftw->setTextcmd(m_oldstring);
        uDebug() << "string after undo: " << m_ftw->text()
            << "oldstring: "<< m_oldstring << "newstring: "<< m_newstring;
    }

}
