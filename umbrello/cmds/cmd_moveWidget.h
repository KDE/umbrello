/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_MOVEWIDGET_H
#define CMD_MOVEWIDGET_H

#include <QtGui/QUndoCommand>

class UMLWidgetController;

namespace Uml
{
    class CmdMoveWidget : public QUndoCommand
    {
        public:
            CmdMoveWidget(UMLWidgetController* wc);
            ~CmdMoveWidget();

            void redo();
            void undo();

            bool mergeWith(const QUndoCommand* other);

        private:
            UMLWidgetController*    m_widgetCtrl;
            int         m_x;
            int         m_y;
            int         m_oldX;
            int         m_oldY;
            bool        m_already;

    };
}

#endif
