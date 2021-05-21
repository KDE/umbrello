/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdchangelinecolor.h"

// app includes
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    CmdChangeLineColor::CmdChangeLineColor(UMLWidget* widget, const QColor& col)
      : CmdBaseWidgetCommand(widget),
        m_newColor(col)
    {
        setText(i18n("Change line color : %1", widget->name()));

        m_oldColor = widget->lineColor() ;
        m_oldUsesDiagramValue = widget->usesDiagramLineColor();
    }

    CmdChangeLineColor::~CmdChangeLineColor()
    {
    }

    void CmdChangeLineColor::redo()
    {
        widget()->setLineColorCmd(m_newColor);
    }

    void CmdChangeLineColor::undo()
    {
        UMLWidget* umlWidget = widget();
        umlWidget->setLineColorCmd(m_oldColor);
        umlWidget->setUsesDiagramLineColor(m_oldUsesDiagramValue);
    }

}
