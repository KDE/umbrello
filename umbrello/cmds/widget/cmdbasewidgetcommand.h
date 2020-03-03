/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMDBASEWIDGETCOMMAND_H
#define CMDBASEWIDGETCOMMAND_H

#include "basictypes.h"

#include <QPointer>
#include <QUndoCommand>

class AssociationWidget;
class UMLScene;
class UMLWidget;

namespace Uml
{
    class CmdBaseWidgetCommand : public QUndoCommand
    {
        public:
            explicit CmdBaseWidgetCommand(UMLWidget* widget);
            CmdBaseWidgetCommand(AssociationWidget* widget);
            virtual ~CmdBaseWidgetCommand();

        protected:
            Uml::ID::Type m_sceneId;
            Uml::ID::Type m_widgetId;
            QPointer<UMLScene> m_scene;
            QPointer<UMLWidget> m_widget;
            QPointer<AssociationWidget> m_assocWidget;
            bool m_isAssoc;

            void setWidget(UMLWidget* widget);
            void setWidget(AssociationWidget* widget);
            void addWidgetToScene(UMLWidget* widget);
            void addWidgetToScene(AssociationWidget* widget);
            void removeWidgetFromScene(UMLWidget *widget);
            void removeWidgetFromScene(AssociationWidget *widget);
            UMLWidget* widget();
            AssociationWidget* assocWidget();
            UMLScene* scene();
    };
}

#endif // CMDBASEWIDGETCOMMAND_H
