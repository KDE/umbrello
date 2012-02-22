/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2012                                               *
 *   Umbrello UML Modeller Authors <uml-devel@uml.sf.net>                  *
 ***************************************************************************/

// own header
#include "messagewidgetcontroller.h"

// kde includes
#include <kcursor.h>

// app includes
#include "messagewidget.h"
#include "floatingtextwidget.h"
#include "objectwidget.h"
#include "listpopupmenu.h"

/**
 * Constructor for MessageWidgetController.
 *
 * @param messageWidget The message widget which uses the controller.
 */
MessageWidgetController::MessageWidgetController(MessageWidget* messageWidget)
  : UMLWidgetController(messageWidget)
{
    m_messageWidget = messageWidget;
    m_unconstrainedPositionY = 0;
}

/**
 * Destructor for MessageWidgetController.
 */
MessageWidgetController::~MessageWidgetController()
{
}

/**
 * Overridden from UMLWidgetController.
 * Saves the values of the widget needed for move/resize.
 * Calls parent method and then saves the value of m_unconstrainedPositionY
 *
 * @param me The QMouseEvent to get the offset from.
 */
void MessageWidgetController::saveWidgetValues(QMouseEvent *me)
{
    UMLWidgetController::saveWidgetValues(me);

    m_unconstrainedPositionY = m_widget->getY();
}

/**
 * Overridden from UMLWidgetController.
 * Returns the cursor to be shown when resizing the widget.
 * The cursor shown is KCursor::sizeVerCursor().
 *
 * @return The cursor to be shown when resizing the widget.
 */
QCursor MessageWidgetController::getResizeCursor()
{
    return Qt::SizeVerCursor;
}

/**
 * Overridden from UMLWidgetController.
 * Resizes the height of the message widget and emits the message moved signal.
 * Message widgets can only be resized vertically, so width isn't modified.
 *
 * @param newW The new width for the widget (isn't used).
 * @param newH The new height for the widget.
 */
void MessageWidgetController::resizeWidget(int newW, int newH)
{
    if (m_messageWidget->sequenceMessageType() == Uml::sequence_message_creation)
        m_messageWidget->setSize(m_messageWidget->width(), newH);
    else {
        int x1 = m_messageWidget->m_pOw[Uml::A]->getX();
        int x2 = m_messageWidget->getxclicked();
        int diffX = 0;
        if (x1 < x2) {
            diffX = x2 + (newW - m_messageWidget->width());
        }
        else {
            diffX = x2 - (newW - m_messageWidget->width());
        }
        if (diffX <= 0 )
            diffX = 10;
        m_messageWidget->setxclicked (diffX);
        m_messageWidget->setSize(newW, newH);
        m_messageWidget->calculateWidget();

    }
    emit m_messageWidget->sigMessageMoved();
}

/**
 * Overridden from UMLWidgetController.
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
void MessageWidgetController::moveWidgetBy(int diffX, int diffY)
{
    Q_UNUSED(diffX);
    m_unconstrainedPositionY += diffY;
    int newY = constrainPositionY(diffY);

    if (m_unconstrainedPositionY != newY) {
        if (m_unconstrainedPositionY > m_messageWidget->getY()) {
            newY = m_unconstrainedPositionY;
        } else {
            return;
        }
    }

    m_messageWidget->setY(newY);

    if (m_messageWidget->m_sequenceMessageType == Uml::sequence_message_creation) {
        const int objWidgetHalfHeight = m_messageWidget->m_pOw[Uml::B]->getHeight() / 2;
        m_messageWidget->m_pOw[Uml::B]->UMLWidget::setY(newY - objWidgetHalfHeight);
    }

    m_messageWidget->moveEvent(0);
}

/**
 * Overridden from UMLWidgetController.
 * Modifies the value of the diffX and diffY variables used to move the widgets.
 * All the widgets are constrained to be moved only in Y axis (diffX is set to 0).
 * @see constrainPositionY
 *
 * @param diffX The difference between current X position and new X position.
 * @param diffY The difference between current Y position and new Y position.
 */
void MessageWidgetController::constrainMovementForAllWidgets(int &diffX, int &diffY)
{
    diffX = 0;
    diffY = constrainPositionY(diffY) - m_widget->getY();
}

/**
 * Overridden from UMLWidgetController.
 * Executes the action for double click in the widget.
 * Shows the dialog to select the operation of the message.
 *
 * @param me The QMouseEvent which triggered the double click event.
 */
void MessageWidgetController::doMouseDoubleClick(QMouseEvent* me)
{
    Q_UNUSED(me);
    if (m_messageWidget->m_pFText != NULL) {
        QAction* action = m_messageWidget->m_pMenu->getAction(ListPopupMenu::mt_Select_Operation);
        m_messageWidget->m_pFText->slotMenuSelection(action);
    }
}

/**
 * Constrains the vertical position of the message widget so it doesn't go
 * upper than the bottom side of the lower object.
 * The height of the floating text widget in the message is taken in account
 * if there is any and isn't empty.
 *
 * @param diffY The difference between current Y position and new Y position.
 * @return The new Y position, constrained.
 */
int MessageWidgetController::constrainPositionY(int diffY)
{
    int newY = m_widget->getY() + diffY;

    int minY = m_messageWidget->getMinY();
    if (m_messageWidget->m_pFText && !m_messageWidget->m_pFText->displayText().isEmpty()) {
        minY += m_messageWidget->m_pFText->getHeight();
    }

    if (newY < minY) {
        newY = minY;
    }

    return newY;
}
