/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_MOVEWIDGET_H
#define CMD_MOVEWIDGET_H

#include "cmd_baseWidgetCommand.h"

#include <QPointF>

class UMLWidget;

namespace Uml
{
    class CmdMoveWidget : public CmdBaseWidgetCommand
    {
    public:
        explicit CmdMoveWidget(UMLWidget* widget);
        ~CmdMoveWidget();

        void redo();
        void undo();

    private:
        QPointF     m_pos;
        QPointF     m_posOld;

    };
}

#endif
