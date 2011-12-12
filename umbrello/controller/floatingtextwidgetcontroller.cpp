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
#include "floatingtextwidgetcontroller.h"

// app includes
#include "basictypes.h"
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"

/**
 * Constructor for FloatingTextWidgetController.
 *
 * @param floatingTextWidget The floating text widget which uses the controller.
 */
FloatingTextWidgetController::FloatingTextWidgetController(FloatingTextWidget *floatingTextWidget)
  : UMLWidgetController(floatingTextWidget)
{
    m_floatingTextWidget = floatingTextWidget;
    m_unconstrainedPositionX = 0;
    m_unconstrainedPositionY = 0;
    m_movementDirectionX = 0;
    m_movementDirectionY = 0;
}

/**
 * Destructor for MessageWidgetController.
 */
FloatingTextWidgetController::~FloatingTextWidgetController()
{
}

/**
 * Overridden from UMLWidgetController.
 * Saves the values of the widget needed for move/resize.
 * Calls parent method and then saves the value of m_unconstrainedPositionX/Y
 * and m_movementDirectionX/Y.
 *
 * @param me The UMLSceneMouseEvent to get the offset from.
 */
void FloatingTextWidgetController::saveWidgetValues(UMLSceneMouseEvent *me)
{
    UMLWidgetController::saveWidgetValues(me);

    m_unconstrainedPositionX = m_widget->getX();
    m_unconstrainedPositionY = m_widget->getY();
    m_movementDirectionX = 0;
    m_movementDirectionY = 0;
}

/**
 * Overridden from UMLWidgetController.
 * FloatingTextWidgets can't be resized, so this method always returns false.
 * Cursor isn't changed.
 *
 * @param me The QMouseEVent to check.
 * @return true if the mouse is in resize area, false otherwise.
 */
bool FloatingTextWidgetController::isInResizeArea(UMLSceneMouseEvent* me)
{
    Q_UNUSED(me);
    return false;
}

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
void FloatingTextWidgetController::moveWidgetBy(qreal diffX, qreal diffY)
{
    if (m_floatingTextWidget->textRole() == Uml::TextRole::Seq_Message_Self)
        return;

    if (m_floatingTextWidget->textRole() == Uml::tr_Seq_Message
                    && ((MessageWidget*)m_floatingTextWidget->m_pLink)->isSelected()) {
        return;
    }

    m_unconstrainedPositionX += diffX;
    m_unconstrainedPositionY += diffY;
    QPointF constrainedPosition = constrainPosition(diffX, diffY);

    qreal newX = constrainedPosition.x();
    qreal newY = constrainedPosition.y();

    if (!m_movementDirectionX) {
        if (m_unconstrainedPositionX != constrainedPosition.x()) {
            m_movementDirectionX = (diffX > 0)? 1: -1;
        }
    } else if ((m_movementDirectionX < 0 && m_unconstrainedPositionX > m_floatingTextWidget->getX()) ||
               (m_movementDirectionX > 0 && m_unconstrainedPositionX < m_floatingTextWidget->getX()) ) {
        newX = m_unconstrainedPositionX;
        m_movementDirectionX = 0;
    }

    if (!m_movementDirectionY) {
        if (m_unconstrainedPositionY != constrainedPosition.y()) {
            m_movementDirectionY = (diffY > 0)? 1: -1;
        }
    } else if ((m_movementDirectionY < 0 && m_unconstrainedPositionY > m_floatingTextWidget->getY()) ||
               (m_movementDirectionY > 0 && m_unconstrainedPositionY < m_floatingTextWidget->getY()) ) {
        newY = m_unconstrainedPositionY;
        m_movementDirectionY = 0;
    }

    m_floatingTextWidget->setX(newX);
    m_floatingTextWidget->setY(newY);

    if (m_floatingTextWidget->m_pLink) {
        m_floatingTextWidget->m_pLink->calculateNameTextSegment();
        if (m_floatingTextWidget->m_Role == Uml::tr_Seq_Message) {
            MessageWidget* messageWidget = (MessageWidget*)m_floatingTextWidget->m_pLink;
            messageWidget->setY(newY + m_floatingTextWidget->getHeight());

            //TODO This should be moved to somewhere in MessageWidget, refactor with messagewidgetcontroller.cpp:44
            if (messageWidget->getSequenceMessageType() == Uml::sequence_message_creation) {
                const qreal objWidgetHalfHeight = messageWidget->getWidget(Uml::B)->getHeight() / 2;
                messageWidget->getWidget(Uml::B)->UMLWidget::setY(messageWidget->getY() - objWidgetHalfHeight);
            }
        }
    }
}

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
void FloatingTextWidgetController::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY)
{
    QPointF constrainedPosition = constrainPosition(diffX, diffY);

    diffX = constrainedPosition.x() - m_floatingTextWidget->getX();
    diffY = constrainedPosition.y() - m_floatingTextWidget->getY();
}

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
 * @return A QPointF with the constrained new position.
 */
QPointF FloatingTextWidgetController::constrainPosition(qreal diffX, qreal diffY)
{
    qreal newX = m_floatingTextWidget->getX() + diffX;
    qreal newY = m_floatingTextWidget->getY() + diffY;

    if (m_floatingTextWidget->m_pLink) {
        m_floatingTextWidget->m_pLink->constrainTextPos(newX, newY,
                    m_floatingTextWidget->getWidth(), m_floatingTextWidget->getHeight(),
                    m_floatingTextWidget->m_Role);
    }

    return QPointF(newX, newY);
}

