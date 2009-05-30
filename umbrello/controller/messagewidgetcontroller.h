/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2009                                               *
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
class MessageWidgetController : public UMLWidgetController
{
public:

    MessageWidgetController(MessageWidget* messageWidget);
    ~MessageWidgetController();

protected:

    virtual void saveWidgetValues(QMouseEvent *me);

    virtual QCursor getResizeCursor();

    virtual void resizeWidget(int newW, int newH);

    virtual void moveWidgetBy(int diffX, int diffY);

    virtual void constrainMovementForAllWidgets(int &diffX, int &diffY);

    virtual void doMouseDoubleClick(QMouseEvent *me);

private:

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
