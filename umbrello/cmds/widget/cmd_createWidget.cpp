/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_createWidget.h"

// app includes
#include "umlwidget.h"
#include "umlscene.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdCreateWidget::CmdCreateWidget(UMLWidget* widget)
      : m_widget(widget)
    {
        Q_ASSERT(widget != 0);
        setText(i18n("Create widget : %1", widget->name()));
    }

    CmdCreateWidget::~CmdCreateWidget()
    {
    }

    /**
     * Create the UMLWidget.
     */
    void CmdCreateWidget::redo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setVisible(true);
        }
    }

    /**
     * Suppress the UMLWidget.
     */
    void CmdCreateWidget::undo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setVisible(false);
        }
    }

}
