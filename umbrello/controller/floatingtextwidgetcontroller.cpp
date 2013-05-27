/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   copyright (C) 2006-2013                                               *
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
 * @param me The QGraphicsSceneMouseEvent to get the offset from.
 */
void FloatingTextWidgetController::saveWidgetValues(QGraphicsSceneMouseEvent *me)
{
    UMLWidgetController::saveWidgetValues(me);

    m_unconstrainedPositionX = m_widget->x();
    m_unconstrainedPositionY = m_widget->y();
    m_movementDirectionX = 0;
    m_movementDirectionY = 0;
}

/**
 * Overridden from UMLWidgetController.
 * FloatingTextWidgets can't be resized, so this method always returns false.
 * Cursor isn't changed.
 *
 * @param me The QGraphicsSceneMouseEvent to check.
 * @return true if the mouse is in resize area, false otherwise.
 */
bool FloatingTextWidgetController::isInResizeArea(QGraphicsSceneMouseEvent* me)
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
void FloatingTextWidgetController::moveWidgetBy(UMLSceneValue diffX, UMLSceneValue diffY)
{
    if (m_floatingTextWidget->textRole() == Uml::TextRole::Seq_Message_Self)
        return;

    if (m_floatingTextWidget->textRole() == Uml::TextRole::Seq_Message
                    && ((MessageWidget*)m_floatingTextWidget->link())->isSelected()) {
        return;
    }

    m_unconstrainedPositionX += diffX;
    m_unconstrainedPositionY += diffY;
    UMLScenePoint constrainedPosition = constrainPosition(diffX, diffY);

    int newX = constrainedPosition.x();
    int newY = constrainedPosition.y();

    if (!m_movementDirectionX) {
        if (m_unconstrainedPositionX != constrainedPosition.x()) {
            m_movementDirectionX = (diffX > 0)? 1: -1;
        }
    } else if ((m_movementDirectionX < 0 && m_unconstrainedPositionX > m_floatingTextWidget->x()) ||
               (m_movementDirectionX > 0 && m_unconstrainedPositionX < m_floatingTextWidget->x()) ) {
        newX = m_unconstrainedPositionX;
        m_movementDirectionX = 0;
    }

    if (!m_movementDirectionY) {
        if (m_unconstrainedPositionY != constrainedPosition.y()) {
            m_movementDirectionY = (diffY > 0)? 1: -1;
        }
    } else if ((m_movementDirectionY < 0 && m_unconstrainedPositionY > m_floatingTextWidget->y()) ||
               (m_movementDirectionY > 0 && m_unconstrainedPositionY < m_floatingTextWidget->y()) ) {
        newY = m_unconstrainedPositionY;
        m_movementDirectionY = 0;
    }

    m_floatingTextWidget->setX(newX);
    m_floatingTextWidget->setY(newY);

    if (m_floatingTextWidget->link()) {
        m_floatingTextWidget->link()->calculateNameTextSegment();
        if (m_floatingTextWidget->textRole() == Uml::TextRole::Seq_Message) {
            MessageWidget* messageWidget = (MessageWidget*)m_floatingTextWidget->link();
            messageWidget->setY(newY + m_floatingTextWidget->height());
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
void FloatingTextWidgetController::constrainMovementForAllWidgets(UMLSceneValue &diffX, UMLSceneValue &diffY)
{
    UMLScenePoint constrainedPosition = constrainPosition(diffX, diffY);

    diffX = constrainedPosition.x() - m_floatingTextWidget->x();
    diffY = constrainedPosition.y() - m_floatingTextWidget->y();
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
 * @return A UMLScenePoint with the constrained new position.
 */
UMLScenePoint FloatingTextWidgetController::constrainPosition(UMLSceneValue diffX, UMLSceneValue diffY)
{
    UMLSceneValue newX = m_floatingTextWidget->x() + diffX;
    UMLSceneValue newY = m_floatingTextWidget->y() + diffY;

    if (m_floatingTextWidget->link()) {
        m_floatingTextWidget->link()->constrainTextPos(newX, newY,
                    m_floatingTextWidget->width(), m_floatingTextWidget->height(),
                    m_floatingTextWidget->textRole());
    }

    return UMLScenePoint(newX, newY);
}

