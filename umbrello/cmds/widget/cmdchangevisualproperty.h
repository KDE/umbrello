/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCHANGEVISUALPROPERTY_H
#define CMDCHANGEVISUALPROPERTY_H

#include "classifierwidget.h"
#include "cmdbasewidgetcommand.h"


namespace Uml
{
    class CmdChangeVisualProperty : public CmdBaseWidgetCommand
    {
    public:
        CmdChangeVisualProperty(ClassifierWidget* widget, ClassifierWidget::VisualProperty property, bool value);
        ~CmdChangeVisualProperty();
        void redo();
        void undo();

    private:
        ClassifierWidget::VisualProperty m_property;
        bool m_newValue;
        bool m_oldValue;
    };
}

#endif // CMDCHANGEVISUALPROPERTY_H
