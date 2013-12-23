/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_CHANGEFONT_H
#define CMD_CHANGEFONT_H

#include "umlwidget.h"

#include <QFont>
#include <QUndoCommand>

namespace Uml
{
    class CmdChangeFont: public QUndoCommand
    {
        public:
            CmdChangeFont(UMLWidget* widget, QFont fon);

            void undo();
            void redo();

        private:
            UMLWidget* m_widget;
            QFont     m_newFont;
            QFont     m_oldFont;
    };
}

#endif // CMD_CHANGEFONT_H
