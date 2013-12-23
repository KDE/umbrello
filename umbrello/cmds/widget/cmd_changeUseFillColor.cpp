/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_changeUseFillColor.h"

#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeUseFillColor::CmdChangeUseFillColor(UMLWidget *w, bool value)
      : m_widget(w),
        m_newValue(value)
    {
        Q_ASSERT(w != 0);
        if (value) {
            setText(i18n("Use fill color : %1", w->name()));
        } else {
            setText(i18n("No fill color : %1", w->name()));
        }
        m_oldValue = w->useFillColor();
    }

    CmdChangeUseFillColor::~CmdChangeUseFillColor()
    {
    }

    void CmdChangeUseFillColor::redo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setUseFillColorCmd(m_newValue);
        }
    }

    void CmdChangeUseFillColor::undo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setUseFillColorCmd(m_oldValue);
        }
    }
}
