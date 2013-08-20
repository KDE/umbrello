/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_moveWidget.h"

// app includes
#include "umlwidget.h"

#include <klocale.h>

namespace Uml
{

    CmdMoveWidget::CmdMoveWidget(UMLWidget *widget)
      : QUndoCommand(),
        m_widget(widget)
    {
        Q_ASSERT(widget != 0);
        setText(i18n("Move widget : %1", widget->name()));
        m_pos = widget->pos();
        m_posOld = widget->startMovePosition();
    }

    CmdMoveWidget::~CmdMoveWidget()
    {
    }

    void CmdMoveWidget::redo()
    {
        m_widget->setPos(m_pos);
        m_widget->updateGeometry();
    }

    void CmdMoveWidget::undo()
    {
        m_widget->setPos(m_posOld);
        m_widget->updateGeometry();
    }

//    bool CmdMoveWidget::mergeWith(const QUndoCommand* other)
//    {
//        const CmdMoveWidget* otherCmd = static_cast<const CmdMoveWidget*>(other);
//        if (m_widgetCtrl != otherCmd->m_widgetCtrl)
//            return false;
//        m_x = otherCmd->m_x;
//        m_y = otherCmd->m_y;
//        return true;
//    }
}
