/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2009                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef UMLWIDGETCONTROLLER_H
#define UMLWIDGETCONTROLLER_H

// qt includes
#include <QtCore/QDateTime>

// app includes
#include "umlwidgetlist.h"

class QCursor;
class QMouseEvent;
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
class UMLWidgetController
{
public:

    UMLWidgetController(UMLWidget *widget);
    virtual ~UMLWidgetController();

    virtual void mousePressEvent(QMouseEvent *me);
    virtual void mouseMoveEvent(QMouseEvent* me);
    virtual void mouseReleaseEvent(QMouseEvent * me);
    virtual void mouseDoubleClickEvent(QMouseEvent *me);

    virtual void widgetMoved();

    int getOldX();
    int getOldY();
    int getOldH();
    int getOldW();

    void insertSaveValues(int _oldX, int _oldY, int X, int Y);

    virtual void moveWidgetBy(int diffX, int diffY);

    virtual void resizeWidget(int newW, int newH);

    virtual UMLWidget* getWidget();

protected:

    virtual void saveWidgetValues(QMouseEvent *me);

    virtual bool isInResizeArea(QMouseEvent *me);

    virtual QCursor getResizeCursor();

    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);

    virtual void doMouseDoubleClick(QMouseEvent *me);

    void resetSelection();

    void selectSingle(QMouseEvent *me);
    void selectMultiple(QMouseEvent *me);

    void deselect(QMouseEvent *me);

    void setSelectionBounds();

    void updateSelectionBounds(int diffX, int diffY);

    void resize(QMouseEvent *me);

    int getSmallestX(const UMLWidgetList &widgetList);
    int getSmallestY(const UMLWidgetList &widgetList);

    int getBiggestX(const UMLWidgetList &widgetList);
    int getBiggestY(const UMLWidgetList &widgetList);

    QPoint getPosition(QMouseEvent *me);
    QPoint getPositionDifference(QMouseEvent *me);

    void showPopupMenu(QMouseEvent *me);

    bool wasSizeChanged();
    bool wasPositionChanged();

    /**
     * The widget which uses the controller.
     */
    UMLWidget *m_widget;

    /**
     * Timer that prevents excessive updates (be easy on the CPU).
     */
    QTime m_lastUpdate;

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
     * The previous recorded X/Y position of the widget during its movement
     * This keeps changing as the widget is dragged along its path
     */
    int m_prevX, m_prevY;

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
