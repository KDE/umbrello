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

#ifndef OBJECTWIDGETCONTROLLER_H
#define OBJECTWIDGETCONTROLLER_H

#include "umlwidgetcontroller.h"

class ObjectWidget;

/**
 * Controller for ObjectWidget.
 *
 * When moving an ObjectWidget, it is only moved along X axis. Y axis movement
 * is always ignored.
 * So, if the ObjectWidget is being moved as part of a selection and that
 * selection is moved in X and/or Y axis, the ObjectWidget will only move in X axis.
 * Also, when constraining the move of the selection because the receiver of
 * mouse move events is an ObjectWidget, all the widgets are moved only in X axis.
 *
 * Only horizontal resize is allowed for ObjectWidget. Cursor is set to reflect this.
 *
 * @author Umbrello UML Modeller Authors <uml-devel@lists.sourceforge.net>
 */
class ObjectWidgetController : public UMLWidgetController {
public:

    /**
     * Constructor for ObjectWidgetController.
     *
     * @param objectWidget The object widget which uses the controller.
     */
    ObjectWidgetController(ObjectWidget *objectWidget);

    /**
     * Destructor for ObjectWidgetController.
     */
    virtual ~ObjectWidgetController();

protected:

    /**
     * Overriden from UMLWidgetController.
     * Returns the cursor to be shown when resizing the widget.
     * The cursor shown is KCursor::sizeHorCursor().
     *
     * @return The cursor to be shown when resizing the widget.
     */
    virtual QCursor getResizeCursor();

    /**
     * Overriden from UMLWidgetController.
     * Resizes the width of the object widget.
     * Object widgets can only be resized horizontally, so height isn't modified.
     *
     * @param newW The new width for the widget.
     * @param newH The new height for the widget (isn't used).
     */
    virtual void resizeWidget(int newW, int newH);

    /**
     * Overriden from UMLWidgetController.
     * Moves the widget to a new position using the difference between the
     * current position and the new position.
     * Y position is ignored, and widget is only moved along X axis.
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position
     *                          (isn't used).
     */
    virtual void moveWidgetBy(int diffX, int diffY);

    /**
     * Overriden from UMLWidgetController.
     * Modifies the value of the diffX and diffY variables used to move the widgets.
     * All the widgets are constrained to be moved only in X axis (diffY is set to 0).
     *
     * @param diffX The difference between current X position and new X position.
     * @param diffY The difference between current Y position and new Y position.
     */
    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);
};

#endif
