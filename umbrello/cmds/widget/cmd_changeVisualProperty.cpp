/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_changeVisualProperty.h"

// app includes
#include "umlscene.h"
#include "classifierwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdChangeVisualProperty::CmdChangeVisualProperty(
        ClassifierWidget* w,
        ClassifierWidget::VisualProperty property,
        bool value
    ) : m_widget(w),
        m_property(property),
        m_newValue(value)
    {
        Q_ASSERT(w != 0);
        setText(i18n("Change visual property : %1", w->name()));
        m_oldValue = w->visualProperty(property);
    }

    CmdChangeVisualProperty::~CmdChangeVisualProperty()
    {
    }

    void CmdChangeVisualProperty::redo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setVisualPropertyCmd(m_property, m_newValue);
        }
    }

    void CmdChangeVisualProperty::undo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setVisualPropertyCmd(m_property, m_oldValue);
        }
    }
}
