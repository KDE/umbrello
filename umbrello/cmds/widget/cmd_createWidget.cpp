/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmd_createWidget.h"

// app includes
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <klocale.h>

namespace Uml
{

    /**
     * Constructor.
     */
    CmdCreateWidget::CmdCreateWidget(UMLWidget* widget)
      : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Create widget : %1", widget->name()));

        addWidgetToScene(widget);

        QDomDocument doc;
        m_element = doc.createElement("widget");
        widget->saveToXMI(doc, m_element);
    }

    /**
     *  Destructor.
     */
    CmdCreateWidget::~CmdCreateWidget()
    {
    }

    /**
     * Create the widget
     */
    void CmdCreateWidget::redo()
    {
        UMLWidget* umlWidget = scene()->findWidget(m_widgetId);
        if (umlWidget == 0) {
            // If the widget is not found, the add command was undone. Load the
            // widget back from the saved XMI state.
            QDomElement widgetElement = m_element.firstChild().toElement();
            umlWidget = scene()->loadWidgetFromXMI(widgetElement);

            addWidgetToScene(umlWidget);
        }
    }

    /**
     * Remove the widget
     */
    void CmdCreateWidget::undo()
    {
        UMLWidget* umlWidget = widget();
        if (umlWidget != 0) {
            scene()->removeWidgetCmd(umlWidget);
        }
    }
}
