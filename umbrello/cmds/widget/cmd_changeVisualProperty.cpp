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
#include "classifierwidget.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdChangeVisualProperty::CmdChangeVisualProperty(
        ClassifierWidget* widget,
        ClassifierWidget::VisualProperty property,
        bool value
    ) : CmdBaseWidgetCommand(widget),
        m_property(property),
        m_newValue(value)
    {
        setText(i18n("Change visual property : %1", widget->name()));

        m_oldValue = widget->visualProperty(property);
    }

    CmdChangeVisualProperty::~CmdChangeVisualProperty()
    {
    }

    void CmdChangeVisualProperty::redo()
    {
        ClassifierWidget* classifier = dynamic_cast<ClassifierWidget*>(widget());
        classifier->setVisualPropertyCmd(m_property, m_newValue);
    }

    void CmdChangeVisualProperty::undo()
    {
        ClassifierWidget* classifier = dynamic_cast<ClassifierWidget*>(widget());
        classifier->setVisualPropertyCmd(m_property, m_oldValue);
    }
}
