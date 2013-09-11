/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_changeTextColor.h"

// app includes
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeTextColor::CmdChangeTextColor(UMLWidget *w, const QColor& col)
      : m_widget(w),
        m_newColor(col)
    {
        Q_ASSERT(w != 0);
        setText(i18n("Change text color : %1", w->name()));
        m_oldColor= w->textColor() ;
    }

    CmdChangeTextColor::~CmdChangeTextColor()
    {
    }

    void CmdChangeTextColor::redo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setTextColorcmd(m_newColor);
        }
    }

    void CmdChangeTextColor::undo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setTextColorcmd(m_oldColor);
        }
    }

}
