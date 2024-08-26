/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
        Q_ASSERT(widget != nullptr);
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
