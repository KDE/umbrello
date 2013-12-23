/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2007-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_changeFont.h"

// app includes
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeFont::CmdChangeFont(UMLWidget* widget, QFont font)
    {
        setText(i18n("Change font : %1", widget->name()));

        m_widget = widget;
        m_newFont = font;
        m_oldFont = widget->font();
    }

    void CmdChangeFont::undo()
    {
        m_widget->setFontCmd(m_oldFont);
    }

    void CmdChangeFont::redo()
    {
        m_widget->setFontCmd(m_newFont);
    }

}
