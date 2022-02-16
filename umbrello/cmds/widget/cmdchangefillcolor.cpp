/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#include "cmdchangefillcolor.h"

#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{
    CmdChangeFillColor::CmdChangeFillColor(UMLWidget* widget, const QColor& col)
      : CmdBaseWidgetCommand(widget),
        m_color(col)
    {
        setText(i18n("Change fill color : %1", widget->name()));

        m_oldColor = widget->fillColor();
    }

    CmdChangeFillColor::~CmdChangeFillColor()
    {
    }

    void CmdChangeFillColor::redo()
    {
        widget()->setFillColorCmd(m_color);
    }

    void CmdChangeFillColor::undo()
    {
        widget()->setFillColorCmd(m_oldColor);
    }
}
