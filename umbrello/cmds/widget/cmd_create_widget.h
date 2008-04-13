/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2008                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_CREATE_WIDGET_H
#define CMD_CREATE_WIDGET_H

#include <QtGui/QUndoCommand>

#include "umlwidget.h"
#include "umlview.h"

namespace Uml
{
    class cmdCreateWidget : public QUndoCommand
    {
        public:
            cmdCreateWidget(UMLView* view, UMLWidget* w);
            ~cmdCreateWidget();

            void redo();
            void undo();

        private:
            UMLView*    m_view;
            UMLWidget*  m_widget;
    };
}

#endif
