/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCHANGELINEWIDTH_H
#define CMDCHANGELINEWIDTH_H

#include "cmdbasewidgetcommand.h"

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

#endif // CMDCHANGELINEWIDTH_H
