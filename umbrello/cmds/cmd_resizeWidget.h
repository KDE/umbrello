/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2008                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef CMD_RESIZEWIDGET_H
#define CMD_RESIZEWIDGET_H

#include <QtGui/QUndoCommand>

#include "umlwidget.h"

namespace Uml
{
    class cmdResizeWidget : public QUndoCommand
    {
        public:
            /*Constructor */
            cmdResizeWidget(UMLWidgetController* _UMLwc);
            /*Destructor */
            ~cmdResizeWidget();
            /*Redo method */
            void redo();
            /*Undo method */
            void undo();

        private:
            UMLWidgetController*    UMLwc; // Attribute used to find which widget moved
            int         H; // new H value (used in redo method )
            int         W; // new W value (used in redo method )
            int         oldH; // old H value (used in undo method )
            int         oldW; // old W value (used in undo method )
            bool        already;
    };
}

#endif
