/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#ifndef CMD_CHANGELINECOLOR_H
#define CMD_CHANGELINECOLOR_H

#include "cmd_baseWidgetCommand.h"

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

#endif // CMD_CHANGELINECOLOR_H
