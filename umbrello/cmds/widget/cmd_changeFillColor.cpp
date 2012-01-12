/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#include "cmd_changeFillColor.h"

#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeFillColor::CmdChangeFillColor(UMLWidget *w, const QColor& col)
      : m_umlWidget(w),
        m_color(col)
    {
        setText(i18n("Change fill color : %1", w->name()));
        m_oldColor = w->fillColor();
    }

    CmdChangeFillColor::~CmdChangeFillColor()
    {
    }

    void CmdChangeFillColor::redo()
    {
        m_umlWidget->setFillColorcmd( m_color );
    }

    void CmdChangeFillColor::undo()
    {
        m_umlWidget->setFillColorcmd( m_oldColor );
    }
}
