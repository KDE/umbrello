/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_resizeWidget.h"

// app includes
#include "umlwidgetcontroller.h"
#include "umlwidget.h"

#include <klocale.h>

namespace Uml
{

    CmdResizeWidget::CmdResizeWidget(UMLWidgetController* wc)
      : QUndoCommand(),
        m_widgetCtrl(wc)
    {
        UMLWidget * w = wc->getWidget();
        setText(i18n("Resize widget : %1", w->name()));
        m_w = w->width();
        m_h = w->height();
        m_oldH = wc->getOldH();
        m_oldW = wc->getOldW();
    }

    CmdResizeWidget::~CmdResizeWidget()
    {
    }

    void CmdResizeWidget::redo()
    {
        UMLWidget * w = m_widgetCtrl->getWidget();
        UMLScene* scene = w->umlScene();
        if (scene->widgetOnDiagram(w->id())) {
            m_widgetCtrl->resizeWidget(m_w, m_h);
        }
    }

    void CmdResizeWidget::undo()
    {
        UMLWidget * w = m_widgetCtrl->getWidget();
        UMLScene* scene = w->umlScene();
        if (scene->widgetOnDiagram(w->id())) {
            m_widgetCtrl->resizeWidget(m_oldW, m_oldH);
        }
    }

}
