/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2020                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdremovewidget.h"

// app includes
#include "associationwidget.h"
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
            if (child != nullptr) {
                QDomDocument doc;
                m_children.append(doc.createElement(QLatin1String("child")));
                child->saveToXMI1(doc, m_children.back());
            }
        }

        QDomDocument doc;
        m_element = doc.createElement(QLatin1String("widget"));
        widget->saveToXMI1(doc, m_element);
    }

    /**
     * Constructor.
     */
    CmdRemoveWidget::CmdRemoveWidget(AssociationWidget* widget)
      : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Remove widget : %1", widget->name()));

        QDomDocument doc;
        m_element = doc.createElement(QLatin1String("widget"));
        widget->saveToXMI1(doc, m_element);
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
        if (!m_isAssoc)
            removeWidgetFromScene(widget());
        else
            removeWidgetFromScene(assocWidget());
    }

    /**
     * Add the widget back
     */
    void CmdRemoveWidget::undo()
    {
        if (!m_isAssoc) {
            QDomElement widgetElement = m_element.firstChild().toElement();

            UMLWidget* widget = scene()->loadWidgetFromXMI(widgetElement);
            if (widget) {
                addWidgetToScene(widget);
            }

            foreach(QDomElement childElement, m_children) {
                widgetElement = childElement.firstChild().toElement();
                widget = scene()->loadWidgetFromXMI(widgetElement);
                if (widget != nullptr) {
                    addWidgetToScene(widget);
                }
            }
        } else {
            AssociationWidget* widget = scene()->findAssocWidget(m_widgetId);
            if (widget == nullptr) {
                // If the widget is not found, the add command was undone. Load the
                // widget back from the saved XMI state.
                QDomElement widgetElement = m_element.firstChild().toElement();
                widget = AssociationWidget::create(scene());
                if (widget->loadFromXMI1(widgetElement)) {
                    addWidgetToScene(widget);
                    m_assocWidget = widget;
                    m_widgetId = widget->id();
                } else
                    delete widget;
            }
        }
    }
}
