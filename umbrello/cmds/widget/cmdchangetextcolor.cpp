/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdchangetextcolor.h"

// app includes
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{
    CmdChangeTextColor::CmdChangeTextColor(UMLWidget* widget, const QColor& col)
      : CmdBaseWidgetCommand(widget),
        m_newColor(col)
    {
        setText(i18n("Change text color : %1", widget->name()));

        m_oldColor = widget->textColor() ;
        m_oldUsesDiagramValue = widget->usesDiagramTextColor();
    }

    CmdChangeTextColor::~CmdChangeTextColor()
    {
    }

    void CmdChangeTextColor::redo()
    {
        widget()->setTextColorCmd(m_newColor);
    }

    void CmdChangeTextColor::undo()
    {
        UMLWidget* umlWidget = widget();
        umlWidget->setTextColorCmd(m_oldColor);
        umlWidget->setUsesDiagramTextColor(m_oldUsesDiagramValue);
    }

}
