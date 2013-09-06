/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_CREATE_WIDGET_H
#define CMD_CREATE_WIDGET_H

#include <QUndoCommand>

class UMLWidget;

namespace Uml
{
    class CmdCreateWidget : public QUndoCommand
    {
        public:
            CmdCreateWidget(UMLWidget* widget);
            ~CmdCreateWidget();

            void redo();
            void undo();

        private:
            UMLWidget*  m_widget;
    };
}

#endif
