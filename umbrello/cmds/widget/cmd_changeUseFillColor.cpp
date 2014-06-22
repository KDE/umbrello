/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_changeUseFillColor.h"

#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeUseFillColor::CmdChangeUseFillColor(UMLWidget* widget, bool value)
      : CmdBaseWidgetCommand(widget),
        m_newValue(value)
    {
        if (value) {
            setText(i18n("Use fill color : %1", widget->name()));
        } else {
            setText(i18n("No fill color : %1", widget->name()));
        }

        m_oldValue = widget->useFillColor();
    }

    CmdChangeUseFillColor::~CmdChangeUseFillColor()
    {
    }

    void CmdChangeUseFillColor::redo()
    {
        widget()->setUseFillColorCmd(m_newValue);
    }

    void CmdChangeUseFillColor::undo()
    {
        widget()->setUseFillColorCmd(m_oldValue);
    }
}
