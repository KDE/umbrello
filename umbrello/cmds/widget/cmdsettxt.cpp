/*
    SPDX-License-Identifier: GPL-2.0-or-later

    copyright (C) 2002-2014
    Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdsettxt.h"

// app includes
#include "debug_utils.h"
#include "floatingtextwidget.h"

// kde includes
#include <KLocalizedString>

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
        m_ftw->setName(QLatin1String("balbalbalbalbla"));
        m_ftw->setTextcmd(m_oldstring);
        uDebug() << "string after undo: " << m_ftw->text()
            << "oldstring: "<< m_oldstring << "newstring: "<< m_newstring;
    }

}
