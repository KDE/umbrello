/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#include "cmd_changeFillColor.h"

#include "umlwidget.h"

// kde includes
#include <klocale.h>

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
