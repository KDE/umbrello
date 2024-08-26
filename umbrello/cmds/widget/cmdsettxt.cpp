/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdsettxt.h"

// app includes
#define DBG_SRC QStringLiteral("CmdSetTxt")
#include "debug_utils.h"
#include "floatingtextwidget.h"
#include "uml.h"

// kde includes
#include <KLocalizedString>

DEBUG_REGISTER_DISABLED(CmdSetTxt)

namespace Uml
{

    CmdSetTxt::CmdSetTxt(FloatingTextWidget* ftw, const QString& txt)
      : m_ftw(ftw), m_newstring(txt)
    {
        setText(i18n("Set text : %1 to %2", ftw->name(), txt));
        m_oldstring = ftw->text();
        logDebug2("CmdSetTxt oldstring: %1, newstring: %2", m_oldstring, m_newstring);
    }

    CmdSetTxt::~CmdSetTxt()
    {
    }

    void CmdSetTxt::redo()
    {
        m_ftw->setTextcmd(m_newstring);
        logDebug3("CmdSetTxt::redo: string after redo %1, oldstring: %2, newstring: %3",
                  m_ftw->text(), m_oldstring, m_newstring);
    }

    void CmdSetTxt::undo()
    {
        m_ftw->setName(QStringLiteral("balbalbalbalbla"));
        m_ftw->setTextcmd(m_oldstring);
        logDebug3("CmdSetTxt::undo: string after undo: %1, oldstring: %2, newstring: %3",
                  m_ftw->text(), m_oldstring, m_newstring);
    }

}
