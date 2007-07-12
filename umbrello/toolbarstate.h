/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2004-2006                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef TOOLBARSTATE_H
#define TOOLBARSTATE_H

#include <qevent.h>
#include <qobject.h>

class QEvent;
class QMouseEvent;

class AssociationWidget;
class MessageWidget;
class UMLView;
class UMLWidget;


/**
 * Base class for toolbar states.
 * All toolbar states inherit directly or indirectly from this class. Toolbar
 * states represent tools that work with the diagram (for example, to create
 * widgets, make associations...). All the mouse events received in the diagram
 * are delivered to the toolbar state currently active. The events are handled
 * in the tool and it executes the needed actions.
 *
 * All the mouse event handlers can be overridden in subclasses. However, the
 * behaviour of the main handlers shouldn't be modified (apart from extend it,
 * that is, call the base implementation before any other actions in the derived
 * method).
 *
 * In order to handle the events, each main handler has three protected
 * "sub-handlers" named like the main handler with the suffixes "Association",
 * "Wdiget" and "Empty". The events received in the main handlers are delivered
 * to the suitable handler, depending on if the event happened on an association,
 * on a widget or on an empty space of the diagram. Those methods are the ones to
 * override or extend to specify the behaviour of the toolbar state.
 *
 * The mouse events received in main handlers are tweaked to use the inverse
 * position. The modified event is saved in m_pMouseEvent. This is the event that
 * must be used everywhere.
 *
 * The association or widget that will receive the events is set in press event.
 * How they are set can be tweaked in subclasses overriding setCurrentElement().
 * Once a press event happens, all the mouse events and the release event are sent
 * to the same widget or association. Mouse events are delivered only when mouse
 * tracking is enabled. It is enabled in press event, and disabled in release
 * event. Also, it is disabled in the toolbar state initialization. Additionally,
 * it can be enabled or disabled in other situations by subclasses if needed.
 *
 * After handling a release event, the tool is changed if needed. Default
 * implementation sets the default tool if the button released was the right
 * button. Subclasses can override this behaviour if needed.
 *
 * When a toolbar state is selected, method init is called to revert its state
 * to the initial. Subclasses should extend that method as needed. Also, method
 * cleanBeforeChange() is called before changing it to the new tool. Subclasses
 * should extend that method as needed.
 *
 * @todo Handle, for example, left press, right press, left release, right
 *       release and other similar strange combinations?
 */
class ToolBarState: public QObject {
    Q_OBJECT
public:

    /**
     * Destroys this ToolBarState.
     * Frees m_pMouseEvent.
     */
    virtual ~ToolBarState();

    /**
     * Goes back to the initial state.
     * Subclasses can extend, but not override, this method as needed.
     */
    virtual void init();

    /**
     * Called when the current tool is changed to use another tool.
     * Subclasses can extend, but not override, this method as needed.
     * Default implementation does nothing.
     */
    virtual void cleanBeforeChange();

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
    virtual void mousePress(QMouseEvent *ome);

    /**
     * Handler for mouse release events.
     * Mouse tracking is disabled and the position of the cursor set.
     * The events are delivered to the specific methods, depending on where the
     * cursor was released, and the current association or widget cleaned.
     * Finally, the current tool is changed if needed.
     *
     * @param ome The received event.
     */
    virtual void mouseRelease(QMouseEvent* ome);

    /**
     * Handler for mouse double click events.
     * The current association or widget is set (if any), and events are
     * delivered to the specific methods, depending on where the cursor was pressed.
     * After delivering the events, the current association or widget is cleaned.
     *
     * @param ome The received event.
     */
    virtual void mouseDoubleClick(QMouseEvent* ome);

    /**
     * Handler for mouse double click events.
     * Events are delivered to the specific methods, depending on where the cursor
     * was pressed. It uses the current widget or association set in press event,
     * if any.
     * Then, the view is scrolled if needed (if the cursor is moved in any of the
     * 30 pixels width area from left, top, right or bottom sides, and there is
     * more diagram currently not being shown in that direction).
     * This method is only called when mouse tracking is enabled and the mouse
     * is moved.
     *
     * @param ome The received event.
     */
    virtual void mouseMove(QMouseEvent* ome);

public slots:

    /**
     * An association was removed from the UMLView.
     * If the association removed was the current association, the current
     * association is set to 0.
     * It can be extended in subclasses if needed.
     */
    virtual void slotAssociationRemoved(AssociationWidget* association);

    /**
     * A widget was removed from the UMLView.
     * If the widget removed was the current widget, the current widget is set
     * to 0.
     * It can be extended in subclasses if needed.
     */
    virtual void slotWidgetRemoved(UMLWidget* widget);

protected:

    /**
     * Creates a new ToolBarState.
     * UMLView is set as parent of this QObject, and name is left empty.
     * Protected to avoid classes other than derived to create objects of this
     * class.
     *
     * @param umlView The UMLView to use.
     */
    ToolBarState(UMLView *umlView);

    /**
     * Sets the current association or widget.
     * It sets the current element when a press event happened. The element will
     * be used until the next release event.
     * Default implementation first checks for associations, then message widgets
     * and then any other widgets.
     * It can be overridden in subclasses if needed.
     */
    virtual void setCurrentElement();

    /**
     * Called when the press event happened on an association.
     * Default implementation does nothing.
     */
    virtual void mousePressAssociation();

    /**
     * Called when the press event happened on a widget.
     * Default implementation does nothing.
     */
    virtual void mousePressWidget();

    /**
     * Called when the press event happened on an empty space.
     * Default implementation cleans the selection.
     */
    virtual void mousePressEmpty();

    /**
     * Called when the release event happened on an association.
     * Default implementation does nothing.
     */
    virtual void mouseReleaseAssociation();

    /**
     * Called when the release event happened on a widget.
     * Default implementation does nothing.
     */
    virtual void mouseReleaseWidget();

    /**
     * Called when the release event happened on an empty space.
     * Default implementation does nothing.
     */
    virtual void mouseReleaseEmpty();

    /**
     * Called when the double click event happened on an association.
     * Default implementation does nothing.
     */
    virtual void mouseDoubleClickAssociation();

    /**
     * Called when the double click event happened on a widget.
     * Default implementation does nothing.
     */
    virtual void mouseDoubleClickWidget();

    /**
     * Called when the double click event happened on an empty space.
     * Default implementation cleans the selection.
     */
    virtual void mouseDoubleClickEmpty();

    /**
     * Called when the move event happened when an association is
     * currently available.
     * Default implementation does nothing.
     */
    virtual void mouseMoveAssociation();

    /**
     * Called when the move event happened when a widget is
     * currently available.
     * Default implementation does nothing.
     */
    virtual void mouseMoveWidget();

    /**
     * Called when the move event happened when no association nor
     * widget are currently available.
     * Default implementation does nothing.
     */
    virtual void mouseMoveEmpty();

    /**
     * Changes the current tool to the default one if the right button was released.
     * It can be overridden in subclasses if needed.
     */
    virtual void changeTool();

    /**
     * Returns the widget currently in use.
     *
     * @return The widget currently in use.
     */
    virtual UMLWidget* getCurrentWidget() {
        return m_currentWidget;
    }

    /**
     * Sets the widget currently in use.
     * This method is called in main press events handler just before calling
     * the press event for widgets handler.
     * Default implementation is set the specified widget, although this
     * behaviour can be overridden in subclasses if needed.
     *
     * @param currentWidget The widget to be set.
     */
    virtual void setCurrentWidget(UMLWidget* currentWidget) {
        m_currentWidget = currentWidget;
    }

    /**
     * Returns the association currently in use.
     *
     * @return The association currently in use.
     */
    virtual AssociationWidget* getCurrentAssociation() {
        return m_currentAssociation;
    }

    /**
     * Sets the association currently in use.
     * This method is called in main press events handler just before calling
     * the press event for associations handler.
     * Default implementation is set the specified association, although this
     * behaviour can be overridden in subclasses if needed.
     *
     * @param currentAssociation The association to be set.
     */
    virtual void setCurrentAssociation(AssociationWidget* currentAssociation) {
        m_currentAssociation = currentAssociation;
    }

    /**
     * Sets m_pMouseEvent as the equivalent of the received event after transforming it
     * using the inverse world matrix in the UMLView.
     * This method is called at the beginning of the main event handler methods.
     *
     * @param ome The mouse event to transform.
     * @param type The type of the event.
     */
    void setMouseEvent(QMouseEvent* ome, const QEvent::Type &type);

    /**
     * Returns the AssociationWidget at the specified position, or null if there is none.
     * If there are more than one association at this point, it returns the first found.
     *
     * @param pos The position to get the association.
     * @return The AssociationWidget at the specified position, or null if there is none.
     * @todo Better handling for associations at the same point
     */
    AssociationWidget* getAssociationAt(const QPoint& pos);

    /**
     * Returns the MessageWidget at the specified position, or null if there is none.
     * The message is only returned if it is visible.
     * If there are more than one message at this point, it returns the first found.
     *
     * @param pos The position to get the message.
     * @return The MessageWidget at the specified position, or null if there is none.
     * @todo Better handling for messages at the same point
     */
    MessageWidget* getMessageAt(const QPoint& pos);

    /**
     * The UMLView.
     */
    UMLView* m_pUMLView;

    /**
     * The mouse event currently in use.
     * This event is the equivalent of the received event after transforming it
     * using the inverse world matrix in the UMLView.
     */
    QMouseEvent* m_pMouseEvent;

private:

    /**
     * The widget currently in use, if any.
     */
    UMLWidget* m_currentWidget;

    /**
     * The association currently in use, if any.
     */
    AssociationWidget* m_currentAssociation;

};

#endif //TOOLBARSTATE_H
