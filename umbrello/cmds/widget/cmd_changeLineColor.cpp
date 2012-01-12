/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_changeLineColor.h"

// app includes
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    /*CmdChangeLineColor::CmdChangeLineColor(UMLView *view, const QColor& col)
    {
        setText(i18n("Change Line Color"));
        UMLWidget * widget = view->getFirstMultiSelectedWidget();
        m_view = view;
        m_newColor = col;
        m_oldColor = widget->lineColor();
    }*/

    CmdChangeLineColor::CmdChangeLineColor(UMLWidget *w, const QColor& col)
      : m_umlWidget(w), m_newColor(col)
    {
        setText(i18n("Change line color : %1", w->name()));
        m_oldColor= w->lineColor() ;
    }

    CmdChangeLineColor::~CmdChangeLineColor()
    {
    }

    void CmdChangeLineColor::redo()
    {
        m_umlWidget->setLineColorcmd( m_newColor );
    }

    void CmdChangeLineColor::undo()
    {
        m_umlWidget->setLineColorcmd( m_oldColor );
    }

}
