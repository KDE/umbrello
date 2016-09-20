/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

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
