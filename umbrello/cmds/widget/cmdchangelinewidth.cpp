/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
