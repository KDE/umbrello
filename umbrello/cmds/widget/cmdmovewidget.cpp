/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdmovewidget.h"

// app includes
#include "basictypes.h"
#include "umlscene.h"
#include "umlwidget.h"

#include <KLocalizedString>

namespace Uml
{

    CmdMoveWidget::CmdMoveWidget(UMLWidget *widget)
      : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Move widget : %1", widget->name()));

        m_pos = widget->pos();
        m_posOld = widget->startMovePosition();
    }

    CmdMoveWidget::~CmdMoveWidget()
    {
    }

    void CmdMoveWidget::redo()
    {
        UMLWidget* umlWidget = widget();
        umlWidget->setPos(m_pos);
        umlWidget->updateGeometry();
    }

    void CmdMoveWidget::undo()
    {
        UMLWidget* umlWidget = widget();
        umlWidget->setPos(m_posOld);
        umlWidget->updateGeometry();
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
