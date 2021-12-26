/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2002-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

#include "cmdremovewidget.h"

// app includes
#include "associationwidget.h"
#include "debug_utils.h"
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

        // save "child" elements
        foreach(QGraphicsItem* item, widget->childItems()) {
            UMLWidget* child = dynamic_cast<UMLWidget*>(item);
            uIgnoreZeroPointer(child);
            QString xmi;
            QXmlStreamWriter kidStream(&xmi);
            kidStream.writeStartElement(QLatin1String("child"));
            child->saveToXMI(kidStream);
            kidStream.writeEndElement();  // child
            QString error;
            int line;
            QDomDocument domDoc;
            if (domDoc.setContent(xmi, &error, &line)) {
                QDomElement domElem = domDoc.firstChild().firstChild().toElement();
                if (domElem.isNull())
                    uWarning() << "child QDomElement is null";
                else
                    m_children.append(domElem);
            } else {
                uWarning() << "Cannot set content:" << error << " line:" << line;
            }
        }

        // save "widget" element
        QString xmi;
        QXmlStreamWriter stream(&xmi);
        stream.writeStartElement(QLatin1String("widget"));
        widget->saveToXMI(stream);
        stream.writeEndElement();  // widget
        QString error;
        int line;
        QDomDocument doc;
        if (doc.setContent(xmi, &error, &line)) {
            m_element = doc.firstChild().firstChild().toElement();
            if (m_element.isNull())
                uWarning() << "widget QDomElement is null";
        } else {
            uWarning() << "Cannot set content:" << error << " line:" << line;
        }
    }

    /**
     * Constructor.
     */
    CmdRemoveWidget::CmdRemoveWidget(AssociationWidget* widget)
      : CmdBaseWidgetCommand(widget)
    {
        setText(i18n("Remove widget : %1", widget->name()));

        // save "widget" element
        QString xmi;
        QXmlStreamWriter stream(&xmi);
        stream.writeStartElement(QLatin1String("widget"));
        widget->saveToXMI(stream);
        stream.writeEndElement();  // widget
        QString error;
        int line;
        QDomDocument doc;
        if (doc.setContent(xmi, &error, &line)) {
            m_element = doc.firstChild().firstChild().toElement();
            if (m_element.isNull())
                uWarning() << "widget QDomElement is null";
        } else {
            uWarning() << "Cannot set content:" << error << " line:" << line;
        }
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
                if (widget->loadFromXMI(widgetElement)) {
                    addWidgetToScene(widget);
                    m_assocWidget = widget;
                    m_widgetId = widget->id();
                } else
                    delete widget;
            }
        }
    }
}
