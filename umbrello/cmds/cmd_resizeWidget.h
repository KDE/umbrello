/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_RESIZEWIDGET_H
#define CMD_RESIZEWIDGET_H

#include <QSizeF>
#include <QUndoCommand>

class UMLWidget;

namespace Uml
{
    class CmdResizeWidget : public QUndoCommand
    {
    public:
        explicit CmdResizeWidget(UMLWidget* widget);
        ~CmdResizeWidget();

        void redo();
        void undo();

    private:
        UMLWidget*  m_widget;
        QSizeF      m_size;
        QSizeF      m_sizeOld;
    };
}

#endif
