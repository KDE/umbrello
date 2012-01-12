/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#include "cmd_changeTextColor.h"

// app includes
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeTextColor::CmdChangeTextColor(UMLWidget *w, const QColor& col)
      : m_umlWidget(w), m_newColor(col)
    {
        setText(i18n("Change text color : %1", w->name()));
        m_oldColor= w->textColor() ;
    }

    CmdChangeTextColor::~CmdChangeTextColor()
    {
    }

    void CmdChangeTextColor::redo()
    {
        m_umlWidget->setTextColorcmd( m_newColor );
    }

    void CmdChangeTextColor::undo()
    {
        m_umlWidget->setTextColorcmd( m_oldColor );
    }

}
