/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2011                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "umlwidgetcontroller.h"

// qt includes
#include <QtCore/QEvent>
#include <QtCore/QPoint>

// kde includes
#include <kcursor.h>
#include <klocale.h>

// app includes
#include "umlwidget.h"
#include "umlwidgetlist.h"
#include "uml.h"
#include "umldoc.h"
#include "umlview.h"
#include "umlobject.h"
#include "listpopupmenu.h"
#include "associationwidget.h"
#include "cmds.h"

/**
 * Constructor for UMLWidgetController.
 *
 * @param widget The widget which uses the controller.
 */
UMLWidgetController::UMLWidgetController(UMLWidget *widget)
{
    m_widget = widget;

    m_oldX = m_oldY = 0;
    m_pressOffsetX = m_pressOffsetY = 0;
    m_prevX = m_prevY = 0;
    m_oldW = m_oldH = 0;

    m_minSelectedX = m_minSelectedY = m_maxSelectedX = m_maxSelectedY = 0;

    m_shiftPressed = false;
    m_leftButtonDown = m_middleButtonDown = m_rightButtonDown = false;
    m_inMoveArea = m_inResizeArea = 0;
    m_wasSelected = m_moved = m_resized = 0;
}

/**
 * Destructor for UMLWidgetController.
 */
UMLWidgetController::~UMLWidgetController()
{
}

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
void UMLWidgetController::mousePressEvent(QMouseEvent *me)
{
    // If there is a button pressed already ignore other press events
    if (m_leftButtonDown || m_middleButtonDown || m_rightButtonDown) {
        return;
    }

    if (me->button() == Qt::LeftButton) {
        m_leftButtonDown = true;
        //saveWidgetValues(me);
    } else if (me->button() == Qt::RightButton) {
        m_rightButtonDown = true;
    } else {
        m_middleButtonDown = true;
        return;
    }

    //There is no harm in saving all the values of the widget even when
    //they aren't going to be used
    saveWidgetValues(me);

    m_oldStatusBarMsg = UMLApp::app()->statusBarMsg();

    if (me->modifiers() == Qt::ShiftModifier || me->modifiers() == Qt::ControlModifier) {
        m_shiftPressed = true;

        if (me->button() == Qt::LeftButton) {
            m_inMoveArea = true;
        }

        if (!m_widget->isSelected()) {
            selectMultiple(me);
        } else if (!m_rightButtonDown) {
            m_wasSelected = false;
        }
        return;
    }

    m_shiftPressed = false;

    int count = m_widget->m_scene->getSelectCount(true);
    if (me->button() == Qt::LeftButton) {
        if (m_widget->isSelected() && count > 1) {
            //Single selection is made in release event if the widget wasn't moved
            m_inMoveArea = true;
            m_lastUpdate.start();
            return;
        }

        if (isInResizeArea(me)) {
            m_inResizeArea = true;
        } else {
            m_inMoveArea = true;
        }
    }

    //If widget wasn't selected, or it was selected but with other widgets also selected
    if (!m_widget->isSelected() || count > 1) {
        selectSingle(me);
    } else if (!m_rightButtonDown) {
        m_wasSelected = false;
    }
}

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
void UMLWidgetController::mouseMoveEvent(QMouseEvent* me)
{
    if (!m_leftButtonDown)
        return;

    if (m_inResizeArea) {
        resize(me);
        return;
    }

    if (!m_moved) {
        UMLApp::app()->document()->writeToStatusBar(i18n("Hold shift or ctrl to move in X axis. Hold shift and control to move in Y axis. Right button click to cancel move."));

        m_moved = true;
        //Maybe needed by AssociationWidget
        m_widget->m_startMove = true;

        setSelectionBounds();
    }

    QPoint position = getPosition(me);
    int diffX = position.x() - m_widget->getX();
    int diffY = position.y() - m_widget->getY();

    if ((me->modifiers() & Qt::ShiftModifier) && (me->modifiers() & Qt::ControlModifier)) {
        //Move in Y axis
        diffX = 0;
    } else if ((me->modifiers() & Qt::ShiftModifier) || (me->modifiers() & Qt::ControlModifier)) {
        //Move in X axis
        diffY = 0;
    }

    /* Commands
        UMLDoc* doc = UMLApp::app()->document();
        doc->executeCommand(new CmdMoveWidget(this,diffX,diffY));*/

    // moveWidget(diffX,diffY);

    // uDebug() << "before constrainMovementForAllWidgets:"
    //     << " diffX=" << diffX << ", diffY=" << diffY << endl;
    constrainMovementForAllWidgets(diffX, diffY);
    // uDebug() << "after constrainMovementForAllWidgets:"
    //     << " diffX=" << diffX << ", diffY=" << diffY << endl;

    //Nothing to move
    if (diffX == 0 && diffY == 0) {
        return;
    }

    bool update = false;
    if (m_lastUpdate.elapsed() > 25) {
        update = true;
        m_lastUpdate.restart();

        m_widget->adjustUnselectedAssocs(m_widget->getX(), m_widget->getY());
    }

    foreach(UMLWidget* widget , m_selectedWidgetsList) {

        //UMLDoc* m_doc = UMLApp::app()->document();
        //CmdMoveWidgetBy* cmd = new CmdMoveWidgetBy(widget,diffX,diffY);
        //m_doc->executeCommand(cmd);
        //m_doc->executeCommand(new CmdMoveWidgetBy(widget,diffX,diffY));
        widget->getWidgetController()->moveWidgetBy(diffX, diffY);
    }
    // uDebug();

    // Move any selected associations.

    foreach(AssociationWidget* aw, m_widget->m_scene->getSelectedAssocs()) {
        if (aw->getSelected()) {
            aw->moveEntireAssoc(diffX, diffY);
        }
    }

    m_widget->m_scene->resizeCanvasToItems();
    updateSelectionBounds(diffX, diffY);
}

void UMLWidgetController::widgetMoved()
{
    //reverseOldNewValues();
    m_moved = false;

    //Ensure associations are updated (the timer could prevent the
    //adjustment in the last move event before the release)

    foreach(UMLWidget* widget , m_selectedWidgetsList) {

        widget->adjustAssocs(widget->getX(), widget->getY());
    }

    m_widget->m_startMove = false;
}

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
void UMLWidgetController::mouseReleaseEvent(QMouseEvent *me)
{
    if (me->button() != Qt::LeftButton && me->button() != Qt::RightButton) {
        if (m_middleButtonDown) {
            m_middleButtonDown = false;
            resetSelection();
        }
    } else if (me->button() == Qt::LeftButton) {
        if (m_leftButtonDown) {
            m_leftButtonDown = false;

            if (!m_moved && !m_resized) {
                if (!m_shiftPressed && (m_widget->m_scene->getSelectCount(true) > 1)) {
                    selectSingle(me);
                } else if (!m_wasSelected) {
                    deselect(me);
                }
            } else {
                if (m_moved) {

                    /* Commands */
                    UMLApp::app()->executeCommand(new Uml::CmdMoveWidget(this));

                } else {
                    UMLApp::app()->executeCommand(new Uml::CmdResizeWidget(this));
                    m_resized = false;
                }

                if ((m_inMoveArea && wasPositionChanged()) ||
                        (m_inResizeArea && wasSizeChanged())) {
                    m_widget->umlDoc()->setModified(true);
                }

                UMLApp::app()->document()->writeToStatusBar(m_oldStatusBarMsg);
            }

            if (m_inResizeArea) {
                m_inResizeArea = false;
                m_widget->m_scene->setCursor(Qt::ArrowCursor);
            } else {
                m_inMoveArea = false;
            }
        }
    } else if (me->button() == Qt::RightButton) {
        if (m_rightButtonDown) {
            m_rightButtonDown = false;
            showPopupMenu(me);
        } else if (m_leftButtonDown) {
            //Cancel move/edit
            QMouseEvent move(QMouseEvent::MouseMove,
                             QPoint(m_oldX + m_pressOffsetX, m_oldY + m_pressOffsetY),
                             Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            mouseMoveEvent(&move);
            QMouseEvent release(QMouseEvent::MouseButtonRelease,
                                QPoint(m_oldX + m_pressOffsetX, m_oldY + m_pressOffsetY),
                                Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
            mouseReleaseEvent(&release);
        }
    }

    //TODO Copied from old code. Does it really work as intended?
    UMLWidget *bkgnd = m_widget->m_scene->widgetAt(me->pos());
    if (bkgnd) {
        //uDebug() << "setting Z to " << bkgnd->getZ() + 1;
        m_widget->setZ(bkgnd->getZ() + 1);
    } else {
        m_widget->setZ(0);
    }
}

UMLWidget* UMLWidgetController::getWidget()
{
    return m_widget;
}

/**
 * Handles a mouse double click event.
 * If the button wasn't left button it does nothing. Otherwise, it selects
 * the widget (deselecting other selected widgets, if any) and executes
 * doMouseDoubleClick.
 * @see doMouseDoubleClick
 *
 * @param me The QMouseEvent event.
 */
void UMLWidgetController::mouseDoubleClickEvent(QMouseEvent *me)
{
    if (me->button() != Qt::LeftButton) {
        return;
    }

    selectSingle(me);

    doMouseDoubleClick(me);
}

/**
 * Checks if the mouse is in resize area (right bottom corner), and sets
 * the cursor depending on that.
 * The cursor used when resizing is gotten from getResizeCursor().
 *
 * @param me The QMouseEVent to check.
 * @return true if the mouse is in resize area, false otherwise.
 */
bool UMLWidgetController::isInResizeArea(QMouseEvent *me)
{
    const int m = 10;

    if (m_widget->m_resizable &&
            me->x() >= (m_widget->getX() + m_widget->width() - m) &&
            me->y() >= (m_widget->getY() + m_widget->height() - m)) {
        m_widget->m_scene->setCursor(getResizeCursor());
        return true;
    } else {
        m_widget->m_scene->setCursor(Qt::ArrowCursor);
        return false;
    }
}

/**
 * Returns the cursor to be shown when resizing the widget.
 *
 * Default cursor is KCursor::sizeFDiagCursor().
 *
 * @return The cursor to be shown when resizing the widget.
 */
QCursor UMLWidgetController::getResizeCursor()
{
    return Qt::SizeFDiagCursor;
}

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
void UMLWidgetController::resizeWidget(int newW, int newH)
{
    m_widget->setSize(newW, newH);
}

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
void UMLWidgetController::moveWidgetBy(int diffX, int diffY)
{
    m_widget->setX(m_widget->getX() + diffX);
    m_widget->setY(m_widget->getY() + diffY);
}

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
void UMLWidgetController::constrainMovementForAllWidgets(int &diffX, int &diffY)
{
    Q_UNUSED(diffX); Q_UNUSED(diffY);
}

/**
 * Executes the action for double click in the widget.
 * It's called only if the button used was left button.
 * Before calling this method, the widget is selected.
 *
 * Default behaviour is show the properties dialog for the widget using
 * m_widget->slotMenuSelection(ListPopupMenu::mt_Properties);
 * If the widget doesn't have a property dialog (from the WidgetType enum, those that
 * don't have an UMLObject representation) there's no need to override
 * the method, it simply does nothing.
 *
 * @param me The QMouseEvent which triggered the double click event.
 */
void UMLWidgetController::doMouseDoubleClick(QMouseEvent *)
{
    if (!m_widget || !m_widget->m_pMenu)
        return;
    QAction* action = m_widget->m_pMenu->getAction(ListPopupMenu::mt_Properties);
    m_widget->slotMenuSelection(action);
}

/**
 * Clears the selection, resets the toolbar and deselects the widget.
 */
void UMLWidgetController::resetSelection()
{
    m_widget->m_scene->clearSelected();
    m_widget->m_scene->resetToolbar();
    m_widget->setSelected(false);

    m_wasSelected = false;
}

/**
 * Selects the widget and clears the other selected widgets, if any.
 *
 * @param me The QMouseEvent which made the selection.
 */
void UMLWidgetController::selectSingle(QMouseEvent *me)
{
    m_widget->m_scene->clearSelected();

    //Adds the widget to the selected widgets list, but as it has been cleared
    //only the current widget is selected
    selectMultiple(me);
}

/**
 * Selects the widget and adds it to the list of selected widgets.
 *
 * @param me The QMouseEvent which made the selection.
 */
void UMLWidgetController::selectMultiple(QMouseEvent *me)
{
    m_widget->m_selected = true;
    m_widget->setSelected(m_widget->m_selected);
    m_widget->m_scene->setSelected(m_widget, me);

    m_wasSelected = true;
}

/**
 * Deselects the widget and removes it from the list of selected widgets.
 *
 * @param me The QMouseEvent which made the selection.
 */
void UMLWidgetController::deselect(QMouseEvent *me)
{
    m_widget->m_selected = false;
    m_widget->setSelected(m_widget->m_selected);
    m_widget->m_scene->setSelected(m_widget, me);
    //m_wasSelected is false implicitly, no need to set it again
}

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
void UMLWidgetController::saveWidgetValues(QMouseEvent *me)
{
    m_pressOffsetX = me->x() - m_widget->getX();
    m_pressOffsetY = me->y() - m_widget->getY();

    m_prevX = m_oldX = m_widget->getX();
    m_prevY = m_oldY = m_widget->getY();

    m_oldW = m_widget->width();
    m_oldH = m_widget->height();
}

void UMLWidgetController::insertSaveValues(int _oldX, int _oldY, int X, int Y)
{
    m_widget->setX(X);
    m_widget->setY(Y);
    m_oldX = _oldX;
    m_oldY = _oldY;
}

int UMLWidgetController::getOldX()
{
    return m_oldX;
}

int UMLWidgetController::getOldY()
{
    return m_oldY;
}

int UMLWidgetController::getOldW()
{
    return m_oldW;
}

int UMLWidgetController::getOldH()
{
    return m_oldH;
}

/**
 * Fills m_selectedWidgetsList and sets the selection bounds ((m_min/m_max)X/Y attributes).
 */
void UMLWidgetController::setSelectionBounds()
{
    if (m_widget->m_scene->getSelectCount() > 0) {
        m_selectedWidgetsList.clear();
        m_widget->m_scene->getSelectedWidgets(m_selectedWidgetsList, false);

        updateSelectionBounds(1, 1);
    }
}

/**
 * Updates the selection bounds based on the movement made.
 * If it was only a vertical movement, there's no need to update horizontal bounds,
 * and vice versa.
 * TODO: optimize it
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void UMLWidgetController::updateSelectionBounds(int diffX, int diffY)
{
    if (diffX != 0) {
        m_minSelectedX = getSmallestX(m_selectedWidgetsList);
        m_maxSelectedX = getBiggestX(m_selectedWidgetsList);
    }
    if (diffY != 0) {
        m_minSelectedY = getSmallestY(m_selectedWidgetsList);
        m_maxSelectedY = getBiggestY(m_selectedWidgetsList);
    }
}

/**
 * Resizes the widget and adjusts the associations.
 * It's called when a mouse move event happens and the cursor was
 * in resize area when pressed.
 * Resizing can be constrained to an specific axis using control and shift buttons.
 *
 * @param me The QMouseEvent to get the values from.
 */
void UMLWidgetController::resize(QMouseEvent *me)
{
    UMLApp::app()->document()->writeToStatusBar(i18n("Hold shift or ctrl to move in X axis. Hold shift and control to move in Y axis. Right button click to cancel resize."));

    m_resized = true;

    int newW = m_oldW + me->x() - m_widget->getX() - m_pressOffsetX;
    int newH = m_oldH + me->y() - m_widget->getY() - m_pressOffsetY;

    if ((me->modifiers() & Qt::ShiftModifier) && (me->modifiers() & Qt::ControlModifier)) {
        //Move in Y axis
        newW = m_oldW;
    } else if ((me->modifiers() & Qt::ShiftModifier) || (me->modifiers() & Qt::ControlModifier)) {
        //Move in X axis
        newH = m_oldH;
    }

    m_widget->constrain(newW, newH);
    resizeWidget(newW, newH);
    m_widget->adjustAssocs(m_widget->getX(), m_widget->getY());

    m_widget->m_scene->resizeCanvasToItems();
}

/**
 * Returns the smallest X position of all the widgets in the list.
 * TODO: refactor with AlignToolbar method.
 *
 * @param widgetList A list with UMLWidgets.
 * @return The smallest X position.
 */
int UMLWidgetController::getSmallestX(const UMLWidgetList &widgetList)
{
    int smallestX = 0;
    int i = 1;
    foreach(UMLWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            smallestX = widget->getX();
        } else {
            if (smallestX > widget->getX())
                smallestX = widget->getX();
        }
        i++;
    }

    return smallestX;
}

/**
 * Returns the smallest Y position of all the widgets in the list.
 * TODO: refactor with AlignToolbar method.
 *
 * @param widgetList A list with UMLWidgets.
 * @return The smallest Y position.
 */
int UMLWidgetController::getSmallestY(const UMLWidgetList &widgetList)
{
    int smallestY = 0;
    int i = 1;
    foreach(UMLWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            smallestY = widget->getY();
        } else {
            if (smallestY > widget->getY())
                smallestY = widget->getY();

        }
        i++;
    }

    return smallestY;
}

/**
 * Returns the biggest X position of all the widgets in the list.
 * TODO: refactor with AlignToolbar method.
 *
 * @param widgetList A list with UMLWidgets.
 * @return The biggest X position.
 */
int UMLWidgetController::getBiggestX(const UMLWidgetList &widgetList)
{
    int biggestX = 0;

    int i = 1;
    foreach(UMLWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            biggestX = widget->getX();
            biggestX += widget->getWidth();
        } else {
            if (biggestX < widget->getX() + widget->getWidth())
                biggestX = widget->getX() + widget->getWidth();
        }
        i++;
    }

    return biggestX;
}

/**
 * Returns the biggest Y position of all the widgets in the list.
 * TODO: refactor with AlignToolbar method.
 *
 * @param widgetList A list with UMLWidgets.
 * @return The biggest Y position.
 */
int UMLWidgetController::getBiggestY(const UMLWidgetList &widgetList)
{
    int biggestY = 0;
    int i = 1;
    foreach(UMLWidget* widget , widgetList) {
        if (i == 1) {
            if (widget == NULL)
                break;
            biggestY = widget->getY();
            biggestY += widget->getHeight();
        } else {
            if (biggestY < widget->getY() + widget->getHeight())
                biggestY = widget->getY() + widget->getHeight();
        }
        i++;
    }

    return biggestY;
}

/**
 * Returns the adjusted position for the given mouse event.
 * The adjusted position is computed using the current widget position
 * m_widget->get{X,Y}(), the previous position m_old{X,Y}, and the
 * mouse press offset m_pressOffset{X,Y}.
 *
 * @param me The QMouseEvent for which to get the adjusted position.
 * @return A QPoint with the adjusted position.
 */
QPoint UMLWidgetController::getPosition(QMouseEvent* me)
{
    /*
    uDebug() << "me->x=" << me->x()
        << " m_widget->getX=" << m_widget->getX() << ", m_oldX=" << m_oldX
        << ", m_pressOffsetX=" << m_pressOffsetX << endl;
    uDebug() << "me->y=" << me->y()
        << " m_widget->getY=" << m_widget->getY() << ", m_oldY=" << m_oldY
        << ", m_pressOffsetY=" << m_pressOffsetY << endl;
     */
    int newX = me->x() + m_widget->getX() - m_prevX - m_pressOffsetX;
    int newY = me->y() + m_widget->getY() - m_prevY - m_pressOffsetY;
    int maxX = m_widget->m_scene->canvas()->width();
    int maxY = m_widget->m_scene->canvas()->height();

    m_prevX = newX;
    m_prevY = newY;

    if (newX + (m_minSelectedX - m_widget->getX()) < 0) {
        //uDebug() << "got into cond.1";
        newX = m_widget->getX() - m_minSelectedX;
    }
    if (newY + (m_minSelectedY - m_widget->getY()) < 0) {
        //uDebug() << "got into cond.2";
        newY = m_widget->getY() - m_minSelectedY;
    }
    if (newX + (m_maxSelectedX - m_widget->getX()) > maxX) {
        //uDebug() << "got into cond.3";
        newX = maxX - (m_maxSelectedX - m_widget->getX());
    }
    if (newY + (m_maxSelectedY - m_widget->getY()) > maxY) {
        //uDebug() << "got into cond.4";
        newY = maxY - (m_maxSelectedY - m_widget->getY());
    }
    return QPoint(newX, newY);
}

/**
 * Returns a QPoint with the new X and Y position difference of the mouse event
 * respect to the position of the widget.
 *
 * @param me The QMouseEvent to get the position to compare.
 * @return A QPoint with the position difference.
 */
QPoint UMLWidgetController::getPositionDifference(QMouseEvent* me)
{
    QPoint newPoint = getPosition(me);
    const int diffX = newPoint.x() - m_widget->getX();
    const int diffY = newPoint.y() - m_widget->getY();
    return QPoint(diffX, diffY);
}

/**
 * Shows the widget popup menu where the mouse event points to.
 *
 * @param me The QMouseEvent which triggered the showing.
 */
void UMLWidgetController::showPopupMenu(QMouseEvent *me)
{
    //TODO why this condition?
    if (m_widget->m_pMenu) {
        return;
    }
    ListPopupMenu* menu = m_widget->setupPopupMenu();
    menu->popup(me->globalPos());
}

/**
 * Checks if the size of the widget changed respect to the size that
 * it had when press event was fired.
 *
 * @return true if was resized, false otherwise.
 */
bool UMLWidgetController::wasSizeChanged()
{
    return m_oldW != m_widget->getWidth() || m_oldH != m_widget->getHeight();
}

/**
 * Checks if the position of the widget changed respect to the position that
 * it had when press event was fired.
 *
 * @return true if was moved, false otherwise.
 */
bool UMLWidgetController::wasPositionChanged()
{
    return m_oldX != m_widget->getX() || m_oldY != m_widget->getY();
}
