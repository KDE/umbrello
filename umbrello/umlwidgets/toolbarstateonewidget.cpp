/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2021 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "toolbarstateonewidget.h"

// app includes
#include "activitywidget.h"
#include "debug_utils.h"
#include "dialog_utils.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "model_utils.h"
#include "objectwidget.h"
#include "pinwidget.h"
#include "portwidget.h"
#include "preconditionwidget.h"
#include "regionwidget.h"
#include "uml.h"
#include "umldoc.h"
#include "port.h"
#include "umlscene.h"
#include "umlwidget.h"
#include "object_factory.h"
#include "package.h"
#include "widget_factory.h"
#include "widget_utils.h"

// kde includes
#include <KLocalizedString>
#include <KMessageBox>

// qt includes

// using namespace Uml;

/**
 * Creates a new ToolBarStateOneWidget.
 *
 * @param umlScene The UMLScene to use.
 */
ToolBarStateOneWidget::ToolBarStateOneWidget(UMLScene *umlScene)
  : ToolBarStatePool(umlScene),
    m_firstObject(0),
    m_isObjectWidgetLine(false)
{
}

/**
 * Destroys this ToolBarStateOneWidget.
 */
ToolBarStateOneWidget::~ToolBarStateOneWidget()
{
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

    UMLWidget *widget = m_pUMLScene->widgetAt(m_pMouseEvent->scenePos());
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
    WidgetBase::WidgetType type = widgetType();

    if (type == WidgetBase::wt_Precondition) {
        m_firstObject = 0;
    }
    if (type == WidgetBase::wt_Pin || type == WidgetBase::wt_Port) {
        m_firstObject = 0;
    }

    if (m_pMouseEvent->button() != Qt::LeftButton) {
        return;
    }

    UMLWidget *currWgt = currentWidget();

    // Prevent widget nested in a larger widget from disappearing.
    Widget_Utils::ensureNestedVisible(currWgt, m_pUMLScene->widgetList());

    if (!currWgt->isObjectWidget() &&
            !currWgt->isActivityWidget() &&
            !currWgt->isComponentWidget() &&
            !currWgt->isRegionWidget()) {
        return;
    }

    if (!m_firstObject && (type == WidgetBase::wt_Pin || type == WidgetBase::wt_Port)) {
        setWidget(currWgt);
        return;
    }

    if (!m_isObjectWidgetLine && !m_firstObject) {
        return;
    }

    if (!m_firstObject) {
        setWidget(currWgt);
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
 * The temporary visual message is created and mouse tracking enabled, so
 * mouse events will be delivered.
 *
 * @param firstObject The first object of the message.
 */
void ToolBarStateOneWidget::setWidget(UMLWidget* firstObject)
{
    m_firstObject = firstObject;

    UMLWidget * umlwidget = 0;
    //m_pUMLScene->viewport()->setMouseTracking(true);
    if (widgetType() == WidgetBase::wt_Precondition) {
        QString name = Widget_Utils::defaultWidgetName(WidgetBase::wt_Precondition);
        if (Dialog_Utils::askNewName(WidgetBase::wt_Precondition, name)) {
            umlwidget = new PreconditionWidget(m_pUMLScene, firstObject->asObjectWidget());
            umlwidget->setName(name);
        }
    } else if (widgetType() == WidgetBase::wt_Pin && m_firstObject->isActivityWidget()) {
        QString name = Widget_Utils::defaultWidgetName(WidgetBase::wt_Pin);
        if (Dialog_Utils::askNewName(WidgetBase::wt_Pin, name)) {
            umlwidget = new PinWidget(m_pUMLScene, m_firstObject);
            umlwidget->setName(name);
        }
    } else if (widgetType() == WidgetBase::wt_Port && m_firstObject->isComponentWidget()) {
        UMLPackage* component = m_firstObject->umlObject()->asUMLPackage();
        QString name = Model_Utils::uniqObjectName(UMLObject::ot_Port, component);
        if (Dialog_Utils::askNewName(WidgetBase::wt_Port, name)) {
            UMLPort *port = Object_Factory::createUMLObject(UMLObject::ot_Port, name, component)->asUMLPort();
            PortWidget *widget = new PortWidget(m_pUMLScene, port, m_firstObject);
            widget->setInitialPosition(m_pUMLScene->pos());
            umlwidget = widget;
        }
    }

    if (umlwidget) {
        m_pUMLScene->setupNewWidget(umlwidget);
    }
    emit finished();
}

/**
 * Returns the widget type of this tool.
 *
 * @return The widget type of this tool.
 */
WidgetBase::WidgetType ToolBarStateOneWidget::widgetType()
{
    if (getButton() == WorkToolBar::tbb_Seq_Precondition) {
        return WidgetBase::wt_Precondition;
    }

    if (getButton() == WorkToolBar::tbb_Pin) {
        return WidgetBase::wt_Pin;
    }

    if (getButton() == WorkToolBar::tbb_Port) {
        return WidgetBase::wt_Port;
    }
    // Shouldn't happen
    Q_ASSERT(0);
    return WidgetBase::wt_Pin;
}

/**
 * Goes back to the initial state.
 */
void ToolBarStateOneWidget::init()
{
    ToolBarStatePool::init();
}

