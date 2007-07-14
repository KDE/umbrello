/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006                                                    *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLWIDGETCONTROLLER_H
#define UMLWIDGETCONTROLLER_H

// qt includes
#include <qdatetime.h>

// app includes
#include "umlwidgetlist.h"

class QCursor;
class QMouseEvent;
class QMoveEvent;
class QPoint;

class UMLWidget;

/**
 * Controller for UMLWidget
 * This class takes care of user interaction with UMLWidgets: select, deselect,
 * move, resize...
 * Those actions are done using events. There are handlers for mousePressEvent,
 * mouseMoveEvent, mouseReleaseEvent and mouseDoubleClickEvent. There's more
 * information about each of them in their respective documentation.
 *
 * Behaviour of this class can be customized overriding the handlers themselves
 * (which isn't recommended) or, better, overriding the virtual protected
 * methods.
 *
 * The area that is considered as "resize area" can be customized with
 * isInResizeArea, so when there's a pressed button in this area, mouseMoveEvent
 * will resize the widget.
 * Also, if the resize area doesn't need to be modified, but the cursor to be
 * used when the mouse is in that area can be done with getResizeCursor.
 * When a widget is being resized, it's done using resizeWidget, so overriding
 * it makes possible to, for example, constrain the resizing only in one axis
 * no matter how the mouse was moved.
 *
 * Widget move can also be customized. The widgets are moved in mouseMoveEvent
 * using the moveWidgetBy method of the controller of each selected widget.
 * Overriding this method widget movement can be, for example, constrained to
 * an axis, no matter if the widget is being moved explicitly or as part of a
 * selection.
 * On the other hand, the move of all the selected widgets can be constrained
 * with constrainMovementForAllWidgets. This method, called in the controller
 * that is handling the mouseMoveEvent, modifies the difference between the
 * current position of the widgets and the new position to be moved to. For
 * example, if a widget shouldn't be moved in X axis, it's receiving the
 * mouseMoveEvents and there are other widgets selected, those other widgets
 * shouldn't be allowed either to be moved in X axis.
 *
 * The behaviour when double clicking on the widget after it's selected can be
 * customized with doMouseDoubleClick.
 *
 * @author Umbrello UML Modeller Authors <uml-devel@lists.sourceforge.net>
 */
class UMLWidgetController {
public:

    /**
     * Constructor for UMLWidgetController.
     *
     * @param widget The widget which uses the controller.
     */
    UMLWidgetController(UMLWidget *widget);

    /**
     * Destructor for UMLWidgetController.
     */
    virtual ~UMLWidgetController();

    /**
     * Handles a mouse press event.
     * It'll select the widget (or mark it to be deselected) and prepare it to
     * be moved or resized. Go on reading for more info about this.
     *
     * Widget values and message bar status are saved.
     *
     * If shift or control buttons are pressed, we're in move area no matter
     * where the button was pressed in the widget. Moreover, if the widget
     * wasn't already selected, it's added to the selection. If already selected,
     * it's marked to be deselected when releasing the button (provided it isn't
     * moved).
     * Also, if the widget is already selected with other widgets but shift nor
     * control buttons are pressed, we're in move area. If finally we don't move
     * the widget, it's selected and the other widgets deselected when releasing
     * the left button.
     *
     * If shift nor control buttons are pressed, we're facing a single selection.
     * Depending on the position of the cursor, we're in move or in resize area.
     * If the widget wasn't selected (both when there are no widgets selected, or
     * when there're other widgets selected but not the one receiving the press
     * event) it's selected and the others deselected, if any. If already selected,
     * it's marked to be deselected when releasing the button (provided it wasn't
     * moved or resized).
     *
     * @param me The QMouseEvent event.
     */
    virtual void mousePressEvent(QMouseEvent *me);

    /**
     * Handles a mouse move event.
     * It resizes or moves the widget, depending on where the cursor is pressed
     * on the widget. Go on reading for more info about this.
     *
     * If resizing, the widget is resized using resizeWidget (where specific
     * widget resize constrain can be applied), and then the associations are
     * adjusted.
     * The resizing can be constrained also to an specific axis using control
     * and shift buttons. If on or another is pressed, it's constrained to X axis.
     * If both are pressed, it's constrained to Y axis.
     *
     * If not resizing, the widget is being moved. If the move is being started,
     * the selection bounds are set (which includes updating the list of selected
     * widgets).
     * The difference between the previous position of the selection and the new
     * one is got (taking in account the selection bounds so widgets don't go
     * beyond the canvas limits). Then, it's constrained to X or Y axis depending
     * on shift and control buttons.
     * A further constrain is made using constrainMovementForAllWidgets (for example,
     * if the widget that receives the event can only be moved in Y axis, with this
     * method the movement of all the widgets in the selection can be constrained to
     * be moved only in Y axis).
     * Then, all the selected widgets are moved using moveWidgetBy (where specific
     * widget movement constrain can be applied) and, if an specific amount of time
     * passed from the last move event, the associations are also updated (they're
     * not updated always to be easy on the CPU). Finally, the canvas is resized,
     * and selection bounds updated.
     *
     * @param me The QMouseEvent event.
     */
    virtual void mouseMoveEvent(QMouseEvent* me);

    /**
     * Handles a mouse release event.
     * It selects or deselects the widget and cancels or confirms the move or
     * resize. Go on reading for more info about this.
     * No matter which tool is selected, Z position of widget is updated.
     *
     * Middle button release resets the selection.
     * Left button release, if it wasn't moved nor resized, selects the widget
     * and deselect the others if it wasn't selected and there were other widgets
     * selected. If the widget was marked to be deselected, deselects it.
     * If it was moved or resized, the document is set to modified if position
     * or size changed. Also, if moved, all the associations are adjusted because
     * the timer could have prevented the adjustment in the last move event before
     * the release.
     * If mouse was pressed in resize area, cursor is set again to normal cursor
     * Right button release if right button was pressed shows the pop up menu for
     * the widget.
     * If left button was pressed, it cancels the move or resize with a mouse move
     * event at the same position than the cursor was when pressed. Another left
     * button release is also sent.
     *
     * @param me The QMouseEvent event.
     */
    virtual void mouseReleaseEvent(QMouseEvent * me);

    /**
     * Handles a mouse double click event.
     * If the button wasn't left button it does nothing. Otherwise, it selects
     * the widget (deselecting other selected widgets, if any) and executes
     * doMouseDoubleClick.
     * @see doMouseDoubleClick
     *
     * @param me The QMouseEvent event.
     */
    virtual void mouseDoubleClickEvent(QMouseEvent *me);

protected:

    /**
     * Saves the values of the widget needed for move/resize.
     * The values saved are: the offset from the cursor respect to the upper left
     * corner of the widget in m_pressOffsetX/Y, the position in m_oldX/Y and the
     * size in m_oldW/H.
     *
     * It can be overridden to save subclass specific values whenever a move or
     * resize begins. However, parent method (that is, this method) must be
     * called in the overridden method.
     *
     * @param me The QMouseEvent to get the offset from.
     */
    virtual void saveWidgetValues(QMouseEvent *me);

    /**
     * Checks if the mouse is in resize area (right bottom corner), and sets
     * the cursor depending on that.
     * The cursor used when resizing is gotten from getResizeCursor().
     *
     * @param me The QMouseEVent to check.
     * @return true if the mouse is in resize area, false otherwise.
     */
    virtual bool isInResizeArea(QMouseEvent *me);

    /**
     * Returns the cursor to be shown when resizing the widget.
     *
     * Default cursor is KCursor::sizeFDiagCursor().
     *
     * @return The cursor to be shown when resizing the widget.
     */
    virtual QCursor getResizeCursor();

    /**
     * Resizes the widget.
     * It's called from resize, after the values are constrained and before
     * the associations are adjusted.
     *
     * Default behaviour is resize the widget using the new size values.
     * @see resize
     *
     * @param newW The new width for the widget.
     * @param newH The new height for the widget.
     */
    virtual void resizeWidget(int newW, int newH);

    /**
     * Moves the widget to a new position using the difference between the
     * current position and the new position.
     * This method doesn't adjust associations. It only moves the widget.
     *
     * It can be overridden to constrain movement of m_widget only in one axis even when
     * the user isn't constraining the movement with shift or control buttons, for example.
     * The movement policy set here is applied whenever the widget is moved, being it
     * moving it explicitly, or as a part of a selection but not receiving directly the
     * mouse events.
     *
     * Default behaviour is move the widget to the new position using the diffs.
     * @see constrainMovementForAllWidgets
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     */
    virtual void moveWidgetBy(int diffX, int diffY);

    /**
     * Modifies the value of the diffX and diffY variables used to move the widgets.
     *
     * It can be overridden to constrain movement of all the selected widgets only in one
     * axis even when the user isn't constraining the movement with shift or control
     * buttons, for example.
     * The difference with moveWidgetBy is that the diff positions used here are
     * applied to all the selected widgets instead of only to m_widget, and that
     * moveWidgetBy, in fact, moves the widget, and here simply the diff positions
     * are modified.
     *
     * Default behaviour is do nothing.
     * @see moveWidgetBy
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     */
    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);

    /**
     * Executes the action for double click in the widget.
     * It's called only if the button used was left button.
     * Before calling this method, the widget is selected.
     *
     * Default behaviour is show the properties dialog for the widget using
     * m_widget->slotMenuSelection(ListPopupMenu::mt_Properties);
     * If the widget doesn't have a property dialog (from the Widget_Type enum, those that
     * don't have an UMLObject representation) there's no need to override
     * the method, it simply does nothing.
     *
     * @param me The QMouseEvent which triggered the double click event.
     */
    virtual void doMouseDoubleClick(QMouseEvent *me);

    /**
     * Clears the selection, resets the toolbar and deselects the widget.
     */
    void resetSelection();

    /**
     * Selects the widget and clears the other selected widgets, if any.
     *
     * @param me The QMouseEvent which made the selection.
     */
    void selectSingle(QMouseEvent *me);

    /**
     * Selects the widget and adds it to the list of selected widgets.
     *
     * @param me The QMouseEvent which made the selection.
     */
    void selectMultiple(QMouseEvent *me);

    /**
     * Deselects the widget and removes it from the list of selected widgets.
     *
     * @param me The QMouseEvent which made the selection.
     */
    void deselect(QMouseEvent *me);

    /**
     * Fills m_selectedWidgetsList and sets the selection bounds ((m_min/m_max)X/Y attributes).
     */
    void setSelectionBounds();

    /**
     * Updates the selection bounds based on the movement made.
     * If it was only a vertical movement, there's no need to update horizontal bounds,
     * and vice versa.
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     */
    void updateSelectionBounds(int diffX, int diffY);

    /**
     * Resizes the widget and adjusts the associations.
     * It's called when a mouse move event happens and the cursor was
     * in resize area when pressed.
     * Resizing can be constrained to an specific axis using control and shift buttons.
     *
     * @param me The QMouseEvent to get the values from.
     */
    void resize(QMouseEvent *me);

    /**
     * Returns the smallest X position of all the widgets in the list.
     *
     * @param widgetList A list with UMLWidgets.
     * @return The smallest X position.
     */
    int getSmallestX(const UMLWidgetList &widgetList);

    /**
     * Returns the smallest Y position of all the widgets in the list.
     *
     * @param widgetList A list with UMLWidgets.
     * @return The smallest Y position.
     */
    int getSmallestY(const UMLWidgetList &widgetList);

    /**
     * Returns the biggest X position of all the widgets in the list.
     *
     * @param widgetList A list with UMLWidgets.
     * @return The biggest X position.
     */
    int getBiggestX(const UMLWidgetList &widgetList);

    /**
     * Returns the biggest Y position of all the widgets in the list.
     *
     * @param widgetList A list with UMLWidgets.
     * @return The biggest Y position.
     */
    int getBiggestY(const UMLWidgetList &widgetList);

    /**
     * Returns the adjusted position for the given mouse event.
     * The adjusted position is computed using the current widget position
     * m_widget->get{X,Y}(), the previous position m_old{X,Y}, and the
     * mouse press offset m_pressOffset{X,Y}.
     *
     * @param me The QMouseEvent for which to get the adjusted position.
     * @return A QPoint with the adjusted position.
     */
    QPoint getPosition(QMouseEvent *me);

    /**
     * Returns a QPoint with the new X and Y position difference of the mouse event
     * respect to the position of the widget.
     *
     * @param me The QMouseEvent to get the position to compare.
     * @return A QPoint with the position difference.
     */
    QPoint getPositionDifference(QMouseEvent *me);

    /**
     * Shows the widget popup menu where the mouse event points to.
     *
     * @param me The QMouseEvent which triggered the showing.
     */
    void showPopupMenu(QMouseEvent *me);

    /**
     * Checks if the size of the widget changed respect to the size that
     * it had when press event was fired.
     *
     * @return true if was resized, false otherwise.
     */
    bool wasSizeChanged();

    /**
     * Checks if the position of the widget changed respect to the position that
     * it had when press event was fired.
     *
     * @return true if was moved, false otherwise.
     */
    bool wasPositionChanged();

    /**
     * The widget which uses the controller.
     */
    UMLWidget *m_widget;

    /**
     * Timer that prevents excessive updates (be easy on the CPU).
     */
    QTime lastUpdate;

    /**
     * A list containing the selected widgets.
     * It's filled by setSelectionBounds method. It must be filled again if
     * selected widgets changed. It is cleared only in setSelectionBounds, just
     * before filling it.
     * Select, deselect and so on methods DON'T modify this list.
     */
    UMLWidgetList m_selectedWidgetsList;

    /**
     * The text in the status bar when the cursor was pressed.
     */
    QString m_oldStatusBarMsg;

    /**
     * The X/Y offset from the position of the cursor when it was pressed to the
     * upper left corner of the widget.
     */
    int m_pressOffsetX, m_pressOffsetY;

    /**
     * The X/Y position the widget had when the movement started.
     */
    int m_oldX, m_oldY;

    /**
     * The width/height the widget had when the resize started.
     */
    int m_oldW, m_oldH;

    /**
     * The minimum/maximum X/Y position of all the selected widgets.
     */
    int m_minSelectedX, m_minSelectedY, m_maxSelectedX, m_maxSelectedY;

    /**
     * If shift or control button were pressed in mouse press event.
     */
    bool m_shiftPressed;

    /**
     * If the left/middle/right button is pressed.
     */
    bool m_leftButtonDown, m_middleButtonDown, m_rightButtonDown;

    /**
     * If cursor was in move/resize area when left button was pressed (and no
     * other widgets were selected).
     */
    bool m_inMoveArea, m_inResizeArea;

    /**
     * If the widget was selected/moved/resized in the press and release cycle.
     * Moved/resized is true if the widget was moved/resized even if the final
     * position/size is the same as the starting one.
     */
    bool m_wasSelected, m_moved, m_resized;
};

#endif
