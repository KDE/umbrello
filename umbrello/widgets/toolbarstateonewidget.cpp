/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "toolbarstateonewidget.h"

// kde includes
#include <kdebug.h>

// local includes
#include "floatingtextwidget.h"
#include "pinwidget.h"
#include "preconditionwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"
#include "activitywidget.h"
#include "regionwidget.h"
#include "umlwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "umlscene.h"
#include "dialog_utils.h"
#include "umlscene.h"

#include <klocale.h>
#include <kmessagebox.h>

using namespace Uml;

ToolBarStateOneWidget::ToolBarStateOneWidget(UMLScene *umlScene) : ToolBarStatePool(umlScene)
{
    m_umlScene = umlScene;
    m_firstObject = 0;
}

ToolBarStateOneWidget::~ToolBarStateOneWidget()
{
}

/**
 * Goes back to the initial state.
 */
void ToolBarStateOneWidget::init()
{
    ToolBarStatePool::init();
}

/**
 * Called when the current tool is changed to use another tool.
 * Executes base method and cleans the message.
 */
void ToolBarStateOneWidget::cleanBeforeChange()
{
    ToolBarStatePool::cleanBeforeChange();
}

/**
 * Called when a mouse event happened.
 * It executes the base method and then updates the position of the
 * message line, if any.
 */
void ToolBarStateOneWidget::mouseMove(QGraphicsSceneMouseEvent* ome)
{
    ToolBarStatePool::mouseMove(ome);
}

/**
 * A widget was removed from the UMLView.
 * If the widget removed was the current widget, the current widget is set
 * to 0.
 * Also, if it was the first object, the message is cleaned.
 */
void ToolBarStateOneWidget::slotWidgetRemoved(UMLWidget* widget)
{
    ToolBarState::slotWidgetRemoved(widget);
}

/**
 * Selects only widgets, but no associations.
 * Overrides base class method.
 * If the press event happened on the line of an object, the object is set
 * as current widget. If the press event happened on a widget, the widget is
 * set as current widget.
 */
void ToolBarStateOneWidget::setCurrentElement()
{
    m_isObjectWidgetLine = false;
    ObjectWidget* objectWidgetLine = m_pUMLScene->onWidgetLine(m_pMouseEvent->scenePos());
    if (objectWidgetLine) {
        setCurrentWidget(objectWidgetLine);
        m_isObjectWidgetLine = true;
        return;
    }

    UMLWidget *widget = m_pUMLScene->getWidgetAt(m_pMouseEvent->scenePos());
    if (widget) {
        setCurrentWidget(widget);
        return;
    }
}

/**
 * Called when the release event happened on a widget.
 * If the button pressed isn't left button or the widget isn't an object
 * widget, the message is cleaned.
 * If the release event didn't happen on the line of an object and the first
 * object wasn't selected, nothing is done. If the first object was already
 * selected, a creation message is made.
 * If the event happened on the line of an object, the first object or the
 * second are set, depending on whether the first object was already set or
 * not.
 */
void ToolBarStateOneWidget::mouseReleaseWidget()
{
    Uml::Widget_Type widgetType = getWidgetType();

    if (widgetType == Uml::wt_Precondition) {
        m_firstObject = 0;
    }
    if (widgetType == Uml::wt_Pin) {
        m_firstObject = 0;
    }

    if (m_pMouseEvent->button() != Qt::LeftButton ||(
                getCurrentWidget()->baseType() != Uml::wt_Object &&
                getCurrentWidget()->baseType() != Uml::wt_Activity &&
                getCurrentWidget()->baseType() != Uml::wt_Region)) {
        return;
    }

    if (!m_firstObject && widgetType == Uml::wt_Pin) {
        setWidget(getCurrentWidget());
        return ;
    }

    if (!m_isObjectWidgetLine && !m_firstObject) {
        return;
    }

    if (!m_firstObject) {
        setWidget(getCurrentWidget());
    }

}

/**
 * Called when the release event happened on an empty space.
 * Cleans the message.
 * Empty spaces are not only actual empty spaces, but also associations.
 */
void ToolBarStateOneWidget::mouseReleaseEmpty()
{
}

/**
 * Sets the first object of the message using the specified object.
 * The temporal visual message is created and mouse tracking enabled, so
 * mouse events will be delivered.
 *
 * @param firstObject The first object of the message.
 */
void ToolBarStateOneWidget::setWidget(UMLWidget* firstObject)
{
    m_firstObject = firstObject;

    UMLWidget * umlwidget = NULL;
    //m_pUMLScene->viewport()->setMouseTracking(true);
    if (getWidgetType() == Uml::wt_Precondition) {
        umlwidget = new PreconditionWidget(static_cast<ObjectWidget*>(m_firstObject));
        Q_ASSERT (umlwidget->umlScene() == m_pUMLScene);

        Dialog_Utils::askNameForWidget(umlwidget, i18n("Enter Precondition Name"), i18n("Enter the precondition"), i18n("new precondition"));
            // Create the widget. Some setup functions can remove the widget.
    }

    if (getWidgetType() == Uml::wt_Pin) {
        umlwidget = new PinWidget(m_firstObject);
            // Create the widget. Some setup functions can remove the widget.
    }

    if (umlwidget != NULL) {
        m_pUMLScene->setupNewWidget(umlwidget);

    }

}

/**
 * Returns the widget type of this tool.
 *
 * @return The widget type of this tool.
 */
Uml::Widget_Type ToolBarStateOneWidget::getWidgetType()
{
    if (getButton() == WorkToolBar::tbb_Seq_Precondition) {
        return Uml::wt_Precondition;
    }

    if (getButton() == WorkToolBar::tbb_Pin) {
        return Uml::wt_Pin;
    }
    // Shouldn't happen
    Q_ASSERT(0);
    return Uml::wt_Pin;
}

#include "toolbarstateonewidget.moc"
