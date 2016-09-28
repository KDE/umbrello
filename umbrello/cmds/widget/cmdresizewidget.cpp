/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdresizewidget.h"

// app includes
#include "umlscene.h"
#include "umlwidget.h"

#include <KLocalizedString>

namespace Uml
{

    CmdResizeWidget::CmdResizeWidget(UMLWidget *widget)
      : CmdBaseWidgetCommand(widget)
    {
        Q_ASSERT(widget != 0);
        setText(i18n("Resize widget : %1", widget->name()));

        m_size = QSizeF(widget->width(), widget->height());
        m_sizeOld = widget->startResizeSize();
    }

    CmdResizeWidget::~CmdResizeWidget()
    {
    }

    void CmdResizeWidget::redo()
    {
        widget()->setSize(m_size);
    }

    void CmdResizeWidget::undo()
    {
        widget()->setSize(m_sizeOld);
    }

}
