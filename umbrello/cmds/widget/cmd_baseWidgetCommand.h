/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_BASEWIDGETCOMMAND_H
#define CMD_BASEWIDGETCOMMAND_H

#include "basictypes.h"

#include <QUndoCommand>

class UMLScene;
class UMLWidget;

namespace Uml
{
    class CmdBaseWidgetCommand : public QUndoCommand
    {
        public:
            explicit CmdBaseWidgetCommand(UMLWidget* widget);
            virtual ~CmdBaseWidgetCommand();

        protected:
            Uml::ID::Type m_sceneId;
            Uml::ID::Type m_widgetId;

            void setWidget(UMLWidget* widget);
            void addWidgetToScene(UMLWidget* widget);
            UMLWidget* widget();
            UMLScene* scene();
    };
}

#endif // CMD_BASEWIDGETCOMMAND_H
