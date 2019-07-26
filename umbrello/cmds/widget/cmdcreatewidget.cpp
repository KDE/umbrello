/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2002-2014                                               *
 *   Umbrello UML Modeller Authors <umbrello-devel@kde.org>                *
 ***************************************************************************/

#include "cmdcreatewidget.h"

// app includes
#include "associationwidget.h"
#include "model_utils.h"
#include "uml.h"
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>

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
        m_element = doc.createElement(QLatin1String("widget"));
        widget->saveToXMI1(doc, m_element);
    }

    /**
     * Constructor.
     */
    CmdCreateWidget::CmdCreateWidget(AssociationWidget* widget)
      : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Create widget : %1", widget->name()));

        addWidgetToScene(widget);

        QDomDocument doc;
        m_element = doc.createElement(QLatin1String("widget"));
        widget->saveToXMI1(doc, m_element);
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
        if (!m_isAssoc) {
            UMLWidget* widget = scene()->findWidget(m_widgetId);
            if (widget == nullptr) {
                // If the widget is not found, the add command was undone. Load the
                // widget back from the saved XMI state.
                QDomElement widgetElement = m_element.firstChild().toElement();
                widget = scene()->loadWidgetFromXMI(widgetElement);
                if (widget) {
                    addWidgetToScene(widget);
                }
            }
        } else {
            AssociationWidget* widget = scene()->findAssocWidget(m_widgetId);
            if (widget)
                return;
            if (m_assocWidget) {
                scene()->addAssociation(m_assocWidget, false);
                return;
            }

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

    /**
     * Remove the widget
     */
    void CmdCreateWidget::undo()
    {
        if (!m_isAssoc)
            removeWidgetFromScene(widget());
        else
            removeWidgetFromScene(assocWidget());
    }
}
