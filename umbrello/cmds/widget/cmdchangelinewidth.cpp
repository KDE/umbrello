/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdchangelinewidth.h"

// app includes
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    CmdChangeLineWidth::CmdChangeLineWidth(UMLWidget* widget, const uint width)
      : CmdBaseWidgetCommand(widget),
        m_newWidth(width)
    {
        setText(i18n("Change line width : %1", widget->name()));

        m_oldWidth = widget->lineWidth() ;
    }

    CmdChangeLineWidth::~CmdChangeLineWidth()
    {
    }

    void CmdChangeLineWidth::redo()
    {
        widget()->setLineWidthCmd(m_newWidth);
    }

    void CmdChangeLineWidth::undo()
    {
        widget()->setLineWidthCmd(m_oldWidth);
    }
}
