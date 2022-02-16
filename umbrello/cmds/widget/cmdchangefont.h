/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCHANGEFONT_H
#define CMDCHANGEFONT_H

#include "cmdbasewidgetcommand.h"

#include <QFont>

class UMLWidget;

namespace Uml
{
    class CmdChangeFont : public CmdBaseWidgetCommand
    {
        public:
            CmdChangeFont(UMLWidget* widget, QFont font);

            void undo();
            void redo();

        private:
            QFont         m_newFont;
            QFont         m_oldFont;
    };
}

#endif // CMDCHANGEFONT_H
