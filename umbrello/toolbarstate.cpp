/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2004-2022 Umbrello UML Modeller Authors <umbrello-devel@kde.org>
*/

// own header
#include "toolbarstate.h"

// app includes
#include "associationwidget.h"
#include "debug_utils.h"
#include "messagewidget.h"
#include "floatingdashlinewidget.h"
#include "objectwidget.h"
#include "umlapp.h"
#include "umlview.h"
#include "umlwidget.h"
#include "widget_utils.h"

// qt includes
#include <QScrollBar>

DEBUG_REGISTER(ToolBarState)

/**
 * Destroys this ToolBarState.
 * Frees m_pMouseEvent.
 */
ToolBarState::~ToolBarState()
{
    delete m_pMouseEvent;
}

/**
 * Goes back to the initial state.
 * Subclasses can extend, but not override, this method as needed.
 */
void ToolBarState::init()
{
    if (m_pUMLScene->activeView())
        m_pUMLScene->activeView()->viewport()->setMouseTracking(false);
    m_pMouseEvent = nullptr;
    m_currentWidget = nullptr;
    m_currentAssociation = nullptr;

    connect(m_pUMLScene, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
            this, SLOT(slotAssociationRemoved(AssociationWidget*)));
    connect(m_pUMLScene, SIGNAL(sigWidgetRemoved(UMLWidget*)),
            this, SLOT(slotWidgetRemoved(UMLWidget*)));
}

/**
 * Called when the current tool is changed to use another tool.
 * Subclasses can extend, but not override, this method as needed.
 * Default implementation does nothing.
 */
void ToolBarState::cleanBeforeChange()
{
    disconnect(m_pUMLScene, SIGNAL(sigAssociationRemoved(AssociationWidget*)),
               this, SLOT(slotAssociationRemoved(AssociationWidget*)));
    disconnect(m_pUMLScene, SIGNAL(sigWidgetRemoved(UMLWidget*)),
               this, SLOT(slotWidgetRemoved(UMLWidget*)));
}

/**
 * Handler for mouse press events.
 * Mouse tracking is enabled, any pop up menu removed, the position of the
 * cursor set and paste state disabled.
 * Then, the current association or widget are set (if any), and events are
 * delivered to the specific methods, depending on where the cursor was
 * pressed.
 *
 * @param ome The received event.
 * @see setCurrentElement()
 */
void ToolBarState::mousePress(QGraphicsSceneMouseEvent* ome)
{
    setMouseEvent(ome, QEvent::MouseButtonPress);

    m_pUMLScene->activeView()->viewport()->setMouseTracking(true);

    // TODO: Check who needs this.
    m_pUMLScene->setPos(m_pMouseEvent->scenePos());

    //TODO check why
    m_pUMLScene->setPaste(false);

    setCurrentElement();

    if (currentWidget()) {
        mousePressWidget();
    } else if (currentAssociation()) {
        mousePressAssociation();
    } else {
        mousePressEmpty();
    }
}

/**
 * Handler for mouse release events.
 * Mouse tracking is disabled and the position of the cursor set.
 * The events are delivered to the specific methods, depending on where the
 * cursor was released, and the current association or widget cleaned.
 * Finally, the current tool is changed if needed.
 *
 * @param ome The received event.
 */
void ToolBarState::mouseRelease(QGraphicsSceneMouseEvent* ome)
{
    setMouseEvent(ome, QEvent::MouseButtonRelease);

    // Set the position of the mouse
    // TODO, should only be available in this state?
    m_pUMLScene->setPos(m_pMouseEvent->scenePos());

    m_pUMLScene->activeView()->viewport()->setMouseTracking(false);

    if (currentWidget()) {
        logDebug0("ToolBarState::mouseRelease calling mouseReleaseWidget");
        mouseReleaseWidget();
        setCurrentWidget(nullptr);
    } else if (currentAssociation()) {
        logDebug0("ToolBarState::mouseRelease calling mouseReleaseAssociation");
        mouseReleaseAssociation();
        setCurrentAssociation(nullptr);
    } else {
        logDebug0("ToolBarState::mouseRelease calling mouseReleaseEmpty");
        mouseReleaseEmpty();
    }

    // Default, rightbutton changes the tool.
    // The arrow tool overrides the changeTool() function.
    changeTool();
}

/**
 * Handler for mouse double click events.
 * The current association or widget is set (if any), and events are
 * delivered to the specific methods, depending on where the cursor was pressed.
 * After delivering the events, the current association or widget is cleaned.
 *
 * @param ome The received event.
 */
void ToolBarState::mouseDoubleClick(QGraphicsSceneMouseEvent* ome)
{
    setMouseEvent(ome, QEvent::MouseButtonDblClick);

    UMLWidget* currentWidget = m_pUMLScene->widgetAt(m_pMouseEvent->scenePos());
    AssociationWidget* currentAssociation = associationAt(m_pMouseEvent->scenePos());

    if (currentWidget || currentAssociation) {
        ome->accept();
    }

    if (currentWidget) {
        setCurrentWidget(currentWidget);
        mouseDoubleClickWidget();
        setCurrentWidget(nullptr);
    } else if (currentAssociation) {
        setCurrentAssociation(currentAssociation);
        mouseDoubleClickAssociation();
        setCurrentAssociation(nullptr);
    } else {
        mouseDoubleClickEmpty();
    }
}

/**
 * Handler for mouse move events.
 * Events are delivered to the specific methods, depending on where the cursor
 * was pressed. It uses the current widget or association set in press event,
 * if any.
 * Then, the scene is scrolled if needed (if the cursor is moved in any of the
 * 30 pixels width area from left, top, right or bottom sides, and there is
 * more diagram currently not being shown in that direction).
 * This method is only called when mouse tracking is enabled and the mouse
 * is moved.
 *
 * @param ome The received event.
 */
void ToolBarState::mouseMove(QGraphicsSceneMouseEvent* ome)
{
    setMouseEvent(ome, QEvent::MouseMove);

    if (currentWidget()) {
        mouseMoveWidget();
    } else if (currentAssociation()) {
        mouseMoveAssociation();
    } else {
        mouseMoveEmpty();
    }
#if 0
    // scrolls the view
    static int mouseCount = 0;
    int vx = ome->scenePos().x();
    int vy = ome->scenePos().y();
    UMLView* view = m_pUMLScene->activeView();
//    QRectF maxArea = view->sceneRect();
    QRectF visibleArea = view->mapToScene(view->rect()).boundingRect();
    int dtr = visibleArea.x() + visibleArea.width() - vx;   // delta right
    int dtb = visibleArea.y() + visibleArea.height() - vy;  // delta bottom
    int dtt = vy - visibleArea.y();        // delta top
    int dtl = vx - visibleArea.x();        // delta left
//    uDebug() << "mouse [x, y] = [ " << vx << ", " << vy << "] / "
//             << "visibleArea [x, y, w, h] = [ " << visibleArea.x() << ", " << visibleArea.y() << ", " << visibleArea.width() << ", " << visibleArea.height() << "] / "
//             << "maxArea [x, y, w, h] = [ " << maxArea.x() << ", " << maxArea.y() << ", " << maxArea.width() << ", " << maxArea.height() << "] / "
//             << "delta right=" << dtr << ", bottom=" << dtb << ", top=" << dtt << ", left=" << dtl;
    if (dtr < 30) {
        logDebug0("ToolBarState::mouseMove translate RIGHT");
        view->ensureVisible(vx, vy, 0.1 /*30-dtr*/, 0, 2, 2);
    }
    if (dtb < 30) {
        mouseCount++;
        logDebug1("ToolBarState::mouseMove translate BOTTOM %1", mouseCount);
//        view->ensureVisible(vx, vy, 0, 0.1 /*30-dtb*/, 2,  2);
        if (mouseCount > 30) {
            view->verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
            mouseCount = 0;
        }
    }
    if (dtl < 30) { uDebug() << "translate LEFT";   view->ensureVisible(vx, vy, -0.1 /*-(30-dtl)*/, 0, 2, 2); }
    if (dtt < 30) { uDebug() << "translate TOP";    view->ensureVisible(vx, vy, 0, -0.1 /*-(30-dtt)*/, 2, 2); }
#endif
}

/**
 * An association was removed from the UMLScene.
 * If the association removed was the current association, the current
 * association is set to 0.
 * It can be extended in subclasses if needed.
 */
void ToolBarState::slotAssociationRemoved(AssociationWidget* association)
{
    if (association == currentAssociation()) {
        setCurrentAssociation(nullptr);
    }
}

/**
 * A widget was removed from the UMLScene.
 * If the widget removed was the current widget, the current widget is set
 * to nullptr.
 * It can be extended in subclasses if needed.
 */
void ToolBarState::slotWidgetRemoved(UMLWidget* widget)
{
    if (widget == currentWidget()) {
        setCurrentWidget(nullptr);
    }
}

/**
 * Creates a new ToolBarState.
 * UMLScene is set as parent of this QObject, and name is left empty.
 * Protected to avoid classes other than derived to create objects of this
 * class.
 *
 * @param umlScene The UMLScene to use.
 */
ToolBarState::ToolBarState(UMLScene *umlScene)
  : QObject(umlScene),
    m_pUMLScene(umlScene),
    m_pMouseEvent(nullptr)
{
    init();
}

/**
 * Sets the current association or widget.
 * It sets the current element when a press event happened. The element will
 * be used until the next release event.
 * Default implementation first checks for associations, then message widgets
 * and then any other widgets.
 * It can be overridden in subclasses if needed.
 */
void ToolBarState::setCurrentElement()
{
    // Check associations.
    AssociationWidget* association = associationAt(m_pMouseEvent->scenePos());
    if (association) {
        setCurrentAssociation(association);
        return;
    }

    // Check messages.
    //TODO check why message widgets are treated different
    MessageWidget* message = messageAt(m_pMouseEvent->scenePos());
    if (message) {
        setCurrentWidget(message);
        return;
    }

    //TODO check why message widgets are treated different
    FloatingDashLineWidget* floatingline = floatingLineAt(m_pMouseEvent->scenePos());
    if (floatingline) {
        setCurrentWidget(floatingline);
        return;
    }

    ObjectWidget* objectWidgetLine = m_pUMLScene->onWidgetDestructionBox(m_pMouseEvent->scenePos());
    if (objectWidgetLine) {
        setCurrentWidget(objectWidgetLine);
        return;
    }

    objectWidgetLine = m_pUMLScene->onWidgetLine(m_pMouseEvent->scenePos());
    if (objectWidgetLine) {
        setCurrentWidget(objectWidgetLine);
        return;
    }

    // Check widgets.
    UMLWidget *widget = m_pUMLScene->widgetAt(m_pMouseEvent->scenePos());
    if (widget) {
        setCurrentWidget(widget);
        return;
    }
}

/**
 * Called when the press event happened on an association.
 * Default implementation does nothing.
 */
void ToolBarState::mousePressAssociation()
{
}

/**
 * Called when the press event happened on a widget.
 * Default implementation does nothing.
 */
void ToolBarState::mousePressWidget()
{
}

/**
 * Called when the press event happened on an empty space.
 * Default implementation cleans the selection.
 */
void ToolBarState::mousePressEmpty()
{
    // TODO activate when QGraphicsScene selection handling is used
    //m_pUMLScene->clearSelection();
    m_pUMLScene->clearSelected();
}

/**
 * Called when the release event happened on an association.
 * Default implementation does nothing.
 */
void ToolBarState::mouseReleaseAssociation()
{
}

/**
 * Called when the release event happened on a widget.
 * Default implementation does nothing.
 */
void ToolBarState::mouseReleaseWidget()
{
}

/**
 * Called when the release event happened on an empty space.
 * Default implementation does nothing.
 */
void ToolBarState::mouseReleaseEmpty()
{
    if (m_currentWidget) {
        logDebug0("ToolBarState::mouseReleaseEmpty : m_currentWidget is set => ensureNestedVisible");
        Widget_Utils::ensureNestedVisible(m_currentWidget, m_pUMLScene->widgetList());
    }
}

/**
 * Called when the double click event happened on an association.
 * Default implementation does nothing.
 */
void ToolBarState::mouseDoubleClickAssociation()
{
}

/**
 * Called when the double click event happened on a widget.
 * Default implementation does nothing.
 */
void ToolBarState::mouseDoubleClickWidget()
{
}

/**
 * Called when the double click event happened on an empty space.
 * Default implementation cleans the selection.
 */
void ToolBarState::mouseDoubleClickEmpty()
{
    // TODO activate when QGraphicsScene selection handling is used
    //m_pUMLScene->clearSelection();
    m_pUMLScene->clearSelected();
}

/**
 * Called when the move event happened when an association is
 * currently available.
 * Default implementation does nothing.
 */
void ToolBarState::mouseMoveAssociation()
{
}

/**
 * Called when the move event happened when a widget is
 * currently available.
 * Default implementation does nothing.
 */
void ToolBarState::mouseMoveWidget()
{
}

/**
 * Called when the move event happened when no association nor
 * widget are currently available.
 * Default implementation does nothing.
 */
void ToolBarState::mouseMoveEmpty()
{
}

/**
 * Changes the current tool to the default one if the right button was released.
 * It can be overridden in subclasses if needed.
 */
void ToolBarState::changeTool()
{
    if (m_pMouseEvent->buttons() == Qt::RightButton) {
        UMLApp::app()->workToolBar()->setDefaultTool();
    }
}

/**
 * Returns the widget currently in use.
 *
 * @return The widget currently in use.
 */
UMLWidget* ToolBarState::currentWidget() const
{
    return m_currentWidget;
}

/**
 * Sets the widget currently in use.
 * This method is called in main press events handler just before calling
 * the press event for widgets handler.
 * Default implementation is set the specified widget, although this
 * behaviour can be overridden in subclasses if needed.
 *
 * @param widget The widget to be set.
 */
void ToolBarState::setCurrentWidget(UMLWidget* widget)
{
    m_currentWidget = widget;
}

/**
 * Returns the association currently in use.
 *
 * @return The association currently in use.
 */
AssociationWidget* ToolBarState::currentAssociation() const
{
    return m_currentAssociation;
}

/**
 * Sets the association currently in use.
 * This method is called in main press events handler just before calling
 * the press event for associations handler.
 * Default implementation is set the specified association, although this
 * behaviour can be overridden in subclasses if needed.
 *
 * @param association The association to be set.
 */
void ToolBarState::setCurrentAssociation(AssociationWidget* association)
{
    m_currentAssociation = association;
}

/**
 * Sets m_pMouseEvent as the equivalent of the received event after transforming it
 * using the inverse world matrix in the UMLScene.
 * This method is called at the beginning of the main event handler methods.
 *
 * @param ome The mouse event to transform.
 * @param type The type of the event.
 */
void ToolBarState::setMouseEvent(QGraphicsSceneMouseEvent* ome, const QEvent::Type &type)
{
    delete m_pMouseEvent;

    //:TODO: uDebug() << "[PORT] Check if scenePos works like view->inverseWorldMatrix().map()";
    // Using copy constructor here.
    m_pMouseEvent = new QGraphicsSceneMouseEvent(type);
    m_pMouseEvent->setPos(ome->pos());
    m_pMouseEvent->setScenePos(ome->scenePos());
    m_pMouseEvent->setScreenPos(ome->screenPos());
    m_pMouseEvent->setLastPos(ome->lastPos());
    m_pMouseEvent->setLastScenePos(ome->lastScenePos());
    m_pMouseEvent->setLastScreenPos(ome->lastScreenPos());
    m_pMouseEvent->setButtons(ome->buttons());
    m_pMouseEvent->setButton(ome->button());
    m_pMouseEvent->setModifiers(ome->modifiers());
}

/**
 * Returns the MessageWidget at the specified position, or null if there is none.
 * The message is only returned if it is visible.
 * If there are more than one message at this point, it returns the first found.
 *
 * @param pos The position to get the message.
 * @return The MessageWidget at the specified position, or null if there is none.
 * @todo Better handling for messages at the same point
 */
MessageWidget* ToolBarState::messageAt(const QPointF& pos)
{
    for(MessageWidget* message : m_pUMLScene->messageList()) {
        if (message->isVisible() && message->onWidget(pos)) {
            return message;
        }
    }
    return nullptr;
}

/**
 * Returns the AssociationWidget at the specified position, or null if there is none.
 * If there are more than one association at this point, it returns the first found.
 *
 * @param pos The position to get the association.
 * @return The AssociationWidget at the specified position, or null if there is none.
 * @todo Better handling for associations at the same point
 */
AssociationWidget* ToolBarState::associationAt(const QPointF& pos)
{
    for(AssociationWidget* association : m_pUMLScene->associationList()) {
        if (association->onAssociation(pos)) {
            return association;
        }
    }
    return nullptr;
}

/**
 * Returns the FloatingDashLineWidget at the specified position, or null if there is none.
 * The floatingdashline is only returned if it is visible.
 *
 * @param pos The position to get the floatingLine.
 * @return The MessageWidget at the specified position, or null if there is none.
 */
FloatingDashLineWidget* ToolBarState::floatingLineAt(const QPointF& pos)
{
    FloatingDashLineWidget *floatingline = nullptr;

    for(UMLWidget* widget : m_pUMLScene->widgetList()) {
        uIgnoreZeroPointer(widget);
        if (widget->isFloatingDashLineWidget()){
            if (widget->asFloatingDashLineWidget()->onLine(pos)) {
                floatingline = widget->asFloatingDashLineWidget();
            }
        }
    }

    return floatingline;
}

