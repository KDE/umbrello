/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef CMD_CHANGETEXTCOLOR_H
#define CMD_CHANGETEXTCOLOR_H

#include <QtGui/QUndoCommand>
#include <QtGui/QColor>

class UMLWidget;

namespace Uml
{
    class CmdChangeTextColor : public QUndoCommand
    {
    public:
        //CmdChangeLineColor(UMLView *view, const QColor& col);
        CmdChangeTextColor(UMLWidget *w, const QColor& col);
        ~CmdChangeTextColor();
        void redo();
        void undo();

    private:
        UMLWidget* m_umlWidget;
        //UMLView* m_view;

        QColor m_oldColor;
        QColor m_newColor;
    };
}

#endif // CMD_CHANGELINECOLOR_H
