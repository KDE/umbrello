/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_RESIZEWIDGET_H
#define CMD_RESIZEWIDGET_H

#include "cmd_baseWidgetCommand.h"

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
