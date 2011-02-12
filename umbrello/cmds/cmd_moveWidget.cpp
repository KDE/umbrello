/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_moveWidget.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"

#include <klocale.h>

namespace Uml
{

    CmdMoveWidget::CmdMoveWidget(UMLWidgetController* wc)
      : QUndoCommand(), m_widgetCtrl(wc), m_already(false)
    {
        UMLWidget * w = wc->getWidget();
        setText(i18n("Move widget : %1", w->name()));
        m_x = w->getX();
        m_y = w->getY();
        m_oldX = wc->getOldX();
        m_oldY = wc->getOldY();
    }

    CmdMoveWidget::~CmdMoveWidget()
    {
    }

    void CmdMoveWidget::redo()
    {
        m_widgetCtrl->insertSaveValues(m_oldX, m_oldY, m_x, m_y);
        m_widgetCtrl->widgetMoved();

        //UMLWidget * w =
        //m_widgetCtrl->reverseOldNewValues();
        //m_widgetCtrl->widgetMoved();
        //m_widgetCtrl->moveWidget(diffX,diffY);
        //m_widgetCtrl->moveWidget(10,10);
        //widget->getWidgetController()->moveWidgetBy(diffX, diffY);
    }

    void CmdMoveWidget::undo()
    {
        //UMLWidget * w =
        m_widgetCtrl->insertSaveValues(m_x, m_y, m_oldX, m_oldY);
        m_widgetCtrl->widgetMoved();
        //w->setX(m_oldX);
        //w->setY(m_oldY);

        //m_widgetCtrl->moveWidget(w->getX() - m_oldX, w->getY() - m_oldY);
        //m_widgetCtrl->moveWidget(-10,-10);
        //m_widgetCtrl->moveWidget(-diffX,-diffY);
        //widget->getWidgetController()->moveWidgetBy(-diffX, -diffY);
    }

    bool CmdMoveWidget::mergeWith(const QUndoCommand* other)
    {
        const CmdMoveWidget* otherCmd = static_cast<const CmdMoveWidget*>(other);
        if (m_widgetCtrl != otherCmd->m_widgetCtrl)
            return false;
        m_x = otherCmd->m_x;
        m_y = otherCmd->m_y;
        return true;
    }
}
