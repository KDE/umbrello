/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_CHANGEVISUALPROPERTY_H
#define CMD_CHANGEVISUALPROPERTY_H

#include "classifierwidget.h"
#include "cmd_baseWidgetCommand.h"


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

#endif // CMD_CHANGEVISUALPROPERTY_H
