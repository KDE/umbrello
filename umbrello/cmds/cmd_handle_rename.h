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

#ifndef CMD_HANDLE_RENAME_H
#define CMD_HANDLE_RENAME_H

#include <QtGui/QUndoCommand>

#include "floatingtextwidget.h"


namespace Uml
{
    class cmdHandleRename : public QUndoCommand
    {
        //friend class FloatingTextWidget;
        public:
            cmdHandleRename(FloatingTextWidget* _ftw, QString& txt);
            ~cmdHandleRename();

            void redo();
            void undo();

        private:
            FloatingTextWidget*     ftw;
            QString     newstring;
            QString     oldstring;
    };
}

#endif
