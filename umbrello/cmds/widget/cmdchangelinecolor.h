/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2014 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#ifndef CMDCHANGELINECOLOR_H
#define CMDCHANGELINECOLOR_H

#include "cmdbasewidgetcommand.h"

#include <QColor>

class UMLWidget;

namespace Uml
{
    class CmdChangeLineColor : public CmdBaseWidgetCommand
    {
    public:
        CmdChangeLineColor(UMLWidget *w, const QColor& col);
        ~CmdChangeLineColor();
        void redo();
        void undo();

    private:
        QColor        m_oldColor;
        QColor        m_newColor;
        bool          m_oldUsesDiagramValue;
    };
}

#endif // CMDCHANGELINECOLOR_H
