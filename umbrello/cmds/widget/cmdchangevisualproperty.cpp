/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdchangevisualproperty.h"

// app includes
#include "classifierwidget.h"
#include "umlwidget.h"
#include "debug_utils.h"

// kde includes
#include <KLocalizedString>

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
        ClassifierWidget* classifier = widget()->asClassifierWidget();
        if (classifier)
            classifier->setVisualPropertyCmd(m_property, m_newValue);
        else
            uWarning() << "could not find classifier widget with id" << Uml::ID::toString(m_widgetId);
    }

    void CmdChangeVisualProperty::undo()
    {
        ClassifierWidget* classifier = widget()->asClassifierWidget();
        if (classifier)
            classifier->setVisualPropertyCmd(m_property, m_oldValue);
        else
            uWarning() << "could not find classifier widget with id" << Uml::ID::toString(m_widgetId);
    }
}
