/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDMOVEWIDGET_H
#define CMDMOVEWIDGET_H

#include "cmdbasewidgetcommand.h"

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
