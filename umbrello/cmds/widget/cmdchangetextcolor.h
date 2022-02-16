/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCHANGETEXTCOLOR_H
#define CMDCHANGETEXTCOLOR_H

#include "cmdbasewidgetcommand.h"

#include <QColor>

class UMLWidget;

namespace Uml
{
    class CmdChangeTextColor : public CmdBaseWidgetCommand
    {
    public:
        CmdChangeTextColor(UMLWidget *w, const QColor& col);
        ~CmdChangeTextColor();
        void redo();
        void undo();

    private:
        QColor m_oldColor;
        QColor m_newColor;
        bool m_oldUsesDiagramValue;
    };
}

#endif // CMDCHANGELINECOLOR_H
