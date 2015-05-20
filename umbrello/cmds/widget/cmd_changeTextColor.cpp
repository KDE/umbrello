/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_changeTextColor.h"

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
