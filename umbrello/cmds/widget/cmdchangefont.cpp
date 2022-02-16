/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2007-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdchangefont.h"

// app includes
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{
    CmdChangeFont::CmdChangeFont(UMLWidget* widget, QFont font)
        : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Change font : %1", widget->name()));

        m_newFont = font;
        m_oldFont = widget->font();
    }

    void CmdChangeFont::undo()
    {
        widget()->setFontCmd(m_oldFont);
    }

    void CmdChangeFont::redo()
    {
        widget()->setFontCmd(m_newFont);
    }

}
