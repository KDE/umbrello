/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2009                                               *
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
class FloatingTextWidgetController : public UMLWidgetController
{
public:

    FloatingTextWidgetController(FloatingTextWidget *floatingTextWidget);

    virtual ~FloatingTextWidgetController();

protected:

    virtual void saveWidgetValues(QMouseEvent *me);

    virtual bool isInResizeArea(QMouseEvent *me);

    virtual void moveWidgetBy(int diffX, int diffY);

    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);

private:

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
