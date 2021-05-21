/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDRESIZEWIDGET_H
#define CMDRESIZEWIDGET_H

#include "cmdbasewidgetcommand.h"

#include <QSizeF>

class UMLWidget;

namespace Uml
{
    class CmdResizeWidget : public CmdBaseWidgetCommand
    {
    public:
        explicit CmdResizeWidget(UMLWidget* widget);
        ~CmdResizeWidget();

        void redo();
        void undo();

    private:
        QSizeF      m_size;
        QSizeF      m_sizeOld;
    };
}

#endif
