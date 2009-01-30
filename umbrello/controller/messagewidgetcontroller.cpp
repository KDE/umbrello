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

// own header
#include "messagewidgetcontroller.h"

// kde includes
#include <kcursor.h>
#include <kdebug.h>

// app includes
#include "messagewidget.h"
#include "floatingtextwidget.h"
#include "objectwidget.h"
#include "listpopupmenu.h"

MessageWidgetController::MessageWidgetController(MessageWidget* messageWidget):
            UMLWidgetController(messageWidget) {
    m_messageWidget = messageWidget;
    m_unconstrainedPositionY = 0;
}

MessageWidgetController::~MessageWidgetController() {
}

/**
 * Overridden from UMLWidgetController.
 * Saves the values of the widget needed for move/resize.
 * Calls parent method and then saves the value of m_unconstrainedPositionY
 *
 * @param me The QGraphicsSceneMouseEvent to get the offset from.
 */
void MessageWidgetController::saveWidgetValues(QGraphicsSceneMouseEvent *me) {
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
QCursor MessageWidgetController::getResizeCursor() {
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
void MessageWidgetController::resizeWidget(qreal newW, qreal newH) {
    if (m_messageWidget->getSequenceMessageType() == Uml::sequence_message_creation)
        m_messageWidget->setSize(m_messageWidget->getWidth(), newH);
    else {
        qreal x1 = m_messageWidget->m_pOw[Uml::A]->getX();
        qreal x2 = m_messageWidget->getxclicked();
        qreal diffX = 0;
        if (x1 < x2) {
            diffX = x2 + (newW - m_messageWidget->getWidth());
        }
        else {
            diffX = x2 - (newW - m_messageWidget->getWidth());
        }
        if (diffX <= 0 )
            diffX = 10;
        m_messageWidget->setxclicked (diffX);
        m_messageWidget->setSize(newW, newH);
        m_messageWidget->calculateWidget();

    }
    emit m_messageWidget->sigMessageMoved();
}

void MessageWidgetController::moveWidgetBy(qreal /*diffX*/, qreal diffY) {
    m_unconstrainedPositionY += diffY;
    qreal newY = constrainPositionY(diffY);

    if (m_unconstrainedPositionY != newY) {
        if (m_unconstrainedPositionY > m_messageWidget->getY()) {
            newY = m_unconstrainedPositionY;
        } else {
            return;
        }
    }

    m_messageWidget->setY(newY);

    if (m_messageWidget->m_sequenceMessageType == Uml::sequence_message_creation) {
        const qreal objWidgetHalfHeight = m_messageWidget->m_pOw[Uml::B]->getHeight() / 2;
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
void MessageWidgetController::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY) {
    diffX = 0;
    diffY = constrainPositionY(diffY) - m_widget->getY();
}

void MessageWidgetController::doMouseDoubleClick(QGraphicsSceneMouseEvent* /*me*/) {
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
qreal MessageWidgetController::constrainPositionY(qreal diffY) {
    qreal newY = m_widget->getY() + diffY;

    qreal minY = m_messageWidget->getMinY();
    if (m_messageWidget->m_pFText && !m_messageWidget->m_pFText->displayText().isEmpty()) {
        minY += m_messageWidget->m_pFText->getHeight();
    }

    if (newY < minY) {
        newY = minY;
    }

    return newY;
}

