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

#ifndef MESSAGEWIDGETCONTROLLER_H
#define MESSAGEWIDGETCONTROLLER_H

#include "umlwidgetcontroller.h"

class MessageWidget;

/**
 * Controller for MessageWidget.
 *
 * When moving a MessageWidget, it is only moved along Y axis. X axis movement
 * is always ignored.
 * So, if the MessageWidget is being moved as part of a selection and that
 * selection is moved in X and/or Y axis, the MessageWidget will only move in
 * Y axis. Another constrain is applied in Y axis, so the message doesn't pass
 * over the related object widgets. Due to this constrain, the vertical
 * position the message would have if it wasn't constrained is calculated, so
 * when the widget lowers the position where it was constrained it begins to
 * move again.
 * Also, when constraining the move of the selection because the receiver of
 * mouse move events is a MessageWidget, all the widgets are moved only in Y
 * axis. Another constrain is applied in Y axis, so the message doesn't pass
 * over the related object widgets. The unconstrained position isn't need here,
 * because the message widget is the receiver of the events, so when the cursor
 * goes lower than where it was constrained it begins to lower automatically.
 *
 * Creation messages take care of moving the object created when they're moved.
 *
 * Only vertical resize is allowed for MessageWidget. Cursor is set to reflect
 * this.
 *
 * Double click shows the dialog to select the operation of the message.
 *
 * @author Umbrello UML Modeller Authors <uml-devel@lists.sourceforge.net>
 */
class MessageWidgetController : public UMLWidgetController {
public:

    /**
     * Constructor for MessageWidgetController.
     *
     * @param messageWidget The message widget which uses the controller.
     */
    MessageWidgetController(MessageWidget* messageWidget);

    /**
     * Destructor for MessageWidgetController.
     */
    ~MessageWidgetController();

protected:

    /**
     * Overriden from UMLWidgetController.
     * Saves the values of the widget needed for move/resize.
     * Calls parent method and then saves the value of m_unconstrainedPositionY
     *
     * @param me The QMouseEvent to get the offset from.
     */
    virtual void saveWidgetValues(QMouseEvent *me);

    /**
     * Overriden from UMLWidgetController.
     * Returns the cursor to be shown when resizing the widget.
     * The cursor shown is KCursor::sizeVerCursor().
     *
     * @return The cursor to be shown when resizing the widget.
     */
    virtual QCursor getResizeCursor();

    /**
     * Overriden from UMLWidgetController.
     * Resizes the height of the message widget and emits the message moved signal.
     * Message widgets can only be resized vertically, so width isn't modified.
     *
     * @param newW The new width for the widget (isn't used).
     * @param newH The new height for the widget.
     */
    virtual void resizeWidget(int newW, int newH);

    /**
     * Overriden from UMLWidgetController.
     * Moves the widget to a new position using the difference between the
     * current position and the new position. X position is ignored, and widget
     * is only moved along Y axis. If message goes upper than the object, it's
     * kept at this position until it should be lowered again (the unconstrained
     * Y position is saved to know when it's the time to lower it again).
     * If the message is a creation message, the object created is also moved to
     * the new vertical position.
     * @see constrainPositionY
     *
     * @param diffX The difference between current X position and new X position
     *                          (isn't used).
     * @param diffY The difference between current Y position and new Y position.
     */
    virtual void moveWidgetBy(int diffX, int diffY);

    /**
     * Overriden from UMLWidgetController.
     * Modifies the value of the diffX and diffY variables used to move the widgets.
     * All the widgets are constrained to be moved only in Y axis (diffX is set to 0).
     * @see constrainPositionY
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     */
    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);

    /**
     * Overriden from UMLWidgetController.
     * Executes the action for double click in the widget.
     * Shows the dialog to select the operation of the message.
     *
     * @param me The QMouseEvent which triggered the double click event.
     */
    virtual void doMouseDoubleClick(QMouseEvent *me);

private:

    /**
     * Constrains the vertical position of the message widget so it doesn't go
     * upper than the bottom side of the lower object.
     * The height of the floating text widget in the message is taken in account
     * if there is any and isn't empty.
     *
     * @param diffY The difference between current Y position and new Y position.
     * @return The new Y position, constrained.
     */
    int constrainPositionY(int diffY);

    /**
     * The message widget which uses the controller.
     */
    MessageWidget *m_messageWidget;

    /**
     * The vertical position the widget would have if its move wasn't constrained.
     */
    int m_unconstrainedPositionY;

};

#endif
