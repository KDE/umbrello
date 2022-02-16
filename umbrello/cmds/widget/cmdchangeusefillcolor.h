/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCHANGEUSEFILLCOLOR_H
#define CMDCHANGEUSEFILLCOLOR_H

#include "cmdbasewidgetcommand.h"

class UMLWidget;

namespace Uml
{

    class CmdChangeUseFillColor : public CmdBaseWidgetCommand
    {
        public:
            CmdChangeUseFillColor(UMLWidget* w, const bool value);
            ~CmdChangeUseFillColor();

            void redo();
            void undo();

        private:
            bool       m_oldValue;
            bool       m_newValue;
    };
}

#endif
