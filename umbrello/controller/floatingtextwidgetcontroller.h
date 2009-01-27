/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2007                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

#ifndef FLOATINGTEXTWIDGETCONTROLLER_H
#define FLOATINGTEXTWIDGETCONTROLLER_H

#include "umlwidgetcontroller.h"

class FloatingTextWidget;

/**
 * Controller for FloatingTextWidget.
 *
 * When moving a FloatingTextWidget, it is constrained using constrainTextPos
 * method from the LinkWidget, if any.
 * This applies both when moving as part of a selection and when constraining
 * the move of the selection because it's the receiver of mouse move events.
 * The only exception to this is that when moving the widget, if it's a sequence
 * message and the message widget is selected, the floating text widget isn't
 * moved itself, but automatically by the message widget.
 * If the sequence message wasn't part of the selection, the floating text
 * widget moves it.
 * When moving the floating text as part of a selection, if the position of
 * the floating text is constrained, it's kept at that position until it can
 * be moved to another valid position.
 * No resize is allowed for FloatingTextWidget.
 *
 * @author Umbrello UML Modeller Authors <uml-devel@lists.sourceforge.net>
 */
class FloatingTextWidgetController : public UMLWidgetController {
public:

    /**
     * Constructor for FloatingTextWidgetController.
     *
     * @param floatingTextWidget The floating text widget which uses the controller.
     */
    FloatingTextWidgetController(FloatingTextWidget *floatingTextWidget);

    /**
     * Destructor for MessageWidgetController.
     */
    ~FloatingTextWidgetController();

protected:

    /**
     * Overridden from UMLWidgetController.
     * Saves the values of the widget needed for move/resize.
     * Calls parent method and then saves the value of m_unconstrainedPositionX/Y
     * and m_movementDirectionX/Y.
     *
     * @param me The QMouseEvent to get the offset from.
     */
    virtual void saveWidgetValues(QMouseEvent *me);

    /**
     * Overridden from UMLWidgetController.
     * FloatingTextWidgets can't be resized, so this method always returns false.
     * Cursor isn't changed.
     *
     * @param me The QMouseEVent to check.
     * @return true if the mouse is in resize area, false otherwise.
     */
    virtual bool isInResizeArea(QMouseEvent *me);

    /**
     * Overridden from UMLWidgetController.
     * Moves the widget to a new position using the difference between the
     * current position and the new position.
     * If the floating text widget is part of a sequence message, and the
     * message widget is selected, it does nothing: the message widget will
     * update the text position when it's moved.
     * In any other case, the floating text widget constrains its move using
     * constrainPosition. When the position of the floating text is constrained,
     * it's kept at that position until it can be moved to another valid
     * position (m_unconstrainedPositionX/Y and m_movementDirectionX/Y are
     * used for that).
     * Moreover, if is part of a sequence message (and the message widget
     * isn't selected), it updates the position of the message widget.
     * @see constrainPosition
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     */
    virtual void moveWidgetBy(int diffX, int diffY);

    /**
     * Overridden from UMLWidgetController.
     * Modifies the value of the diffX and diffY variables used to move the
     * widgets.
     * The values are constrained using constrainPosition.
     * @see constrainPosition
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     */
    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);

private:

    /**
     * Returns a constrained position for the widget after applying the position
     * difference.
     * If no link widget exists, the position returned is the current widget
     * position with the difference applied. If there's a link, the position
     * to be returned is constrained using constrainTextPos method from the
     * LinkWidget, if any.
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     * @return A QPoint with the constrained new position.
     */
    QPoint constrainPosition(int diffX, int diffY);

    /**
     * The floating text widget which uses the controller.
     */
    FloatingTextWidget *m_floatingTextWidget;

    /**
     * The horizontal position the widget would have if its move wasn't constrained.
     */
    int m_unconstrainedPositionX;

    /**
     * The vertical position the widget would have if its move wasn't constrained.
     */
    int m_unconstrainedPositionY;

    /**
     * The X direction the widget was moved when the constrain was applied.
     * -1 means left, 1 means right.
     */
    int m_movementDirectionX;

    /**
     * The Y direction the widget was moved when the constrain was applied.
     * -1 means up, 1 means down.
     */
    int m_movementDirectionY;
};

#endif
