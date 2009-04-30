/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_RESIZEWIDGET_H
#define CMD_RESIZEWIDGET_H

#include <QtGui/QUndoCommand>

class UMLWidgetController;

namespace Uml
{
    class CmdResizeWidget : public QUndoCommand
    {
        public:
            CmdResizeWidget(UMLWidgetController* wc);
            ~CmdResizeWidget();

            void redo();
            void undo();

        private:
            UMLWidgetController*  m_widgetCtrl;
            int         m_h;
            int         m_w;
            int         m_oldH;
            int         m_oldW;
            bool        m_already;
    };
}

#endif
