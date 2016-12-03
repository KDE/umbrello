/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdremovewidget.h"

// app includes
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

namespace Uml
{

    /**
     * Constructor.
     */
    CmdRemoveWidget::CmdRemoveWidget(UMLWidget* widget)
      : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Remove widget : %1", widget->name()));

        foreach(QGraphicsItem* item, widget->childItems()) {
            UMLWidget* child = dynamic_cast<UMLWidget*>(item);
            QDomDocument doc;
            m_children.append(doc.createElement(QLatin1String("child")));
            child->saveToXMI(doc, m_children.back());
        }

        QDomDocument doc;
        m_element = doc.createElement(QLatin1String("widget"));
        widget->saveToXMI(doc, m_element);
    }

    /**
     *  Destructor.
     */
    CmdRemoveWidget::~CmdRemoveWidget()
    {
    }

    /**
     * Remove the widget
     */
    void CmdRemoveWidget::redo()
    {
        UMLScene* umlScene = scene();
        UMLWidget* widget = umlScene->findWidget(m_widgetId);
        if (widget != 0) {
            umlScene->removeWidgetCmd(widget);
        }
    }

    /**
     * Add the widget back
     */
    void CmdRemoveWidget::undo()
    {
        QDomElement widgetElement = m_element.firstChild().toElement();

        UMLScene* umlScene = scene();
        UMLWidget* widget = umlScene->loadWidgetFromXMI(widgetElement);
        if (widget) {
            addWidgetToScene(widget);
        }

        foreach(QDomElement childElement, m_children) {
            widgetElement = childElement.firstChild().toElement();
            widget = umlScene->loadWidgetFromXMI(widgetElement);
            if (0 != widget) {
                addWidgetToScene(widget);
            }
        }
    }
}
