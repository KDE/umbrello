/*
    SPDX-License-Identifier: GPL-2.0-or-later

    copyright (C) 2002-2020
    Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

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
