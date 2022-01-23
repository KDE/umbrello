/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdcreatewidget.h"

// app includes
#include "associationwidget.h"
#include "debug_utils.h"
#include "model_utils.h"
#include "uml.h"
#include "umlscene.h"
#include "umlwidget.h"

// kde includes
#include <KLocalizedString>
#include <QXmlStreamWriter>

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

        QString xmi;
        QXmlStreamWriter stream(&xmi);
        stream.writeStartElement(QLatin1String("widget"));
        widget->saveToXMI(stream);
        stream.writeEndElement();  // widget
        QString error;
        int line;
        QDomDocument domDoc;
        if (domDoc.setContent(xmi, &error, &line)) {
            m_element = domDoc.firstChild().firstChild().toElement();
        } else {
            logWarn2("CmdCreateWidget: Cannot set content. Error %1 line %2", error, line);
        }
    }

    /**
     * Constructor.
     */
    CmdCreateWidget::CmdCreateWidget(AssociationWidget* widget)
      : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Create widget : %1", widget->name()));

        addWidgetToScene(widget);

        QString xmi;
        QXmlStreamWriter stream(&xmi);
        stream.writeStartElement(QLatin1String("widget"));
        widget->saveToXMI(stream);
        stream.writeEndElement();  // widget
        QString error;
        int line;
        QDomDocument domDoc;
        if (domDoc.setContent(xmi, &error, &line)) {
            m_element = domDoc.firstChild().firstChild().toElement();
        } else {
            logWarn2("CmdCreateWidget(assocwidget): Cannot set content. Error %1 line %2", error, line);
        }
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
            if (widget->loadFromXMI(widgetElement)) {
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
