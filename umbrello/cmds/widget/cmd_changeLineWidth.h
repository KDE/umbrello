/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_CHANGELINEWIDTH_H
#define CMD_CHANGELINEWIDTH_H

#include "cmd_baseWidgetCommand.h"

class UMLWidget;

namespace Uml
{
    class CmdChangeLineWidth : public CmdBaseWidgetCommand
    {
    public:
        CmdChangeLineWidth(UMLWidget *w, const uint width);
        ~CmdChangeLineWidth();
        void redo();
        void undo();

    private:
        uint          m_oldWidth;
        uint          m_newWidth;
    };
}

#endif // CMD_CHANGELINEWIDTH_H
