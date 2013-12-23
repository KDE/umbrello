/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2013                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

 /* Created By Krzywda Stanislas and Bouchikhi Mohamed-Amine ;) */

#include "cmd_changeFillColor.h"

#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{
    CmdChangeFillColor::CmdChangeFillColor(UMLWidget *w, const QColor& col)
      : m_widget(w),
        m_color(col)
    {
        Q_ASSERT(w != 0);
        setText(i18n("Change fill color : %1", w->name()));
        m_oldColor = w->fillColor();
    }

    CmdChangeFillColor::~CmdChangeFillColor()
    {
    }

    void CmdChangeFillColor::redo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setFillColorCmd(m_color);
        }
    }

    void CmdChangeFillColor::undo()
    {
        UMLScene* scene = m_widget->umlScene();
        if (scene && scene->widgetOnDiagram(m_widget->id())) {
            m_widget->setFillColorCmd(m_oldColor);
        }
    }
}
