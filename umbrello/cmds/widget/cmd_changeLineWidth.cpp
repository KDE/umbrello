/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_changeLineWidth.h"

// app includes
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    CmdChangeLineWidth::CmdChangeLineWidth(UMLWidget *w, const uint width)
      : m_widget(w),
        m_newWidth(width)
    {
        Q_ASSERT(w != 0);
        setText(i18n("Change line width : %1", w->name()));
        m_oldWidth = w->lineWidth() ;
    }

    CmdChangeLineWidth::~CmdChangeLineWidth()
    {
    }

    void CmdChangeLineWidth::redo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setLineWidthCmd(m_newWidth);
        }
    }

    void CmdChangeLineWidth::undo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setLineWidthCmd(m_oldWidth);
        }
    }
}
