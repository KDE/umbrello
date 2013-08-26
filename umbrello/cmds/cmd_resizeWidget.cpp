/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_resizeWidget.h"

// app includes
#include "umlwidget.h"

#include <klocale.h>

namespace Uml
{

    CmdResizeWidget::CmdResizeWidget(UMLWidget *widget)
      : QUndoCommand(),
        m_widget(widget)
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
        UMLScene* scene = m_widget->umlScene();
        if (scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setSize(m_size);
        }
    }

    void CmdResizeWidget::undo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setSize(m_sizeOld);
        }
    }

}
