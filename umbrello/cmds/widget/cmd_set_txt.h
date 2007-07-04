/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *  copyright (C) 2002-2006                                                *
 *  Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                   *
 ***************************************************************************/

#ifndef __CMD_SETTXT__
#define __CMD_SETTXT__

#include <QUndoCommand>

#include "../floatingtextwidget.h"
#include "umlwidget.h"

namespace Uml
{
    class cmdSetTxt : public QUndoCommand
    {
        public:
            /*Constructor */
            cmdSetTxt(FloatingTextWidget* _ftw, const QString& txt);
            /*Destructor */
            ~cmdSetTxt();
            /*Redo method */
            void redo();
            /*Undo method */
            void undo();

        private:

            FloatingTextWidget*     ftw;
            QString     newstring;
            QString     oldstring;
    };
}

#endif
