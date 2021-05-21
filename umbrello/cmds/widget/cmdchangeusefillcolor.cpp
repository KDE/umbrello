/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdchangeusefillcolor.h"

#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

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
