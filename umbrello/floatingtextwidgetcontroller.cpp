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

// own header
#include "floatingtextwidgetcontroller.h"
// qt/kde includes
#include <kdebug.h>
// app includes
#include "floatingtextwidget.h"
#include "messagewidget.h"
#include "objectwidget.h"

FloatingTextWidgetController::FloatingTextWidgetController(FloatingTextWidget *floatingTextWidget):
            UMLWidgetController(floatingTextWidget) {
    m_floatingTextWidget = floatingTextWidget;
    m_unconstrainedPositionX = 0;
    m_unconstrainedPositionY = 0;
    m_movementDirectionX = 0;
    m_movementDirectionY = 0;
}

FloatingTextWidgetController::~FloatingTextWidgetController() {
}

void FloatingTextWidgetController::saveWidgetValues(QGraphicsSceneMouseEvent *me) {
    UMLWidgetController::saveWidgetValues(me);

    m_unconstrainedPositionX = m_widget->getX();
    m_unconstrainedPositionY = m_widget->getY();
    m_movementDirectionX = 0;
    m_movementDirectionY = 0;
}

bool FloatingTextWidgetController::isInResizeArea(QGraphicsSceneMouseEvent* /*me*/) {
    return false;
}

void FloatingTextWidgetController::moveWidgetBy(qreal diffX, qreal diffY) {
    if (m_floatingTextWidget->m_Role == Uml::tr_Seq_Message_Self)
        return;

    if (m_floatingTextWidget->m_Role == Uml::tr_Seq_Message
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

void FloatingTextWidgetController::constrainMovementForAllWidgets(qreal &diffX, qreal &diffY) {
    QPointF constrainedPosition = constrainPosition(diffX, diffY);

    diffX = constrainedPosition.x() - m_floatingTextWidget->getX();
    diffY = constrainedPosition.y() - m_floatingTextWidget->getY();
}

QPointF FloatingTextWidgetController::constrainPosition(qreal diffX, qreal diffY) {
    qreal newX = m_floatingTextWidget->getX() + diffX;
    qreal newY = m_floatingTextWidget->getY() + diffY;

    if (m_floatingTextWidget->m_pLink) {
        m_floatingTextWidget->m_pLink->constrainTextPos(newX, newY,
                    m_floatingTextWidget->getWidth(), m_floatingTextWidget->getHeight(),
                    m_floatingTextWidget->m_Role);
    }

    return QPointF(newX, newY);
}

